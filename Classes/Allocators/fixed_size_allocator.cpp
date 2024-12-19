#include "fixed_size_allocator.h"
#include <windows.h>
#include <iostream>
#include <cassert>

fixed_size_allocator::fixed_size_allocator(const std::size_t total_size, const std::size_t chunk_size)
    : allocator_base(total_size)
{
    assert(chunk_size >= 8 && "Chunk size must be greater or equal to 8");
    assert(total_size % chunk_size == 0 && "Total size must be a N count of chunk size");
    chunk_size_ = chunk_size;
}

fixed_size_allocator::~fixed_size_allocator()
{
    //assert(!is_initialized_ && "Fixed size allocator must be destroyed before destructor");
}

void* fixed_size_allocator::allocate(const size_t allocation_size, const size_t alignment)
{
    node* free_position = free_list_.pop();
    assert(free_position != nullptr && "Fixed size allocator is full");

    used_ += chunk_size_;

#ifdef _DEBUG
    std::cout << "FSR allocated" << "\n\tStart " << start_ptr_ << "\n\tPointer " << static_cast<void*>(
            free_position)
        << "\n\tMemory used " << used_ << std::endl;
#endif

    return static_cast<void*>(free_position);
}

void fixed_size_allocator::free(void* ptr)
{
    used_ -= chunk_size_;
    free_list_.push(static_cast<node*>(ptr));
}

void fixed_size_allocator::init()
{
    start_ptr_ = VirtualAlloc(nullptr, total_size_, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    reset();
    is_initialized_ = true;
}

void fixed_size_allocator::reset()
{
    used_ = 0;
    // Create a linked-list with all free positions
    const int chunks_count = total_size_ / chunk_size_;
    for (int i = chunks_count; i > 0; --i)
    {
        size_t address = reinterpret_cast<size_t>(start_ptr_) + i * chunk_size_;
        free_list_.push(reinterpret_cast<node*>(address));
    }
}

void fixed_size_allocator::destroy()
{
    assert(used_ == 0 && "Pool allocator memory leak");
    reset();
    VirtualFree(start_ptr_, total_size_, MEM_RELEASE);
    is_initialized_ = false;
}
