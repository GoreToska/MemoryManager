#pragma once

class allocator_base
{
public:
    allocator_base(const size_t totalSize) : total_size_{totalSize}, used_{0}
    {
    }

    virtual ~allocator_base() { total_size_ = 0; }
    virtual void* allocate(const size_t size, const size_t alignment = 0) = 0;
    virtual void free(void* ptr) = 0;
    virtual void init() = 0;
    virtual void destroy() = 0;
    void* get_start_pointer() const { return start_ptr_; }
    size_t get_total_size() const { return total_size_; }

protected:
    void* start_ptr_ = nullptr;
    bool is_initialized_;
    size_t total_size_;
    size_t used_;
};
