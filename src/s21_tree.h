#ifndef S21_CONTAINER_SRC_S21_TREE_H_
#define S21_CONTAINER_SRC_S21_TREE_H_

#include <limits>
#include <utility>
#include <iterator>

namespace s21 {

enum class RBTreeNodeColor : bool { kRed, kBlack };

struct RBTreeNodeBase {
  using ColorType = RBTreeNodeColor;
  using BasePtr = RBTreeNodeBase*;

  RBTreeNodeBase() = default;
  explicit RBTreeNodeBase(ColorType o_color) noexcept : color(o_color) {}

  ColorType color{};
  BasePtr parent{};
  BasePtr left{};
  BasePtr right{};
};

template<typename Tp>
struct RBTreeNode : public RBTreeNodeBase {
  Tp m_data;

  RBTreeNode() = default;
  RBTreeNode(const RBTreeNode& other) : RBTreeNodeBase(other.color), m_data(other.m_data) {}
  explicit RBTreeNode(const Tp& value) : m_data(value) {}
};

struct RBTreeIteratorBase {
  using BasePtr = typename RBTreeNodeBase::BasePtr;
  using ColorType = typename RBTreeNodeBase::ColorType;
  using iterator_category = std::bidirectional_iterator_tag;
  using difference_type = ptrdiff_t;

  BasePtr node;

  explicit RBTreeIteratorBase(BasePtr other_node) noexcept : node(other_node) {}

  void Increment() noexcept {
    if (node->right) {
      node = node->right;
      while (node->left) {
        node = node->left;
      }
    } else {
      BasePtr tmp = node->parent;
      while (node == tmp->right) {
        node = tmp;
        tmp = tmp->parent;
      }
      if (node->right != tmp) {
        node = tmp;
      }
    }
  }

  void Decrement() noexcept {
    if (node->color == ColorType::kRed &&
        node->parent->parent == node) {
      node = node->right;
    } else if (node->left) {
      BasePtr tmp = node->left;
      while (tmp->right) {
        tmp = tmp->right;
      }
      node = tmp;
    } else {
      BasePtr tmp = node->parent;
      while (node == tmp->left) {
        node = tmp;
        tmp = tmp->parent;
      }
      node = tmp;
    }
  }
};

template<typename Val, typename Ref, typename Ptr>
struct RBTreeIterator : public RBTreeIteratorBase {
  using value_type = Val;
  using reference = Ref;
  using pointer = Ptr;
  using Self = RBTreeIterator<Val, Ref, Ptr>;
  using NodePtr = RBTreeNode<Val>*;

  explicit RBTreeIterator(BasePtr ptr = nullptr) noexcept : RBTreeIteratorBase(ptr) {}

  reference operator*() const noexcept { return static_cast<NodePtr>(node)->m_data; }

  pointer operator->() const noexcept { return &static_cast<NodePtr>(node)->m_data; }

  Self& operator++() noexcept {
    Increment();
    return *this;
  }

  Self operator++(int) noexcept {
    Self tmp = *this;
    Increment();
    return tmp;
  }

  Self& operator--() noexcept {
    Decrement();
    return *this;
  }

  Self operator--(int) noexcept {
    Self tmp = *this;
    Decrement();
    return tmp;
  }
};

inline bool operator==(const RBTreeIteratorBase& rhs,
                       const RBTreeIteratorBase& lhs) noexcept {
  return rhs.node == lhs.node;
}

inline bool operator!=(const RBTreeIteratorBase& rhs,
                       const RBTreeIteratorBase& lhs) noexcept {
  return rhs.node != lhs.node;
}

namespace KeyGetters {

template<class Tp>
struct Identity {
  const Tp& operator()(const Tp& r) const noexcept { return r; }
};

template<class Pair>
struct SelectFirst {
  const typename Pair::first_type& operator()(const Pair& p) const noexcept { return p.first; }
};

} // KeyGetters

template<typename Key, typename Value, typename KeyOfValue, typename Compare>
class RBTree {
 public:
  using key_type = Key;
  using value_type = Value;
  using pointer = Value*;
  using const_pointer = const Value*;
  using reference = Value&;
  using const_reference = const Value&;
  using Node = RBTreeNode<Value>;
  using NodePtr = Node*;
  using BasePtr = typename RBTreeNodeBase::BasePtr;
  using ColorType = typename RBTreeNodeBase::ColorType;
  using size_type = size_t;
  using difference_type = ptrdiff_t;
  using iterator = RBTreeIterator<value_type, reference, pointer>;
  using const_iterator = RBTreeIterator<value_type, const_reference, const_pointer>;

