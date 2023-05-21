#include <iostream>
#include <vector>
#include <random>
#include <climits>
#include <utility>
#include <cassert>

template<class ValueType>
class Set {
private:
    struct Node {
        ValueType val;
        int priority;
        Node *left = nullptr, *right = nullptr, *parent = nullptr;

        Node(const ValueType &value, int pr) : val(value), priority(pr) {
        }
    };

    Node *root_ = nullptr;
    std::mt19937 generator_;
    std::uniform_int_distribution<int> dist_;
    int size_ = 0;

    void UpdateParent(Node *root) {
        if (!root) {
            return;
        }
        if (root->left) {
            root->left->parent = root;
        }
        if (root->right) {
            root->right->parent = root;
        }
    }

    std::pair<Node *, Node *> split(Node *root, const ValueType &value) {
        // делает два дерева, в первом все с ключом < value, во втором >=
        if (!root) {
            return {nullptr, nullptr};
        }
        if (value < root->val) {
            auto res = split(root->left, value);
            root->left = res.second;
            UpdateParent(root);
            UpdateParent(res.first);
            return {res.first, root};

        } else if (root->val < value) {
            auto res = split(root->right, value);
            root->right = res.first;
            UpdateParent(root);
            UpdateParent(res.second);
            return {root, res.second};
        } else {
            throw std::runtime_error{"equal element"};
        }
    }

    Node *merge(Node *lhs, Node *rhs) {
        // слияет деревья с корнями lhs, rhs, причем все ключи lhs <= всех в rhs
        if (!lhs || !rhs) {
            return (!lhs ? rhs : lhs);
        }
        if (lhs->priority > rhs->priority) {
            lhs->right = merge(lhs->right, rhs);
            UpdateParent(lhs);
            return lhs;
        } else {
            rhs->left = merge(lhs, rhs->left);
            UpdateParent(rhs);
            return rhs;
        }
    }

    void Delete(Node *root) {
        if (!root) {
            return;
        }
        Delete(root->left);
        Delete(root->right);
        delete root;
    }

    void Copy(Node *root) {
        if (!root) {
            return;
        }
        insert(root->val);
        Copy(root->left);
        Copy(root->right);
    }

    void PrintTree(Node *root) const {
        if (!root) {
            return;
        }
        PrintTree(root->left);
        if (root) {
            std::cout << root->val << " ";
            if (root->parent) {
                std::cout << "Parent: " << root->parent->val << "\n";
            } else {
                std::cout << "\n";
            }
        }
        PrintTree(root->right);
    }

    static Node *GetLeft(Node *root) {
        Node *cur = root;
        while (root) {
            cur = root;
            root = root->left;
        }
        return cur;
    }

    static Node *GetRight(Node *root) {
        Node *cur = root;
        while (root) {
            cur = root;
            root = root->right;
        }
        return cur;
    }

    Node *Insert(Node *root, Node *vertex) {
        auto[l, r] = split(root, vertex->val);
        root = merge(l, vertex);
        root = merge(root, r);
        return root;
    }

    void Erase(Node *&root, const ValueType &value, Node *prev) {
        if (!root) {
            return;
        }
        if (value < root->val) {
            Erase(root->left, value, root);
        } else if (root->val < value) {
            Erase(root->right, value, root);
        } else {
            // нашли элемент
            Node *cur = root;
            if (root->left) {
                root->left->parent = prev;
            }
            if (root->right) {
                root->right->parent = prev;
            }
            root = merge(root->left, root->right);
            size_ -= 1;
            delete cur;
        }
    }

public:
    Set() : root_(nullptr),
            generator_(std::random_device {}()), dist_(0, INT_MAX), size_(0) {}

    void insert(const ValueType &value) {
        if (find(value) != end()) {
            return;
        }
        size_ += 1;
        Node *vertex = new Node(value, dist_(generator_));
        root_ = Insert(root_, vertex);
    }

    void erase(const ValueType &value) {
        Erase(root_, value, nullptr);
    }

