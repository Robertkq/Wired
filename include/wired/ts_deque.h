#ifndef WIRED_TS_DEQUE_H
#define WIRED_TS_DEQUE_H

#include <algorithm>
#include <deque>
#include <mutex>

namespace wired {

/**
 * @brief Thread-safe deque implementation
 * This class is a deque wrapper that provides thread-safety
 * use of variadic args to easily mimic deque's interface
 *
 * to ensure thread safe iteration over the deque, use the for_each method
 */
template <typename T>
class ts_deque {
  public:
    using iterator = typename std::deque<T>::iterator;
    using reference = typename std::deque<T>::reference;
    using const_reference = typename std::deque<T>::const_reference;
    using size_type = typename std::deque<T>::size_type;

  public:
    ts_deque();
    ts_deque(const ts_deque& other);
    ts_deque(ts_deque&& other);
    ~ts_deque();

    ts_deque& operator=(const ts_deque& other);
    ts_deque& operator=(ts_deque&& other);

    bool empty() const;
    size_type size() const;
    void clear();

    void shrink_to_fit();
    template <typename... Args>
    void resize(Args&&... args);

    reference front();
    const_reference front() const;

    reference back();
    const_reference back() const;

    reference at(size_type pos);
    const_reference at(size_type pos) const;
    reference operator[](size_type pos);
    const_reference operator[](size_type pos) const;

    template <typename Func>
    void for_each(Func func);

    template <typename... Args>
    iterator insert(Args&&... args);
    template <typename... Args>
    void push_back(Args&&... args);
    template <typename... Args>
    void push_front(Args&&... args);

    template <typename... Args>
    iterator emplace(Args&&... args);
    template <typename... Args>
    void emplace_back(Args&&... args);
    template <typename... Args>
    void emplace_front(Args&&... args);

    template <typename... Args>
    iterator erase(Args&&... args);
    void pop_back();
    void pop_front();

  private:
    std::deque<T> deque_;
    mutable std::mutex mutex_;
}; // class ts_deque

template <typename T>
ts_deque<T>::ts_deque() : deque_(), mutex_() {}

template <typename T>
ts_deque<T>::ts_deque(const ts_deque& other) {
    std::lock_guard<std::mutex> lock(other.mutex_);
    deque_ = other.deque_;
}

template <typename T>
ts_deque<T>::ts_deque(ts_deque&& other)
    : deque_(std::move(other.deque_)), mutex_() {}

template <typename T>
ts_deque<T>::~ts_deque() {}

template <typename T>
ts_deque<T>& ts_deque<T>::operator=(const ts_deque& other) {
    if (this == &other) {
        return *this;
    }

    std::lock(mutex_, other.mutex_);
    std::lock_guard<std::mutex> lock1(mutex_, std::adopt_lock);
    std::lock_guard<std::mutex> lock2(other.mutex_, std::adopt_lock);

    deque_ = other.deque_;

    return *this;
}

template <typename T>
ts_deque<T>& ts_deque<T>::operator=(ts_deque&& other) {
    if (this == &other) {
        return *this;
    }

    std::lock(mutex_, other.mutex_);
    std::lock_guard<std::mutex> lock1(mutex_, std::adopt_lock);
    std::lock_guard<std::mutex> lock2(other.mutex_, std::adopt_lock);

    deque_ = std::move(other.deque_);

    return *this;
}

template <typename T>
bool ts_deque<T>::empty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return deque_.empty();
}

template <typename T>
typename ts_deque<T>::size_type ts_deque<T>::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return deque_.size();
}

template <typename T>
void ts_deque<T>::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    deque_.clear();
}

template <typename T>
void ts_deque<T>::shrink_to_fit() {
    std::lock_guard<std::mutex> lock(mutex_);
    deque_.shrink_to_fit();
}

template <typename T>
template <typename... Args>
void ts_deque<T>::resize(Args&&... args) {
    std::lock_guard<std::mutex> lock(mutex_);
    deque_.resize(std::forward<Args>(args)...);
}

template <typename T>
typename ts_deque<T>::reference ts_deque<T>::front() {
    std::lock_guard<std::mutex> lock(mutex_);
    return deque_.front();
}

template <typename T>
typename ts_deque<T>::const_reference ts_deque<T>::front() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return deque_.front();
}

template <typename T>
typename ts_deque<T>::reference ts_deque<T>::back() {
    std::lock_guard<std::mutex> lock(mutex_);
    return deque_.back();
}

template <typename T>
typename ts_deque<T>::const_reference ts_deque<T>::back() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return deque_.back();
}

template <typename T>
typename ts_deque<T>::reference ts_deque<T>::at(size_type pos) {
    std::lock_guard<std::mutex> lock(mutex_);
    return deque_.at(pos);
}

template <typename T>
typename ts_deque<T>::const_reference ts_deque<T>::at(size_type pos) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return deque_.at(pos);
}

template <typename T>
typename ts_deque<T>::reference ts_deque<T>::operator[](size_type pos) {
    std::lock_guard<std::mutex> lock(mutex_);
    return deque_[pos];
}

template <typename T>
typename ts_deque<T>::const_reference
ts_deque<T>::operator[](size_type pos) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return deque_[pos];
}

template <typename T>
template <typename Func>
void ts_deque<T>::for_each(Func func) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& elem : deque_) {
        func(elem);
    }
}

template <typename T>
template <typename... Args>
typename ts_deque<T>::iterator ts_deque<T>::insert(Args&&... args) {
    std::lock_guard<std::mutex> lock(mutex_);
    return deque_.insert(std::forward<Args>(args)...);
}

template <typename T>
template <typename... Args>
void ts_deque<T>::push_back(Args&&... args) {
    std::lock_guard<std::mutex> lock(mutex_);
    deque_.push_back(std::forward<Args>(args)...);
}

template <typename T>
template <typename... Args>
void ts_deque<T>::push_front(Args&&... args) {
    std::lock_guard<std::mutex> lock(mutex_);
    deque_.push_front(std::forward<Args>(args)...);
}

template <typename T>
template <typename... Args>
typename ts_deque<T>::iterator ts_deque<T>::emplace(Args&&... args) {
    std::lock_guard<std::mutex> lock(mutex_);
    return deque_.emplace(std::forward<Args>(args)...);
}

template <typename T>
template <typename... Args>
void ts_deque<T>::emplace_back(Args&&... args) {
    std::lock_guard<std::mutex> lock(mutex_);
    deque_.emplace_back(std::forward<Args>(args)...);
}

template <typename T>
template <typename... Args>
void ts_deque<T>::emplace_front(Args&&... args) {
    std::lock_guard<std::mutex> lock(mutex_);
    deque_.emplace_front(std::forward<Args>(args)...);
}

template <typename T>
template <typename... Args>
typename ts_deque<T>::iterator ts_deque<T>::erase(Args&&... args) {
    std::lock_guard<std::mutex> lock(mutex_);
    return deque_.erase(std::forward<Args>(args)...);
}

template <typename T>
void ts_deque<T>::pop_back() {
    std::lock_guard<std::mutex> lock(mutex_);
    deque_.pop_back();
}

template <typename T>
void ts_deque<T>::pop_front() {
    std::lock_guard<std::mutex> lock(mutex_);
    deque_.pop_front();
}

} // namespace wired

#endif // WIRED_TS_DEQUE_H