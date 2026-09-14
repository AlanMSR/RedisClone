#pragma once

#include <cstdint>
#include <memory>

class Buffer {
public:
    Buffer(size_t size);
    ~Buffer();

    void append(const uint8_t* data, size_t len);
    void consume(size_t n);
    bool empty() const;
    const uint8_t* data() const;
    uint8_t* data();
    void resize(size_t new_size);
    size_t size() const;

private:
    void ensure_capacity(size_t new_size);

private:
    std::unique_ptr<uint8_t[]> buffer_begin_;
    uint8_t* buffer_end_;
    uint8_t* data_begin_;
    uint8_t* data_end_;
    size_t buffer_size_;
};