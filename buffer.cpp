#include "buffer.hpp"
#include <cstring>
#include <cassert>

Buffer::Buffer(size_t size)
    : buffer_begin_(new uint8_t[size]),
      buffer_end_(buffer_begin_.get() + size),
      data_begin_(buffer_begin_.get()),
      data_end_(buffer_begin_.get()),
      buffer_size_(size) {}

Buffer::~Buffer() {}

void Buffer::append(const uint8_t* data, size_t len) {
    ensure_capacity(size() + len);
    std::memcpy(data_end_, data, len);
    data_end_ += len;
}

void Buffer::consume(size_t n) {
    assert(n <= size());

    data_begin_ += n;
    if (data_begin_ == data_end_) {
        data_begin_ = buffer_begin_.get();
        data_end_ = buffer_begin_.get();
    }
}

bool Buffer::empty() const {
    return data_begin_ == data_end_;
}

const uint8_t* Buffer::data() const {
    return data_begin_;
}

size_t Buffer::size() const {
    return static_cast<size_t>(data_end_ - data_begin_);
}

void Buffer::ensure_capacity(size_t new_size) {
    // back space
    size_t used = size();
    size_t back_space = static_cast<size_t>(buffer_end_ - data_end_);
    // enough space
    if (back_space >= new_size)
        return;

    size_t front_space = static_cast<size_t>(data_begin_ - buffer_begin_.get());
    if (front_space + back_space >= new_size) {
        // move data to the front of the buffer
        std::memmove(buffer_begin_.get(), data_begin_, used);
        data_begin_ = buffer_begin_.get();
        data_end_ = buffer_begin_.get() + used;
        return;
    }

    size_t new_buffer_size = buffer_size_ * 2;
    // making sure the new buffer size is at least as large as the requested new size
    while (new_buffer_size < new_size) {
        new_buffer_size *= 2;
    }

    std::unique_ptr<uint8_t[]> new_buffer(new uint8_t[new_buffer_size]);
    size_t data_len = size();
    std::memcpy(new_buffer.get(), data_begin_, data_len);
    buffer_begin_ = std::move(new_buffer);

    buffer_end_ = buffer_begin_.get() + new_buffer_size;
    data_begin_ = buffer_begin_.get();
    data_end_ = buffer_begin_.get() + data_len;
    buffer_size_ = new_buffer_size;
}