  RBTree() : base_(new Node()) {
    base_->parent = nullptr;
    base_->left = base_->right = base_;
  }

  RBTree(const RBTree& other)
      : node_count_(other.node_count_), compare_(other.compare_), base_(new Node()) {
    base_->color = ColorType::kRed;
    if (!other.base_->parent) {
      base_->parent = nullptr;
      base_->left = base_->right = base_;
    } else {
      base_->parent = CopyTree(other.base_->parent, base_);
      base_->left = GetMinNode(base_->parent);
      base_->right = GetMaxNode(base_->parent);
    }
  }

  RBTree(RBTree&& other) noexcept
      : node_count_(other.node_count_), compare_(other.compare_), base_(other.base_) {
    other.base_ = nullptr;
    other.node_count_ = 0;
  }

  ~RBTree() {
    clear();
    delete base_;
  }

  RBTree& operator=(const RBTree& other) {
    if (this != &other) {
      RBTree tmp(other);
      swap(tmp);
    }
    return *this;
  }

  RBTree& operator=(RBTree&& other) noexcept {
    RBTree tmp(std::move(other));
    if (this != &other) {
      swap(tmp);
    }
    return *this;
  }

  void clear() {
    if (node_count_ != 0) {
      DeleteTree(base_->parent);
      base_->parent = nullptr;
      base_->left = base_->right = base_;
      node_count_ = 0;
    }
  }

  void swap(RBTree& other) noexcept {
    std::swap(other.base_, base_);
    std::swap(other.node_count_, node_count_);
    std::swap(other.compare_, compare_);
  }

  iterator begin() noexcept { return iterator(base_->left); }

  const_iterator begin() const noexcept { return const_iterator(base_->left); }

  const_iterator cbegin() const noexcept { return const_iterator(base_->left); }

  iterator end() noexcept { return iterator(base_); }

  const_iterator end() const noexcept { return const_iterator(base_); }

  const_iterator cend() const noexcept { return const_iterator(base_); }

  bool empty() const noexcept { return node_count_ == 0; }

  size_type size() const noexcept { return node_count_; }

  size_type max_size() const noexcept { return std::numeric_limits<size_type>::max()/2/sizeof(Node); }

  iterator find(const Key& key) noexcept {
    return iterator(find_(key));
  }

  const_iterator find(const Key& key) const noexcept {
    return const_iterator(find_(key));
  }

  std::pair<iterator, iterator> equal_range(const Key& key) noexcept {
    return std::make_pair(lower_bound(key), upper_bound(key));
  }

  std::pair<const_iterator, const_iterator> equal_range(const Key& key) const noexcept {
    return std::make_pair(lower_bound(key), upper_bound(key));
  }

  iterator lower_bound(const Key& key) noexcept {
    return iterator(lower_bound_(key));
  }

  const_iterator lower_bound(const Key& key) const noexcept {
    return const_iterator(lower_bound_(key));
  }

  iterator upper_bound(const Key& key) noexcept {
    return iterator(upper_bound_(key));
  }

  const_iterator upper_bound(const Key& key) const noexcept {
    return const_iterator(upper_bound_(key));
  }

  size_type count(const Key& key) const noexcept {
    std::pair<const_iterator, const_iterator> r_pair = equal_range(key);
    size_type n_nodes = 0;
    for (; r_pair.first != r_pair.second; ++r_pair.first, ++n_nodes);
    return n_nodes;
  }

