#ifndef S21_CONTAINER_SRC_S21_MAP_H_
#define S21_CONTAINER_SRC_S21_MAP_H_

#include "s21_tree.h"
#include "s21_vector.h"

namespace s21 {

template<typename Key, typename Tp, typename Compare = std::less<Key>>
class map {
 public:
  using key_type = Key;
  using mapped_type = Tp;
  using value_type = std::pair<key_type, mapped_type>;
  using reference = value_type&;
  using const_reference = const value_type&;
  using tree = RBTree<key_type, value_type, KeyGetters::SelectFirst<value_type>, Compare>;
  using iterator = typename tree::iterator;
  using const_iterator = typename tree::const_iterator;
  using size_type = typename tree::size_type;

  map() = default;

  map(std::initializer_list<value_type> const& items) {
    for(auto& item : items) {
      tree_.insert_unique(item);
    }
  }

  map(const map& other) noexcept : tree_(other.tree_) {}

  map(map&& other) noexcept : tree_(std::move(other.tree_)) {}

  ~map() = default;

  map& operator=(map&& other)  noexcept {
    tree_ = std::move(other.tree_);
    return *this;
  }

  map& operator=(const map& other) {
    tree_ = other.tree_;
    return *this;
  }

  mapped_type& at(const Key& key) {
    iterator it = tree_.lower_bound(key);
    if (it == end() || Compare()(key, (*it).first)) {
      throw std::out_of_range("Missing key in map");
    }
    return (*it).second;
  }

  const mapped_type& at(const Key& key) const {
    const_iterator it = tree_.lower_bound(key);
    if (it == end() || Compare()(key, (*it).first)) {
      throw std::out_of_range("Missing key in map");
    }
    return (*it).second;
  }

  mapped_type& operator[](const Key& key) {
    iterator it = tree_.lower_bound(key);
    if (it == end() || Compare()(key, (*it).first)) {
      auto ins_it = insert(value_type(key, mapped_type())).first;
      return (*ins_it).second;
    }
    return (*it).second;
  }

  const mapped_type& operator[](const Key& key) const {
    const_iterator it = tree_.lower_bound(key);
    if (it == end() || Compare()(key, (*it).first)) {
      auto ins_it = insert(value_type(key, mapped_type())).first;
      return (*ins_it).second;
    }
    return (*it).second;
  }

  void clear() {
    tree_.clear();
  }

   bool empty() const noexcept {
    return tree_.empty();
  }

  size_type size() const noexcept {
    return tree_.size();
  }

  size_type max_size() const noexcept {
    return tree_.max_size();
  }

  std::pair<iterator, bool> insert(const value_type& value) {
    return tree_.insert_unique(value);
  }

  template <typename... Args>
  vector<std::pair<iterator,bool>> insert_many(Args&&... args) {
    vector<std::pair<iterator, bool>> inserted;
    std::pair<iterator, bool> ins_pair;
    ((ins_pair = insert(args),
      inserted.push_back(ins_pair)), ...);
    return inserted;
  }

  void erase(iterator pos) {
    tree_.erase((*pos).first);
  }

  void swap(map& other) noexcept {
    tree_.swap(other.tree_);
  }

  void merge(map& other) {
    tree_.merge_unique(other.tree_);
  }

  std::pair<iterator, bool> insert(const Key& key, const Tp& obj) {
    return tree_.insert_unique(value_type(key, obj));
  }

  std::pair<iterator, bool> insert_or_assign(const Key& key, const Tp& obj) {
    iterator it = tree_.lower_bound(key);
    if (it == end() || Compare()(key, (*it).first)) {
      tree_.insert_unique(value_type(key, obj));
      return std::make_pair(it, true);
    }
    (*it).second = obj;
    return std::pair<iterator, bool>(it, false);
  }

  bool contains(const Key& key) const noexcept {
    return tree_.count(key) != 0;
  }

  iterator begin() noexcept {
    return tree_.begin();
  }

  const_iterator begin() const noexcept {
    return tree_.cbegin();
  }

  const_iterator cbegin() const noexcept {
    return tree_.cbegin();
  }

  iterator end() noexcept {
    return tree_.end();
  }

  const_iterator end() const noexcept {
    return tree_.cend();
  }

  const_iterator cend() const noexcept {
    return tree_.cend();
  }

 private:
  tree tree_{};
};

} //namespace s21

#endif // S21_CONTAINER_SRC_S21_MAP_H_