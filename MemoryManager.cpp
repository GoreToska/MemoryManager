#include <iostream>

#include "Classes/memory_allocator.h"

int main(int argc, char* argv[])
{
    memory_allocator allocator;
    allocator.init();
    int* pi = static_cast<int*>(allocator.alloc(sizeof(int)));
    int* pi2 = static_cast<int*>(allocator.alloc(sizeof(int)));
    double* pd = static_cast<double*>(allocator.alloc(sizeof(double)));
    int* pi3 = static_cast<int*>(allocator.alloc(sizeof(int)));

    double* p32 = static_cast<double*>(allocator.alloc(32));
    double* p32_2 = static_cast<double*>(allocator.alloc(32));

    int* big_array = static_cast<int*>(allocator.alloc(1e+6, 64));
    int* big_array2 = static_cast<int*>(allocator.alloc(1e+6, 64));

    int* very_big_array = static_cast<int*>(allocator.alloc(1e+9, 8));
    
    allocator.free(pi);
    allocator.free(pi2);
    allocator.free(pd);
    allocator.free(pi3);
    allocator.free(big_array);
    allocator.free(big_array2);
    allocator.free(very_big_array);
    allocator.free(p32);
    allocator.free(p32_2);

    pi = static_cast<int*>(allocator.alloc(sizeof(int)));
    pi2 = static_cast<int*>(allocator.alloc(sizeof(int)));
    pd = static_cast<double*>(allocator.alloc(sizeof(double)));
    pi3 = static_cast<int*>(allocator.alloc(sizeof(int)));

    allocator.free(pi);
    allocator.free(pi2);
    allocator.free(pd);
    allocator.free(pi3);
    
    std::cin.ignore(1);
    allocator.destroy();
}