    bool empty() const {
        return size_ == 0;
    }

    int size() const {
        return size_;
    }

    void Print() const {
        PrintTree(root_);
        std::cout << "End print\n";
    }

    ~Set() {
        Delete(root_);
    }

    template<typename Iter>
    Set(Iter first, Iter last) : root_(nullptr),
                                 generator_(std::random_device {}()), dist_(0, INT_MAX) {
        for (auto iter = first; iter != last; ++iter) {
            insert(*iter);
        }
    }

    explicit Set(const std::initializer_list<ValueType> &arr) : root_(nullptr),
                                                                generator_(std::random_device {}()), dist_(0, INT_MAX) {
        for (auto it = arr.begin(); it != arr.end(); ++it) {
            insert(*it);
        }
    }

    Set(const Set &other) {
        Copy(other.root_);
        size_ = other.size_;
        generator_ = other.generator_;
        dist_ = other.dist_;
    }

    Set &operator=(const Set &other) {
        if (this == &other) {
            return *this;
        }
        this->~Set();
        new(this) Set(other);
        return *this;
    }

    class iterator :
            public std::iterator<std::bidirectional_iterator_tag, ValueType> {
    private:
        Node *ptr_ = nullptr;
        bool end_ = false;

        friend class Set<ValueType>;

    public:
        iterator() : ptr_(nullptr), end_(true) {}

        explicit iterator(Node *ptr, bool end) : ptr_(ptr), end_(end) {
        }

        iterator &operator++() {
            if (!ptr_) {
                throw std::runtime_error{"Null pointer in ++"};
            }
            Node *prev = ptr_;
            if (ptr_->right) {
                ptr_ = GetLeft(ptr_->right);
                return *this;
            }
            Node *par = ptr_->parent;
            while (par && par->right == ptr_) {
                ptr_ = par;
                par = ptr_->parent;
            }
            if (!par) {
                ptr_ = prev;
                end_ = true;
                return *this;
            }
            ptr_ = par;
            return *this;
        }

        iterator &operator--() {
            if (end_) {
                end_ = false;
                return *this;
            }
            if (ptr_->left) {
                ptr_ = GetRight(ptr_->left);
                return *this;
            }
            Node *par = ptr_->parent;
            while (par && par->left == ptr_) {
                ptr_ = par;
                par = ptr_->parent;
            }
            ptr_ = par;
            return *this;
        }

        iterator operator++(int) {
            iterator current = *this;
            ++(*this);
            return current;
        }

        iterator operator--(int) {
            iterator current = *this;
            --(*this);
            return current;
        }

        ValueType &operator*() const {
            return ptr_->val;
        }

        ValueType *operator->() const {
            return &(ptr_->val);
        }

        bool operator==(const iterator &other) const {
            if (end_ && other.end_) {
                return true;
            }
            if (!ptr_ && !other.ptr_) {
                return true;
            }
            return ptr_ == other.ptr_ && end_ == other.end_;
        }

        bool operator!=(const iterator &other) const {
            return !(*this == other);
        }
    };

    iterator find(const ValueType &value) const {
        Node *ptr = root_;
        while (ptr) {
            if (value < ptr->val) {
                ptr = ptr->left;
            } else if (ptr->val < value) {
                ptr = ptr->right;
            } else {
                return iterator(ptr, 0);
            }
        }
        return end();
    }

    iterator begin() const {
        return iterator(GetLeft(root_), 0);
    }

    iterator end() const {
        return iterator(GetRight(root_), 1);
    }

    iterator lower_bound(const ValueType &val) const {
        Node *prev = nullptr;
        Node *cur = root_;
        while (cur) {
            if (val < cur->val) {
                prev = cur;
                cur = cur->left;
            } else if (cur->val < val) {
                cur = cur->right;
            } else {
                return iterator(cur, 0);
            }
        }
        if (!prev) {
            return end();
        }
        return iterator(prev, 0);
    }
};