  void merge_unique(RBTree& other) {
    int i = 0;
    iterator to_delete[other.size()];
    for(auto it = other.begin(); it != other.end(); ++it) {
      auto pair_found = insert_unique(*it);
      if (pair_found.second) {
        to_delete[i++] = it;
      }
    }
    while(i) {
      other.erase(to_delete[--i]);
    }
  }

  void merge_equal(RBTree& other) {
    for(auto& val : other) {
      insert_equal(val);
    }
    other.clear();
  }

  std::pair<iterator, bool> insert_unique(const Value& val) {
    BasePtr curr_node = base_->parent, prev_node = base_;
    bool cmp = true;
    while (curr_node) {
      prev_node = curr_node;
      cmp = compare_(KeyOfValue()(val), GetKey(curr_node));
      curr_node = cmp ? curr_node->left : curr_node->right;
    }
    auto it = iterator(prev_node);
    if (cmp) {
      if (it == begin()) {
        return std::pair<iterator, bool>(insert_(curr_node, prev_node, val), true);
      } else {
        --it;
      }
    }
    if (compare_(GetKey(it.node), KeyOfValue()(val))) {
      return std::pair<iterator, bool>(insert_(curr_node, prev_node, val), true);
    }
    return std::pair<iterator, bool>(it, false);
  }

  std::pair<iterator, bool> insert_equal(const Value& val) {
    BasePtr prev_node = base_, curr_node = base_->parent;
    while (curr_node) {
      prev_node = curr_node;
      curr_node = compare_(KeyOfValue()(val), GetKey(curr_node)) ?
                  curr_node->left : curr_node->right;
    }
    return std::make_pair(insert_(curr_node, prev_node, val), true);
  }

  void erase(iterator position) {
    auto node_to_delete = EraseRebalance(position.node);
    delete node_to_delete;
    --node_count_;
  }

  void erase(const Key& key) {
    std::pair<iterator, iterator> r_pair = equal_range(key);
    if (r_pair.first == begin() && r_pair.second == end()) {
      clear();
    } else {
      while (r_pair.first != r_pair.second) {
        erase(r_pair.first++);
      }
    }
  }

 protected:
  static const Key& GetKey(BasePtr node) noexcept { return KeyOfValue()(static_cast<NodePtr>(node)->m_data); }

  static BasePtr GetMinNode(BasePtr root) noexcept {
    while (root->left) {
      root = root->left;
    }
    return root;
  }
  static BasePtr GetMaxNode(BasePtr root) noexcept {
    while (root->right) {
      root = root->right;
    }
    return root;
  }

  static void DeleteTree(BasePtr root) {
    while (root) {
      DeleteTree(root->right);
      BasePtr left = root->left;
      delete root;
      root = left;
    }
  }

  static NodePtr CopyTree(BasePtr root, BasePtr head) {
    auto top = new Node(*static_cast<NodePtr>(root));
    top->parent = head;
    if (root->right) {
      top->right = CopyTree(root->right, top);
    }
    head = top;
    root = root->left;
    while (root) {
      auto copy = new Node(*static_cast<NodePtr>(root));
      head->left = copy;
      copy->parent = head;
      if (root->right) {
        copy->right = CopyTree(root->right, copy);
      }
      head = copy;
      root = root->left;
    }
    return top;
  }

  BasePtr upper_bound_(const Key& key) const noexcept {
    BasePtr prev_node = base_, curr_node = base_->parent;
    while (curr_node) {
      if (compare_(key, GetKey(curr_node))) {
        prev_node = curr_node;
        curr_node = curr_node->left;
      } else {
        curr_node = curr_node->right;
      }
    }
    return prev_node;
  }

  BasePtr lower_bound_(const Key& key) const noexcept {
    BasePtr prev_node = base_, curr_node = base_->parent;
    while (curr_node) {
      if (!compare_(GetKey(curr_node), key)) {
        prev_node = curr_node;
        curr_node = curr_node->left;
      } else {
        curr_node = curr_node->right;
      }
    }
    return prev_node;
  }

