#ifndef S21_CONTAINER_SRC_S21_VECTOR_H_
#define S21_CONTAINER_SRC_S21_VECTOR_H_

#include <memory>
#include <initializer_list>
#include <stdexcept>

namespace s21 {

template<typename Tp, typename Alloc = std::allocator<Tp>>
class vector {
 public:
  using value_type = Tp;
  using iterator = Tp*;
  using const_iterator = const Tp*;
  using reference = Tp&;
  using const_reference = const Tp&;
  using size_type = size_t;
  using allocator_type = Alloc;

  vector() = default;

  explicit vector(size_type n) {
    create_storage(n);
    for(;n != 0; --n) {
      allocator_.construct(finish_++);
    }
  }

  vector(std::initializer_list<value_type> const& items) {
    create_storage(items.size());
    for (auto& item : items) {
      allocator_.construct(finish_++, item);
    }
  }

  vector(const vector& other) {
    create_storage(other.capacity());
    for (auto& val : other) {
      allocator_.construct(finish_++, val);
    }
  }

  vector(vector&& other) noexcept
    : start_(other.start_),finish_(other.finish_),
      capacity_(other.capacity_), allocator_(std::move(other.allocator_)) {
    other.start_ = other.finish_ = other.capacity_ = nullptr;
  }

  ~vector() {
    destroy_storage();
  }

  vector& operator=(const vector& other) {
    if (this != &other) {
      vector tmp(other);
      swap(tmp);
    }
    return *this;
  }

  vector& operator=(vector&& other)  noexcept {
    vector tmp(std::move(other));
    if (this != &other) {
      swap(tmp);
    }
    return *this;
  }

  const Tp* data() const noexcept {
    return start_;
  }

  iterator begin() noexcept { return start_; }

  const_iterator begin() const noexcept { return start_; }

  const_iterator cbegin() const noexcept {
    return start_;
  }

  iterator end() noexcept { return finish_; }

  const_iterator end() const noexcept { return finish_; }

  const_iterator cend() const noexcept {
    return finish_;
  }

  size_type size() const noexcept {
    return finish_ - start_;
  }

  size_type max_size() const noexcept {
    return allocator_.max_size();
  }

  bool empty() const noexcept {
    return begin() == end();
  }

  size_type capacity() const noexcept {
    return capacity_ - start_;
  }

  void clear() noexcept {
    for(iterator p = begin(); p != end(); ++p) {
      allocator_.destroy(p);
    }
    finish_ = start_;
  }

  void reserve(size_type n) {
    if (capacity() > max_size()) {
      throw std::out_of_range("Invalid reserve size");
    }
    if (capacity() < n) {
      reallocate_storage(end(), n);
    }
  }

  reference at(size_type pos) {
    if (pos >= size()) {
      throw std::out_of_range("Invalid position number");
    }
    return (*this)[pos];
  }

  const_reference at(size_type pos) const {
    if (pos >= size()) {
      throw std::out_of_range("Invalid position number");
    }
    return (*this)[pos];
  }

  reference operator[](size_type pos) noexcept { return *(start_ + pos); }

  const_reference operator[](size_type pos) const noexcept { return *(start_ + pos); }

  reference front() { return *begin(); }

  const_reference front() const { return *begin(); }

  reference back() { return *(end() - 1); }

  const_reference back() const { return *(end() - 1); }

  template <typename... Args>
  iterator emplace(const_iterator pos, Args&&... args) {
    auto offset = pos - cbegin();
    iterator new_pos = make_gap(offset);
    allocator_.construct(new_pos, std::forward<Args>(args)...);
    ++finish_;
    return new_pos;
  }

  template <typename... Args>
  void emplace_back(Args&&... args) {
    emplace(cend(), std::forward<Args>(args)...);
  }

  iterator insert(iterator pos, const_reference value) {
    auto offset = pos - begin();
    iterator new_pos = make_gap(offset);
    allocator_.construct(new_pos, value);
    ++finish_;
    return new_pos;
  }

  template <typename... Args>
  iterator insert_many(iterator pos, Args&&... args) {
    ((pos = insert(pos, args),++pos), ...);
    return pos;
  }

  template <typename... Args>
  void insert_many_back(Args&&... args) {
    (push_back(args), ...);
  }

  void push_back(const_reference value) {
    insert(end(), value);
  }

  void pop_back() {
    --finish_;
    allocator_.destroy(end());
  }

  void shrink_to_fit() {
    if (!storage_is_full() && !empty()) {
      shrink_storage(size());
    }
  }

  void erase(iterator pos) {
    if (pos + 1 != end()) {
      std::move(pos + 1, end(), pos);
    }
    pop_back();
  }

  void swap(vector& other) noexcept {
    std::swap(start_, other.start_);
    std::swap(finish_, other.finish_);
    std::swap(capacity_, other.capacity_);
    std::swap(allocator_, other.allocator_);
  }

 private:
  iterator make_gap(size_type offset) {
    if (storage_is_full()) {
      grow_storage();
    }
    iterator new_pos = begin() + offset;
    std::move(new_pos, end(), new_pos + 1);
    return new_pos;
  }

  bool storage_is_full() noexcept {
    return finish_ == capacity_;
  }

  void reallocate_storage(iterator end_of_storage, size_type new_cap) {
    size_type old_size = size();
    size_type new_size = (new_cap < old_size) ? new_cap : old_size;
    iterator new_start = allocator_.allocate(new_cap);
    std::move(begin(), end_of_storage, new_start);
    destroy_storage();
    start_ = new_start;
    finish_ = start_ + new_size;
    capacity_ = start_ + new_cap;
  }

  void grow_storage() {
    size_type old_cap = capacity();
    size_type new_cap = (old_cap == 0) ? 1 : old_cap * 2;
    reallocate_storage(end(), new_cap);
  }

  void shrink_storage(size_type new_cap) {
    reallocate_storage(begin() + new_cap, new_cap);
  }

  void create_storage(size_type n) {
    start_ = (n != 0) ? allocator_.allocate(n) : nullptr;
    finish_ = start_;
    capacity_ = start_ + n;
  }

  void destroy_storage() {
    clear();
    allocator_.deallocate(start_, capacity_ - start_);
    start_ = finish_ = capacity_ = nullptr;
  }

  iterator start_{},
           finish_{},
           capacity_{};
  allocator_type allocator_;

};

} // namespace s21

#endif // S21_CONTAINER_SRC_S21_VECTOR_H_