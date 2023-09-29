#ifndef S21_CONTAINER_SRC_S21_ARRAY_H_
#define S21_CONTAINER_SRC_S21_ARRAY_H_

#include <iostream>
#include <stdexcept>
#include <utility>

namespace s21 {

template<typename T, size_t N> 
class array {
public:
    using value_type = T;
    using size_type = size_t;

    using reference = T &;
    using const_reference = const T &;
    
    using iterator = T *;
    using const_iterator = const T *;

    array() = default;
    ~array() = default;
    array(std::initializer_list<value_type> const &items) {
        if (items.size() != N) {
            throw std::out_of_range("Initializer list size does not match array size");
        }
        std::copy(items.begin(), items.end(), data_);
    }

    // конструктор копирования
    array(const array &a) noexcept {
        for (size_type i = 0; i < N; ++i) {
            data_[i] = a.data_[i];
        }
    }
    // конструктор перемещения
    array(array &&a) noexcept {
        std::move(a.begin(), a.end(), this->data_);
    }
    
    // оператор перемещения
    array& operator=(array &&a) noexcept {
        if (this != &a) {
            std::move(a.begin(), a.end(), data_);
        }
        return *this;
    }
    // оператор копирования
    array& operator=(const array &a) noexcept {
        if (this != &a) {
            std::copy(a.begin(), a.end(), data_);
        }
        return *this;
    }

    // доступ к указанному элементу с проверкой границ
    reference at(size_type pos) {
        if (pos >= size_) {
            throw std::out_of_range("Index out of range");
        } 
        return data_[pos];
    }
    const_reference at(size_type pos) const {
        if (pos >= size_) {
            throw std::out_of_range("Index out of range");
        } 
        return data_[pos];
    }

    // доступ к указанному элементу
    T& operator[](size_t index) noexcept {
        return data_[index];
    }
    const T& operator[](size_t index) const noexcept {
        return data_[index];
    }
    const_reference front() const noexcept { return data_[0]; }
    const_reference back() const noexcept { return data_[N - 1]; }

    // возвращает итератор в начало
    iterator data() noexcept { return data_;}
    const_iterator data() const noexcept { return data_;}

    iterator begin() noexcept { return data_; }
    iterator end() noexcept { return data_ + N; }
    const_iterator begin() const noexcept { return data_; }
    const_iterator end() const noexcept { return data_ + N; }

    bool empty() const noexcept { return size_ == 0; }
    size_type size() const noexcept { return N; }
    constexpr size_type max_size() const noexcept { return N; }

    void swap(array& other) {
        if (size_ != other.size_) {
            throw std::out_of_range("Arrays not equal");
        }
        std::swap(data_, other.data_);

    }
    void fill(const_reference value) {
        for (int i = 0; i < size_; ++i) {
            data_[i] = value;
        }
    }
    
private:
    size_type size_{N};
    T data_[N];
};
}   // namespace s21

#endif // S21_CONTAINER_SRC_S21_ARRAY_H_