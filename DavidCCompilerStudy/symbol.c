#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "symbol.h"
#include "dynarray.h"
#include "dcommon.h"

void initSymbolTable(symbol_table** table) {
	*table = malloc(sizeof(symbol_table));
	(*table)->d_array = dd_makeDynamicArray();
}

BOOL addSymbol(symbol_table* table, symbol_type type, const char* name, int nameLength) {
	// Check if the symbol is already in the table
	d_symbol* temp = NULL;
	for (int i = 0; i < table->d_array->size; i++) {
		temp = dd_get(table->d_array, i);
		if (memcmp(temp->symbol_name, name, nameLength) == 0) {
			// The symbol is already in the table
			return FALSE;
		}
	}

	// The symbol isn't in the table, add it
	d_symbol* newSymbol = malloc(sizeof(d_symbol));
	newSymbol->symbol_name = calloc(nameLength + 1,sizeof(char));
	if (name != NULL) {
		memcpy(newSymbol->symbol_name, name, nameLength);
	}
	else {
		fprintf(stderr, "Error in symbolTable add, name is NULL");
	}
	newSymbol->type = type;

	dd_push(table->d_array, newSymbol);
	return TRUE;
}

d_symbol* getSymbol(symbol_table* table, const char* name, int nameLength) {
	d_symbol* temp = NULL;
	for (int i = 0; i < table->d_array->size; i++) {
		temp = dd_get(table->d_array, i);
		if (memcmp(name, temp->symbol_name, nameLength) == 0) {
			return temp;
		}
	}
	return NULL;
}

void printSymbol(d_symbol* symbol) {
	printf(symbol->symbol_name);
	switch (symbol->type) {
	case(dtype_INT):
		printf(": Int\n");
		break;
	case(dtype_BOOL):
		printf(": Bool\n");
		break;
	default:
		printf(": INVALID TYPE\n");
	}
}

void printSymbolTable(symbol_table* table) {
	printf("Symbol Table: \n");
	d_symbol* temp = NULL;
	for (int i = 0; i < table->d_array->size; i++) {
		temp = dd_get(table->d_array, i);
		printSymbol(temp);
	}
}
