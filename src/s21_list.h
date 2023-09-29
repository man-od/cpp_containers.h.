#ifndef S21_CONTAINER_SRC_S21_LIST_H_
#define S21_CONTAINER_SRC_S21_LIST_H_

#include <initializer_list>
#include <iterator>
#include <limits>

namespace s21 {

struct ListNodeBase {
  ListNodeBase() noexcept : prev(this), next(this) {}

  ListNodeBase(ListNodeBase* const o_prev, ListNodeBase* const o_next) noexcept
    : prev(o_prev), next(o_next) {
    prev->next = next->prev = this;
  }

  virtual ~ListNodeBase() noexcept {
    prev->next = next;
    next->prev = prev;
  }

  ListNodeBase *prev;
  ListNodeBase *next;
};

template<typename Tp>
struct ListNode : public ListNodeBase {
  Tp data;
  ListNode() = default;
  ListNode(ListNodeBase* const o_prev, ListNodeBase* const o_next, const Tp& o_data)
    : ListNodeBase(o_prev, o_next), data(o_data) {}
};

template<typename Tp>
struct ListIterator {
  using Node = ListNode<Tp>;
  using difference_type = ptrdiff_t;
  using iterator_category = std::bidirectional_iterator_tag;
  using value_type = Tp;
  using pointer = Tp*;
  using reference = Tp&;

  ListIterator() noexcept : node() {}

  explicit ListIterator(ListNodeBase* node) noexcept : node(node) {}

  reference operator*() const noexcept { return static_cast<Node*>(node)->data; }

  pointer operator->() const noexcept { return &static_cast<Node*>(node)->data; }

  ListIterator& operator++() noexcept {
    node = node->next;
    return *this;
  }

  ListIterator operator++(int) noexcept {
    ListIterator ret(*this);
    node = node->next;
    return ret;
  }

  ListIterator& operator--() noexcept {
    node = node->prev;
    return *this;
  }

  ListIterator operator--(int) noexcept {
    ListIterator ret(*this);
    node = node->prev;
    return ret;
  }

  friend bool operator==(const ListIterator& lhs, const ListIterator& rhs) noexcept {
    return lhs.node == rhs.node;
  }

  friend bool operator!=(const ListIterator& lhs, const ListIterator& rhs) noexcept {
    return lhs.node != rhs.node;
  }

  ListNodeBase* node;

};

template<typename Tp>
struct ListConstIterator {
  using iterator = ListIterator<Tp>;
  using Node = const ListNode<Tp>;
  using difference_type = ptrdiff_t;
  using iterator_category = std::bidirectional_iterator_tag;
  using value_type = Tp;
  using pointer = const Tp*;
  using reference = const Tp&;

  ListConstIterator() noexcept : node() {}

  explicit ListConstIterator(const ListNodeBase* node) noexcept : node(node) {}

  ListConstIterator(const iterator& it) noexcept : node(it.node) {}

  iterator const_cast_() noexcept {
    return iterator(const_cast<ListNodeBase*>(node));
  }

  reference operator*() const noexcept { return static_cast<Node*>(node)->data; }

  pointer operator->() const noexcept { return &static_cast<Node*>(node)->data; }

  ListConstIterator& operator++() noexcept {
    node = node->next;
    return *this;
  }

  ListConstIterator operator++(int) noexcept {
    ListConstIterator ret(*this);
    node = node->next;
    return ret;
  }

  ListConstIterator& operator--() noexcept {
    node = node->prev;
    return *this;
  }

  ListConstIterator operator--(int) noexcept {
    ListConstIterator ret(*this);
    node = node->prev;
    return ret;
  }

  friend bool operator==(const ListConstIterator& lhs, const ListConstIterator& rhs) noexcept {
    return lhs.node == rhs.node;
  }

  friend bool operator!=(const ListConstIterator& lhs, const ListConstIterator& rhs) noexcept {
    return lhs.node != rhs.node;
  }

  const ListNodeBase* node;
};

template<typename Tp>
class list {
 public:
  using value_type = Tp;
  using reference = Tp&;
  using const_reference = const Tp&;
  using pointer = Tp*;
  using const_pointer = const Tp*;
  using Node = ListNode<value_type>;
  using iterator = ListIterator<value_type>;
  using const_iterator = ListConstIterator<value_type>;
  using size_type = size_t;

  list() : base_(new Node()) {}

  explicit list(size_type n) : list()  {
    for (;n != 0; --n) {
      emplace_back();
    }
  }

  list(std::initializer_list<value_type> const& items) : list()  {
    for (auto& item : items) {
      push_back(item);
    }
  }

  list(const list& other) : list()  {
    for (auto& val : other) {
      push_back(val);
    }
  }

  list(list&& other) noexcept : size_(other.size_), base_(other.base_) {
    other.base_ = new Node();
    other.size_ = 0;
  }

  ~list() {
    clear();
    delete base_;
  }

  list& operator=(const list& other) {
    if (this != &other) {
      list tmp(other);
      swap(tmp);
    }
    return *this;
  }

  list& operator=(list&& other) noexcept {
    list tmp(std::move(other));
    if (this != &other) {
      swap(tmp);
    }
    return *this;
  }

