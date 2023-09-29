#ifndef S21_CONTAINER_SRC_S21_SET_H_
#define S21_CONTAINER_SRC_S21_SET_H_

#include <initializer_list>
#include <limits>

#include "s21_tree.h"
#include "s21_vector.h"

namespace s21 {

template<typename Key, typename Compare = std::less<Key>>
class set {
 public:
  using key_type = Key;
  using value_type = Key;
  using reference = Key&;
  using const_reference = const Key&;
  using tree = RBTree<key_type, value_type, KeyGetters::Identity<value_type>, Compare>;
  using iterator = typename tree::iterator;
  using const_iterator = typename tree::const_iterator;
  using size_type = typename tree::size_type;

  set() = default;

  set(std::initializer_list<value_type> const& items) {
    for(auto& item : items) {
      tree_.insert_unique(item);
    }
  }

  set(const set& other) noexcept : tree_(other.tree_) {}

  set(set&& other) noexcept : tree_(std::move(other.tree_)) {}

  ~set() = default;

  set& operator=(set&& other) noexcept {
    tree_ = std::move(other.tree_);
    return *this;
  }

  set& operator=(const set& other) {
    tree_ = other.tree_;
    return *this;
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
    tree_.erase(*pos);
  }

  void swap(set& other) noexcept {
    tree_.swap(other.tree_);
  }

  void merge(set& other) {
    tree_.merge_unique(other.tree_);
  }

  iterator find(const Key& key) noexcept {
    return tree_.find(key);
  }

  const_iterator find(const Key& key) const noexcept {
    return tree_.find(key);
  }

  bool contains(const Key& key) const noexcept {
    return tree_.find(key) != end();
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

} // namespace s21

#endif // S21_CONTAINER_SRC_S21_SET_H_