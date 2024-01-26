# CS-project-2-Custom-Malloc
## Project Overview
This project is focused on implementing custom versions of the malloc(3), free(3), realloc(3), and calloc(3) functions in C. The goal is to create memory allocation functions that operate on the heap of a program, using the brk(2) and sbrk(2) system calls. This implementation aims to mimic the behavior of the standard libc functions, offering a unique opportunity to understand and control memory management at a low level.

## Team Members
Timothée Popesco - Maximilien Menesguen - Romain Theron 

## Key Features
- Custom Malloc (my_malloc): Allocates a block of memory of the specified size.
- Custom Free (my_free): Frees a previously allocated block of memory.
- Custom Realloc (my_realloc): Changes the size of a previously allocated memory block.
- Custom Calloc (my_calloc): Allocates a block of memory for an array of elements, initializing them to zero.

## Implementation Details
Our implementation includes the following key components:

- Header Structure: Represents a memory block with its size and a pointer to the next block
- Memory Allocation Algorithm: Efficiently allocates and deallocates memory blocks from the heap.
- Dynamic Memory Management: Handles varying memory requirements at runtime using sbrk(2).
