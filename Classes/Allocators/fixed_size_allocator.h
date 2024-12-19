#pragma once

#include "allocator_base.h"
#include "../Containers/linked_stack.h"

class fixed_size_allocator : public allocator_base
{
public:
    fixed_size_allocator(const size_t total_size, const size_t chunk_size);
    ~fixed_size_allocator() override;

    void* allocate(const size_t size, const size_t alignment = 0) override;
    void free(void* ptr) override;
    void init() override;
    virtual void reset();
    void destroy() override;
    size_t get_chunk_size() const { return chunk_size_; }

private:
    struct fixed_free_header
    {
    };

    using node = linked_stack<fixed_free_header>::stack_node;
    linked_stack<fixed_free_header> free_list_;
    size_t chunk_size_;
};
