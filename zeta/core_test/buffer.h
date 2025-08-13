#pragma once

#include <utility>

class Buffer {
public:
    Buffer() : buffer_{ nullptr }, size_{ 0 } {}

    ~Buffer() { std::free(this->buffer_); }

    template <typename Value>
    Value* GetBuffer(size_t size) {
        if (this->size_ < size) {
            std::free(this->buffer_);

            this->buffer_ = std::malloc(size);
            this->size_ = size;
        }

        return static_cast<Value*>(this->buffer_);
    }

private:
    void* buffer_;
    size_t size_;
};
