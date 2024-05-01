#include "umem.h"
#include <stdio.h>
#include <stdint.h>
#include <assert.h>


//call the memdump to show current mem state. should show size and status of each block, shows us how things are going
void print_mem_state()
{
    printf("\n Mem state:\n");
    umemdump();
    printf("\n");
}


//start w/ basics of functionality. allocate 100 bytes, free it, and show the mem state. validate that the memory is aligned to 8 bytes. 
void test_basic_operations()
{
    printf("\n");
    void *a = umalloc(100);
    printf("Allocated 100 bytes at: %p\n", a);
    assert(((uintptr_t)a % 8) == 0 && "Memory isn't aligned 8b");
    ufree(a);
    print_mem_state();
}

//test edge cases. allocate 0 bytes, allocate negative bytes, allocate 50 bytes, free it twice, allocate 20 bytes, allocate 30 bytes, free 20 bytes, free 30 bytes, free 10 bytes, and show the mem state. zero should be null, negative should be null, and repeat frees should be null.
//FIXED CRASHES THAT WAS HAPPENING!!!
void test_edge_cases()
{
    printf("Edge:\n");
    void *a = umalloc(0);
    assert(a == NULL && "Size 0 null");
    void *b = umalloc((size_t)-1);
    assert(b == NULL && "Negative is null");

    void *c = umalloc(50);
    ufree(c);
    ufree(c);
    printf("repeat  %p.\n", c);

    void *d = umalloc(20);
    void *e = umalloc(30);
    ufree(d);
    void *f = umalloc(10);
    ufree(e);
    ufree(f);
    print_mem_state();
}

//test allocation strategies. allocate 5000, 1000, 3000 bytes, free 1000 bytes, allocate 800 bytes, and show the mem state. repeat for best, worst, first, and next fit. intializes each one and then does the alloc/dealloc to verify how its going
void test_allocation_strategies()
{
    printf("\n");
    umeminit(1024 * 1024, BEST_FIT);
    void *a = umalloc(5000);
    void *b = umalloc(1000);
    void *c = umalloc(3000);
    printf("Alloc 5000, 1000, 3000 bytes at: %p, %p, %p\n", a, b, c);
    ufree(b);
    void *d = umalloc(800);
    printf("Alloc 800 bytes at: %p\n", d);
    print_mem_state();

    umeminit(1024 * 1024, WORST_FIT);
    a = umalloc(5000);
    b = umalloc(1000);
    c = umalloc(3000);
    ufree(b);
    d = umalloc(800);
    print_mem_state();

    umeminit(1024 * 1024, FIRST_FIT);
    a = umalloc(5000);
    b = umalloc(1000);
    c = umalloc(3000);
    ufree(b);
    d = umalloc(800);
    print_mem_state();

    umeminit(1024 * 1024, NEXT_FIT);
    a = umalloc(5000);
    b = umalloc(1000);
    c = umalloc(3000);
    ufree(b);
    d = umalloc(800);
    print_mem_state();
}

int main()
{
    umeminit(1024 * 1024, FIRST_FIT);
    test_basic_operations();
    test_edge_cases();
    test_allocation_strategies();
    return 0;
}
