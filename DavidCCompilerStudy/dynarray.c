#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "dynarray.h"

#ifndef INITIAL_DYNARRAY_CAPACITY
#define INITIAL_DYNARRAY_CAPACITY 4
#endif

DynamicArray* dd_makeDynamicArray() {
	DynamicArray* dynarray = malloc(sizeof(DynamicArray));
	dynarray->capacity = INITIAL_DYNARRAY_CAPACITY; // start with an array of size INITIAL_DYNARRAY_CAPACITY (16)
	dynarray->size = 0; // starts with nothing in it
	dynarray->array = malloc(sizeof(void*) * dynarray->capacity); // allocate array of void *s
	return dynarray;
}

// pushes a new item to the end of the dynamic array
void dd_push(DynamicArray* dynarray, void* newItem) {
	if (dynarray->size == dynarray->capacity) {
		dd_resize(dynarray, dynarray->capacity * 2);
	}
	dynarray->array[dynarray->size] = newItem;
	dynarray->size++;
}

// pops an item from the end of the dynamic array
void* dd_pop(DynamicArray* dynarray) {
	void * temp = dynarray->array[dynarray->size - 1];
	dynarray->size--;
	return temp;
}

void* dd_free(DynamicArray* dynarray) {
	free(dynarray->array);
	free(dynarray);
}

void* dd_get(DynamicArray* dynarray, uint32_t index) {
	return dynarray->array[index];
}

void dd_set(DynamicArray* dynarray, uint32_t index, void * newItem) {
	if (index > (dynarray->capacity - 1) || index < 0){
		printf("ERROR (dynarray): Bad set");
		return;
	}
	dynarray->array[index] = newItem;
}

void dd_resize(DynamicArray* dynarray, uint32_t capacity) {
	void** temp = dynarray->array;
	if (capacity < dynarray->capacity) {
		printf("Error(dynarray): Can't resize array with this capacity: %u", capacity);
		return;
	}
	dynarray->array = malloc(sizeof(void*) * capacity);
	memcpy(dynarray->array, temp, dynarray->size * sizeof(void*) );
	dynarray->capacity = capacity;
}
