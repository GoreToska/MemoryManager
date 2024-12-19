#pragma once
#include <array>
#include <vector>

#include "Allocators/coalesce_allocator.h"
#include "Allocators/fixed_size_allocator.h"
#include "Allocators/os_allocator.h"


class coalesce_allocator;
class fixed_size_allocator;
class os_allocator;

class memory_allocator
{
public:
    memory_allocator();
    virtual ~memory_allocator();

    virtual void init();
    virtual void destroy();
    virtual void* alloc(size_t size, size_t alignment = 0);
    virtual void free(void* ptr);
    virtual void dump_stat() const;
    virtual void dump_blocks() const;

private:
    std::vector<fixed_size_allocator*> fsa_allocators_;
    coalesce_allocator* coalesce_allocator_;
    os_allocator* os_allocator_;
    
    bool is_initialized_ = false;
};