  BasePtr find_(const Key& key) const noexcept {
    BasePtr found_node = lower_bound_(key);
    if (found_node == base_ || compare_(key, GetKey(found_node))) {
      return base_;
    }
    return found_node;
  }

  iterator insert_(BasePtr curr_node, BasePtr prev_node, const Value& val) {
    auto new_node = new Node(val);
    if (prev_node == base_ || curr_node ||
        compare_(KeyOfValue()(val), GetKey(prev_node))) {
      prev_node->left = new_node;
      if (prev_node == base_) {
        base_->parent = new_node;
        base_->right = new_node;
      } else if (prev_node == base_->left) {
        base_->left = new_node;
      }
    } else {
      prev_node->right = new_node;
      if (prev_node == base_->right) {
        base_->right = new_node;
      }
    }
    new_node->parent = prev_node;
    new_node->left = new_node->right = nullptr;
    InsertRebalance(new_node);
    ++node_count_;
    return iterator(new_node);
  }

  void RotateLeft(BasePtr node) noexcept {
    BasePtr right_child = node->right;
    node->right = right_child->left;
    if (right_child->left) {
      right_child->left->parent = node;
    }
    right_child->parent = node->parent;
    if (node == base_->parent) {
      base_->parent = right_child;
    } else if (node == node->parent->left) {
      node->parent->left = right_child;
    } else {
      node->parent->right = right_child;
    }
    right_child->left = node;
    node->parent = right_child;
  }

  void RotateRight(BasePtr node) noexcept {
    BasePtr left_child = node->left;
    node->left = left_child->right;
    if (left_child->right) {
      left_child->right->parent = node;
    }
    left_child->parent = node->parent;
    if (node == base_->parent) {
      base_->parent = left_child;
    } else if (node == node->parent->right) {
      node->parent->right = left_child;
    } else {
      node->parent->left = left_child;
    }
    left_child->right = node;
    node->parent = left_child;
  }

  void InsertRebalance(BasePtr node) noexcept {
    node->color = ColorType::kRed;
    while (node != base_->parent && node->parent->color == ColorType::kRed) {
      BasePtr uncle;
      if (node->parent == node->parent->parent->left) {
        uncle = node->parent->parent->right;
        if (uncle && uncle->color == ColorType::kRed) {
          node->parent->color = ColorType::kBlack;
          uncle->color = ColorType::kBlack;
          node->parent->parent->color = ColorType::kRed;
          node = node->parent->parent;
        } else {
          if (node == node->parent->right) {
            node = node->parent;
            RotateLeft(node);
          }
          node->parent->color = ColorType::kBlack;
          node->parent->parent->color = ColorType::kRed;
          RotateRight(node->parent->parent);
        }
      } else {
        uncle = node->parent->parent->left;
        if (uncle && uncle->color == ColorType::kRed) {
          node->parent->color = ColorType::kBlack;
          uncle->color = ColorType::kBlack;
          node->parent->parent->color = ColorType::kRed;
          node = node->parent->parent;
        } else {
          if (node == node->parent->left) {
            node = node->parent;
            RotateRight(node);
          }
          node->parent->color = ColorType::kBlack;
          node->parent->parent->color = ColorType::kRed;
          RotateLeft(node->parent->parent);
        }
      }
    }
    base_->parent->color = ColorType::kBlack;
  }