  void clear() {
    while (!empty()) {
      ListNodeBase *next = base_->next->next;
      delete base_->next;
      base_->next = next;
    }
    size_ = 0;
  }

  bool empty() const noexcept { return base_ == base_->next; }

  void erase(iterator pos) {
    delete pos.node;
    --size_;
  }

  void pop_front() {
    erase(begin());
  };

  void pop_back() {
    erase(tail());
  }

  void merge(list& other) {
    if (this == &other) {
      return;
    }
    list tmp(size() + other.size());
    auto it_tmp = tmp.begin();
    for (;;) {
      if (empty() && other.empty()) {
        break;
      } else if (other.empty() || front() < other.front()) {
        *it_tmp = std::move(front());
        pop_front();
      } else {
        *it_tmp = std::move(other.front());
        other.pop_front();
      }
      it_tmp++;
    }
    swap(tmp);
  }

  void splice(const_iterator pos, list& other) {
    ListNodeBase* node = pos.const_cast_().node;

    node->prev->next = other.base_->next;
    other.base_->next->prev = node->prev;

    node->prev = other.base_->prev;
    other.base_->prev->next = node;

    other.base_->next = other.base_;
    other.base_->prev = other.base_;

    size_ += other.size_;
    other.size_ = 0;
  }

  void reverse() noexcept {
    std::swap(base_->next, base_->prev);
    for(auto it = begin(); it != end(); ++it) {
      std::swap(it.node->next, it.node->prev);
    }
  }

  size_type size() const noexcept {
    return size_;
  }

  size_type max_size() const noexcept {
    return std::numeric_limits<size_type>::max()/2/sizeof(ListNodeBase);
  }

  void sort() noexcept {
    qsort(begin(), tail());
  }

  void unique() {
    iterator curr = begin();
    iterator next = curr;
    while (next != end()) {
      ++next;
      if (*next == *curr) {
        iterator tmp = next--;
        erase(tmp);
      }
      if (next == tail()) {
        next = ++curr;
      }
    }
  }

  reference front() noexcept {
    return *begin();
  }

  const_reference front() const noexcept {
    return *begin();
  }

  reference back() noexcept {
    return *iterator(base_->prev);
  }

  const_reference back() const noexcept {
    return *const_iterator(base_->prev);
  }

  iterator insert(iterator pos, const_reference value) {
    ++size_;
    return iterator(new Node(pos.node->prev, pos.node, value));
  }

  template <typename... Args>
  iterator insert_many(iterator pos,Args&&... args) {
    ((pos = insert(pos, args),++pos),...);
    return pos;
  }

  template <typename... Args>
  void insert_many_back(Args&&... args) {
    (push_back(args), ...);
  }

  template <typename... Args>
  void insert_many_front(Args&&... args) {
    (push_front(args), ...);
  }

  void push_front(const_reference value) {
    ++size_;
    new Node(base_, base_->next, value);
  }

  template <typename... Args>
  void emplace_back(Args&&... args) {
    emplace(cend(), std::forward<Args>(args)...);
  }

  template <typename... Args>
  void emplace_front(Args&&... args) {
    emplace(cbegin(), std::forward<Args>(args)...);
  }

  template <typename... Args>
  iterator emplace(const_iterator pos, Args&&... args) {
    iterator place = pos.const_cast_();
    auto node_ptr = static_cast<Node*>(::operator new(sizeof(Node)));
    new (node_ptr) ListNodeBase(ListNodeBase(place.node->prev, place.node));
    new (&node_ptr->data) value_type(std::forward<Args>(args)...);
    ++size_;
    return iterator(node_ptr);
  }

  void swap(list& other) noexcept {
    std::swap(base_, other.base_);
    std::swap(size_, other.size_);
  }

  void push_back(const_reference value) {
    ++size_;
    new Node(base_->prev, base_, value);
  }

  iterator begin() noexcept {
    if (!base_) {
      return iterator();
    }
    return iterator(base_->next);
  }

  iterator end() noexcept {
    return iterator(base_);
  }

  const_iterator begin() const noexcept {
    return const_iterator(base_->next);
  }

  const_iterator end() const noexcept {
    return const_iterator(base_);
  }

  const_iterator cbegin() const noexcept {
    return const_iterator(base_->next);
  }

  const_iterator cend() const noexcept {
    return const_iterator(base_);
  }

 private:
  size_type size_{};
  ListNodeBase* base_;

  iterator tail() noexcept {
    return iterator(base_->prev);
  }

  void qsort(iterator left, iterator right) noexcept {
    if (left == right || left == end() || right == end() || left == tail()) {
      return;
    }
    iterator pivot = partition(left, right);
    qsort(left, --pivot);
    qsort(++pivot, right);
  }

  iterator partition(iterator left, iterator right) noexcept {
    iterator i = left;
    for (iterator j = i; j != right; ++j) {
      if (*j <= *right) {
        std::swap(*i, *j);
        ++i;
      }
    }
    std::swap(*i, *right);
    return i;
  }
};

} // namespace s21

#endif // S21_CONTAINER_SRC_S21_LIST_H_