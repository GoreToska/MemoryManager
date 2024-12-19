#include "os_allocator.h"

#include <assert.h>
#include <windows.h>

os_allocator::~os_allocator()
{
    assert(is_initialized_ == false && "OS allocator must be destroyed first");
}

void* os_allocator::allocate(const size_t size)
{
    assert(size >= 1e+7 && "OS allocator should be used for >=10mb only");

    void* ptr = VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    os_block* block = static_cast<os_block*>(ptr);
    block->size = size;

    if (start_pointer_ == nullptr)
    {
        start_pointer_ = block;
    }
    else
    {
        os_block* new_block = start_pointer_;
        while (new_block->next)
        {
            new_block = new_block->next;
        }

        new_block->next = block;
    }

    return ptr;
}

void os_allocator::free(void* ptr)
{
    assert(ptr != nullptr && "Null pointer");
    os_block* block = static_cast<os_block*>(ptr);
    assert(block != nullptr && "This is not OS allocated memory");

    if (block->next)
        block->next->previous = block->previous;

    VirtualFree(ptr, block->size, MEM_RELEASE);
}

void os_allocator::init()
{
    is_initialized_ = true;
}

void os_allocator::destroy()
{
    os_block* block = start_pointer_;
    while (block->next)
    {
        os_block* next = block->next;
        VirtualFree(block, 0, MEM_RELEASE);
        block = next;
    }

    start_pointer_ = nullptr;
    is_initialized_ = false;
}

void* os_allocator::get_end_pointer()
{
    os_block* block = start_pointer_;
    while (block->next)
    {
        os_block* next = block->next;
        block = next;
    }

    return static_cast<void*>(block);
}