  BasePtr EraseRebalance(BasePtr node) noexcept {
    BasePtr curr_node = node, prev_node, pv_parent;
    if (!curr_node->left) {
      prev_node = curr_node->right;
    } else {
      if (!curr_node->right) {
        prev_node = curr_node->left;
      } else {
        curr_node = GetMinNode(curr_node->right);
        prev_node = curr_node->right;
      }
    }
    if (curr_node != node) {
      node->left->parent = curr_node;
      curr_node->left = node->left;
      if (curr_node != node->right) {
        pv_parent = curr_node->parent;
        if (prev_node) {
          curr_node->parent = curr_node->parent;
        }
        curr_node->parent->left = prev_node;
        curr_node->right = node->right;
        node->right->parent = curr_node;
      } else {
        pv_parent = curr_node;
      }
      if (base_->parent == node) {
        base_->parent = curr_node;
      } else if (node->parent->left == node) {
        node->parent->left = curr_node;
      } else {
        node->parent->right = curr_node;
      }
      curr_node->parent = node->parent;
      std::swap(curr_node->color, node->color);
      curr_node = node;
    } else {
      pv_parent = curr_node->parent;
      if (prev_node) {
        prev_node->parent = curr_node->parent;
      }
      if (base_->parent == node) {
        base_->parent = prev_node;
      } else {
        if (node->parent->left == node) {
          node->parent->left = prev_node;
        } else {
          node->parent->right = prev_node;
        }
      }
      if (base_->left == node) {
        if (!node->right) {
          base_->left = node->parent;
        } else {
          base_->left = GetMinNode(prev_node);
        }
      }
      if (base_->right == node) {
        if (!node->left) {
          base_->right = node->parent;
        } else {
          base_->right = GetMaxNode(prev_node);
        }
      }
    }
    if (curr_node->color != ColorType::kRed) {
      while (prev_node != base_->parent &&
          (!prev_node || prev_node->color == ColorType::kBlack)) {
        BasePtr uncle;
        if (prev_node == pv_parent->left) {
          uncle = pv_parent->right;
          if (uncle->color == ColorType::kRed) {
            uncle->color = ColorType::kBlack;
            pv_parent->color = ColorType::kRed;
            RotateLeft(pv_parent);
            uncle = pv_parent->right;
          }
          if ((!uncle->left || uncle->left->color == ColorType::kBlack) &&
              (!uncle->right || uncle->right->color == ColorType::kBlack)) {
            uncle->color = ColorType::kRed;
            prev_node = pv_parent;
            pv_parent = pv_parent->parent;
          } else {
            if (!uncle->right || uncle->right->color == ColorType::kBlack) {
              if (uncle->left) {
                uncle->left->color = ColorType::kBlack;
              }
              uncle->color = ColorType::kRed;
              RotateRight(uncle);
              uncle = pv_parent->right;
            }
            uncle->color = pv_parent->color;
            pv_parent->color = ColorType::kBlack;
            if (uncle->right) {
              uncle->right->color = ColorType::kBlack;
            }
            RotateLeft(pv_parent);
            break;
          }
        } else {
          uncle = pv_parent->left;
          if (uncle->color == ColorType::kRed) {
            uncle->color = ColorType::kBlack;
            pv_parent->color = ColorType::kRed;
            RotateRight(pv_parent);
            uncle = pv_parent->left;
          }
          if ((!uncle->right || uncle->right->color == ColorType::kBlack) &&
              (!uncle->left || uncle->left->color == ColorType::kBlack)) {
            uncle->color = ColorType::kRed;
            prev_node = pv_parent;
            pv_parent = pv_parent->parent;
          } else {
            if (!uncle->left || uncle->left->color == ColorType::kBlack) {
              if (uncle->right) {
                uncle->right->color = ColorType::kBlack;
              }
              uncle->color = ColorType::kRed;
              RotateLeft(uncle);
              uncle = pv_parent->left;
            }
            uncle->color = pv_parent->color;
            pv_parent->color = ColorType::kBlack;
            if (uncle->left) {
              uncle->left->color = ColorType::kBlack;
            }
            RotateRight(pv_parent);
            break;
          }
        }
      }
      if (prev_node) {
        prev_node->color = ColorType::kBlack;
      }
    }
    return curr_node;
  }

 private:
  size_type node_count_{};
  Compare compare_;
  BasePtr base_;
};

} // namespace s21

#endif // S21_CONTAINER_SRC_S21_TREE_H_