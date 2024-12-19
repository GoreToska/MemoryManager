#pragma once

class os_allocator
{
public:
    ~os_allocator();
    void* allocate(const size_t size);
    void free(void* ptr);
    void init();
    void destroy();
    void* get_start_pointer() const { return start_pointer_; }
    void* get_end_pointer();

private:
    struct os_block
    {
        size_t size;
        os_block* next;
        os_block* previous;
    };

    os_block* start_pointer_ = nullptr;
    bool is_initialized_ = false;
};
