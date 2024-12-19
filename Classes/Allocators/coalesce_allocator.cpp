#include "coalesce_allocator.h"

#include <cassert>
#include <iostream>
#include <windows.h>

coalesce_allocator::coalesce_allocator(const size_t total_size)
    : allocator_base(total_size)
{
}

coalesce_allocator::~coalesce_allocator()
{
    // TODO: check for destruction
    assert(!is_initialized_ && "Coalesce allocator must be destroyed before destructor");
}

void* coalesce_allocator::allocate(const size_t size, const size_t alignment)
{
    constexpr size_t allocation_header_size = sizeof(allocation_header);
    //constexpr size_t free_header_size = sizeof(coalesce_free_header);

    assert("Allocation size must be bigger" && size >= sizeof(node));
    assert("Alignment must be 8 at least" && alignment >= 8);

    // Search through the free list for a free block that has enough space to allocate required data
    size_t padding;
    node* affected_node;
    node* previous_node;
    find(size, alignment, padding, previous_node, affected_node);
    assert(affected_node != nullptr && "Not enough memory");

    const size_t alignment_padding = padding - allocation_header_size;
    const size_t required_size = size + padding;
    const size_t data_remains = affected_node->data.block_size - required_size;

    if (data_remains > 0)
    {
        // split the block into the data block and a free block of size that left 
        node* new_free_node = reinterpret_cast<node*>(reinterpret_cast<std::size_t>(affected_node) + required_size);
        new_free_node->data.block_size = data_remains;
        free_list_.insert(affected_node, new_free_node);
    }

    free_list_.remove(previous_node, affected_node);

    // Setup data block
    const size_t header_address = reinterpret_cast<size_t>(affected_node) + alignment_padding;
    const size_t data_address = header_address + allocation_header_size;
    reinterpret_cast<allocation_header*>(header_address)->block_size = required_size;
    reinterpret_cast<allocation_header*>(header_address)->padding = alignment_padding;
    used_ += required_size;

#ifdef _DEBUG
    std::cout << "Coalesce allocated" << "\n\tHeader " << reinterpret_cast<void*>(header_address)
        << "\n\tData " << reinterpret_cast<void*>(data_address)
        << "\n\tSize " << reinterpret_cast<allocation_header*>(header_address)->block_size
        << "\n\tAlignment padding " << alignment_padding
        << "\n\tPadding " << padding
        << "\n\tMemory used " << used_
        << "\n\tRemains " << data_remains << std::endl;
#endif

    return reinterpret_cast<void*>(data_address);
}

void coalesce_allocator::free(void* ptr)
{
    const size_t current_address = reinterpret_cast<size_t>(ptr);
    const size_t header_address = current_address - sizeof(allocation_header);
    const allocation_header* header{reinterpret_cast<allocation_header*>(header_address)};

    node* free_node = reinterpret_cast<node*>(header_address);
    free_node->data.block_size = header->block_size; //+ header->padding;
    free_node->next = nullptr;

    node* it = free_list_.head;
    node* it_prev = nullptr;
    
    while (it != nullptr)
    {
        if (ptr < it)
        {
            free_list_.insert(it_prev, free_node);
            break;
        }
        it_prev = it;
        it = it->next;
    }

    used_ -= free_node->data.block_size;
    merge(it_prev, free_node);

#ifdef _DEBUG
    std::cout << "Coalesce free\n\t" << "Pointer " << ptr
        << "\n\tHeader " << (void*)free_node
        << "\n\tSize " << free_node->data.block_size
        << "\n\tMemory used " << used_ << std::endl;
#endif
}

void coalesce_allocator::init()
{
    if (start_ptr_ != nullptr)
    {
        start_ptr_ = nullptr;
    }

    start_ptr_ = VirtualAlloc(nullptr, total_size_, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    reset();
    is_initialized_ = true;
}

void coalesce_allocator::reset()
{
    used_ = 0;
    node* first_node = static_cast<node*>(start_ptr_);
    first_node->data.block_size = total_size_;
    first_node->next = nullptr;
    free_list_.head = nullptr;
    free_list_.insert(nullptr, first_node);
}

void coalesce_allocator::destroy()
{
    assert(used_ == 0 && "Сoalesce allocator memory leak");
    reset();
    VirtualFree(start_ptr_, total_size_, MEM_RELEASE);
    is_initialized_ = false;
}

void coalesce_allocator::merge(node* previous_node, node* free_node)
{
    if (free_node->next != nullptr &&
        reinterpret_cast<size_t>(free_node) + free_node->data.block_size == reinterpret_cast<size_t>(free_node->next))
    {
        free_node->data.block_size += free_node->next->data.block_size;
        free_list_.remove(free_node, free_node->next);
#ifdef _DEBUG
        std::cout << "Coalesce merging\n\t" << static_cast<void*>(free_node) << " and "
            << static_cast<void*>(free_node->next)
            << "\n\tSize " << free_node->data.block_size << std::endl;
#endif
    }

    if (previous_node != nullptr &&
        reinterpret_cast<size_t>(previous_node) + previous_node->data.block_size == reinterpret_cast<size_t>(free_node))
    {
        previous_node->data.block_size += free_node->data.block_size;
        free_list_.remove(previous_node, free_node);
#ifdef _DEBUG
        std::cout << "Coalesce merging\n\t" << static_cast<void*>(previous_node) << " and "
            << static_cast<void*>(free_node)
            << "\n\tSize " << previous_node->data.block_size << std::endl;
#endif
    }
}

void coalesce_allocator::find(const size_t size, const size_t alignment, size_t& padding, node*& previousNode,
                              node*& foundNode)
{
    node* it = free_list_.head;
    node* itPrev = nullptr;

    while (it != nullptr)
    {
        padding = calculate_padding_with_header((size_t)it, alignment,
                                                sizeof(coalesce_allocator::allocation_header));
        const size_t requiredSpace = size + padding;

        if (it->data.block_size >= requiredSpace)
            break;

        itPrev = it;
        it = it->next;
    }

    previousNode = itPrev;
    foundNode = it;
}

const size_t coalesce_allocator::calculate_padding_with_header(const size_t baseAddress, const size_t alignment,
                                                               const size_t headerSize)
{
    size_t multiplier = (baseAddress / alignment) + 1;
    size_t alignedAddress = multiplier * alignment;
    size_t padding = alignedAddress - baseAddress;
    size_t neededSpace = headerSize;

    if (padding < neededSpace)
    {
        // Header does not fit - Calculate next aligned address that header fits
        neededSpace -= padding;

        // How many alignments I need to fit the header        
        if (neededSpace % alignment > 0)
        {
            padding += alignment * (1 + (neededSpace / alignment));
        }
        else
        {
            padding += alignment * (neededSpace / alignment);
        }
    }

    return padding;
}
