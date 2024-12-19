template <class T>
class linked_list
{
public:
    struct list_node
    {
        T data;
        list_node* next;
    };

    list_node* head;

    linked_list() = default;

    void insert(list_node* previous_node, list_node* new_node)
    {
        if (previous_node == nullptr)
        {
            // Is the first node
            if (head != nullptr)
            {
                // The list has more elements
                new_node->next = head;
            }
            else
            {
                new_node->next = nullptr;
            }

            head = new_node;
        }
        else
        {
            if (previous_node->next == nullptr)
            {
                // Is the last node
                previous_node->next = new_node;
                new_node->next = nullptr;
            }
            else
            {
                // Is a middle node
                new_node->next = previous_node->next;
                previous_node->next = new_node;
            }
        }
    }

    void remove(list_node* previous_node, list_node* delete_node)
    {
        if (previous_node == nullptr)
        {
            // Is the first node
            if (delete_node->next == nullptr)
            {
                // List only has one element
                head = nullptr;
            }
            else
            {
                // List has more elements
                head = delete_node->next;
            }
        }
        else
        {
            previous_node->next = delete_node->next;
        }
    }
};
