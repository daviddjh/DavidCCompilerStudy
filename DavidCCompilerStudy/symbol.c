#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "symbol.h"
#include "dynarray.h"
#include "dcommon.h"
#include "codeGen.h"

void initSymbolTable(symbol_table** table) {
	*table = malloc(sizeof(symbol_table));
	(*table)->d_array = dd_makeDynamicArray();
	(*table)->stack_offset = 0;
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
	// Set the symbol type
	newSymbol->type = type;

	// Set the offset within the stack
	table->stack_offset += 4;	// TODO: TEMP only used for integers 
	newSymbol->stack_offset = table->stack_offset;

	// Set the current_reg
	//newSymbol->current_reg = dcg_none;

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

dcg_Reg getSymbolReg(symbol_table* table, const char* name, int nameLength) {
	d_symbol* temp = NULL;
	for (int i = 0; i < table->d_array->size; i++) {
		temp = dd_get(table->d_array, i);
		if (memcmp(name, temp->symbol_name, nameLength) == 0) {
			return temp->current_reg;
		}
	}

	return dcg_none;
}

void setSymbolReg(symbol_table* table, const char* name, int nameLength, dcg_Reg reg) {
	d_symbol* temp = NULL;
	for (int i = 0; i < table->d_array->size; i++) {
		temp = dd_get(table->d_array, i);
		if (memcmp(name, temp->symbol_name, nameLength) == 0) {
			temp->current_reg = reg;
			return;
		}
	}

	printf("Symbol: Error, Couldn't set reg\n");
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
