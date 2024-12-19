#pragma once
#include "allocator_base.h"
#include "../Containers/linked_list.h"

class coalesce_allocator : public allocator_base
{
public:
    coalesce_allocator(const size_t total_size);
    ~coalesce_allocator() override;
    void* allocate(const size_t size, const size_t alignment) override;
    void free(void* ptr) override;
    void init() override;
    void reset();
    void destroy() override;

private:
    struct coalesce_free_header
    {
        size_t block_size;
    };

    struct allocation_header
    {
        size_t block_size;
        char padding;
    };

    typedef linked_list<coalesce_free_header>::list_node node;

    void merge(node* previous_node, node* free_node);
    void find(const size_t size, const size_t alignment, size_t& padding, node*& previousNode, node*& foundNode);
    const size_t calculate_padding_with_header(const size_t baseAddress, const size_t alignment,
                                                   const size_t headerSize);

    linked_list<coalesce_free_header> free_list_;
};
