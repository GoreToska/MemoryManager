#pragma once

template <class T>
class linked_stack
{
public:
    struct stack_node
    {
        T data;
        stack_node* next;
    };

    stack_node* head;
    linked_stack() = default;
    
    void push(stack_node* new_node)
    {
        new_node->next = head;
        head = new_node;
    }

    stack_node* pop()
    {
        if(head == nullptr) return nullptr;
        
        stack_node* top = head;
        head = head->next;
        return top;
    }
};
