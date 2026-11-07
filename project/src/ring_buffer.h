#ifndef NANO_EDR_RING_BUFFER_H
#define NANO_EDR_RING_BUFFER_H

#include <array>
#include <cstddef>

namespace nano_edr {

template <typename T, std::size_t N>
class RingBuffer {
 public:
    class Iterator {
     public:
        Iterator(const RingBuffer* buffer, std::size_t index, std::size_t remaining): buffer_(buffer), index_(index), remaining_(remaining) {}

        const T& operator*() const {
            return (*buffer_)[index_];
        }

        Iterator& operator++(){
            ++index_;
            --remaining_;
            return *this;
        }

        bool operator!=(const Iterator& other) const {
            return remaining_ != other.remaining_;
        }

     private:
        const RingBuffer* buffer_ = nullptr;
        std::size_t index_ = 0;
        std::size_t remaining_ = 0;
    };

    void PushBack(const T& value){
        c_[index_] = value;
        index_ = (index_ + 1) % N;
        if (index_ == 0){
            is_full_ = true;
        }
    }

    void Clear(){
        c_ = {};
        index_ = 0;
        is_full_ = false;
    }

    std::size_t size() const {
        return is_full_ ? N : index_;
    }

    static constexpr std::size_t capacity(){
        return N;
    }

    bool empty() const {
        return !is_full_ && index_ == 0;
    }

    bool full() const {
        return is_full_;
    }

    const T& operator[](std::size_t index) const {
        if (is_full_) {
            return c_[(index_ + index) % N];
        }
        return c_[index];
    }

    const T& back() const {
        if (is_full_) {
            return c_[(index_ + N - 1) % N];
        }
        return c_[index_ - 1];
    }

    Iterator begin() const {
        return Iterator(this, 0, size());
    }

    Iterator end() const {
        return Iterator(this, size(), 0);
    }

 private:
    std::array<T, N> c_;
    std::size_t index_ = 0;
    bool is_full_ = false;
};

}

#endif