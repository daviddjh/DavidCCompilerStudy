#pragma once
#include "dynarray.h"
#include "dcommon.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

typedef enum symbol_type{
	dtype_INV  = 0,
	dtype_INT  = 1,
	dtype_BOOL = 2
} symbol_type;

typedef struct symbol_table {
	DynamicArray* d_array;
	unsigned int stack_offset;
} symbol_table;

typedef struct d_symbol {
	symbol_type type;
	char* symbol_name;
	unsigned int stack_offset;
	enum dcg_Reg current_reg;
} d_symbol;

void initSymbolTable(symbol_table* table);
BOOL addSymbol(symbol_table* table, symbol_type type, const char* name, int nameLength);
d_symbol* getSymbol(symbol_table* table, const char* name, int nameLength);
void printSymbol(d_symbol* symbol);
void printSymbolTable(symbol_table* table);
enum dcg_Reg getSymbolReg(symbol_table* table, const char* name, int nameLength);
void setSymbolReg(symbol_table* table, const char* name, int nameLength, enum dcg_Reg reg);
