#include "memory_allocator.h"

#include <assert.h>
#include <iostream>

#include "Allocators/coalesce_allocator.h"
#include "Allocators/fixed_size_allocator.h"
#include "Allocators/os_allocator.h"

memory_allocator::memory_allocator()
{
    fsa_allocators_ = {
        new fixed_size_allocator(64, 16),
        new fixed_size_allocator(128, 32),
        new fixed_size_allocator(256, 64),
        new fixed_size_allocator(512, 128),
        new fixed_size_allocator(1024, 256),
        new fixed_size_allocator(2048, 512)
    };

    coalesce_allocator_ = new coalesce_allocator(1e+7);
    os_allocator_ = new os_allocator();
}

memory_allocator::~memory_allocator()
{
    assert(!is_initialized_ && "Memory allocator must be destroyed first");
}

void memory_allocator::init()
{
    for (auto element : fsa_allocators_)
    {
        element->init();
    }

    coalesce_allocator_->init();
    os_allocator_->init();
    is_initialized_ = true;
}

void memory_allocator::destroy()
{
    for (auto element : fsa_allocators_)
    {
        element->destroy();
    }

    coalesce_allocator_->destroy();
    os_allocator_->destroy();
    is_initialized_ = false;
}

void* memory_allocator::alloc(size_t size, size_t alignment)
{
    for (auto element : fsa_allocators_)
    {
        if (size <= element->get_chunk_size())
            return element->allocate(size);
    }

    if (size < 1e+7)
        return coalesce_allocator_->allocate(size, alignment);

    if (size >= 1e+7)
        return os_allocator_->allocate(size);

    return nullptr;
}

void memory_allocator::free(void* ptr)
{
    assert(ptr != nullptr && "Trying to free nullptr");

    for (auto element : fsa_allocators_)
    {
        std::cout << "Start " << element->get_start_pointer() << " End " <<
            reinterpret_cast<void*>(static_cast<char*>(element->
                get_start_pointer()) + element->get_total_size()) << std::endl;
        if (ptr >= element->get_start_pointer() &&
            ptr <= reinterpret_cast<void*>(static_cast<char*>(element->get_start_pointer()) + element->
                get_total_size()))
        {
            element->free(ptr);
            return;
        }
    }

    std::cout << "Start " << coalesce_allocator_->get_start_pointer() << " End " << reinterpret_cast<void*>(static_cast<
        char*>(
        coalesce_allocator_->get_start_pointer()) + coalesce_allocator_->get_total_size()) << std::endl;
    if (ptr >= coalesce_allocator_->get_start_pointer() &&
        ptr <= reinterpret_cast<void*>(static_cast<char*>(coalesce_allocator_->get_start_pointer()) +
            coalesce_allocator_->get_total_size()))
    {
        coalesce_allocator_->free(ptr);
        return;
    }

    std::cout << "Start " << os_allocator_->get_start_pointer() << " End" << os_allocator_->get_end_pointer() <<
        std::endl;
    if (ptr >= os_allocator_->get_start_pointer() &&
        ptr <= os_allocator_->get_end_pointer())
    {
        os_allocator_->free(ptr);
        return;
    }
}

void memory_allocator::dump_stat() const
{
    for (auto element : fsa_allocators_)
    {
        std::cout << "\nFSR " << element->get_chunk_size()
            << "\n\tStart pointer " << element->get_start_pointer()
            << "\n\tEnd pointer " << static_cast<char*>(element->get_start_pointer()) + element->get_total_size();
    }
}

void memory_allocator::dump_blocks() const
{
}
