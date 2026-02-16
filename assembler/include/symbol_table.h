#pragma once

typedef struct Symbol{
	char* name;
	int address;
} Symbol;

typedef struct SymbolTable{
	struct Symbol* data;
	int size;
	int capacity;
} SymbolTable;


void symbolTableInit(SymbolTable* vec);
void symbolTablePush(SymbolTable* vec, const char* name, int address);
int symbolTableLookup(SymbolTable* vec, const char* name);
void symbolTableFree(SymbolTable* vec);
void printSymbolTable(SymbolTable* vec);
