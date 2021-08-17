#pragma once
#include <stdint.h>

typedef struct DynamicArray {
	void** array;         // array of void *s
	uint32_t capacity;        // max 65,536
	uint32_t size; // max 65,536
} DynamicArray;

// returns a dynamic array "INITIAL_DYNARRAY_CAPACITY (4) big
DynamicArray* dd_makeDynamicArray();

// pushes a new item to the end of the dynamic array
void dd_push(DynamicArray* dynarray, void* newItem);

// pops an item from the end of the dynamic array
void* dd_pop(DynamicArray* dynarray);

// Free dynamic array data-struct and child array
void* dd_free(DynamicArray* dynarray);

// get void * indexed at "index"
void* dd_get(DynamicArray* dynarray, uint32_t index);

// set void * indexed at "index"
void dd_set(DynamicArray* dynarray, uint32_t index, void* newItem);

// resize the array to the specified capacity
void dd_resize(DynamicArray* dynarray, uint32_t capacity);
