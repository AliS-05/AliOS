#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"


static void grow(SymbolTable* table) {
	table->capacity *= 2;
	table->data = realloc(table->data, table->capacity * sizeof(Symbol));
	if (!table->data) {
		printf("Failed to grow symbol table\n");
		exit(1);
	}
}

void symbolTableInit(SymbolTable* table) {
	table->size = 0;
	table->capacity = 16;
	table->data = malloc(table->capacity * sizeof(Symbol));
	if (!table->data) {
		printf("Failed to initialize symbol table\n");
		exit(1);
	}
}

void symbolTableFree(SymbolTable* table) {
	for (int i = 0; i < table->size; i++) {
		free(table->data[i].name);
	}
	free(table->data);
}

void symbolTableInsert(SymbolTable* table, const char* name, int address) {

// Check duplicate label
	for (int i = 0; i < table->size; i++) {
		if (strcmp(table->data[i].name, name) == 0) {
		    printf("Duplicate label: %s\n", name);
		    exit(1);
		}
	}

	if (table->size >= table->capacity) {
		grow(table);
	}

	table->data[table->size].name = strdup(name);
	table->data[table->size].address = address;
	table->size++;
}

int symbolTableLookup(SymbolTable* table, const char* name) {

	for (int i = 0; i < table->size; i++) {
		if (strcmp(table->data[i].name, name) == 0) {
		    return table->data[i].address;
		}
	}

	printf("Undefined label: %s\n", name);
	exit(1);
}


void printSymbolTable(SymbolTable* table){
	for(int i = 0; i < table->size; i++){
		printf("Symbol %s at address %d\n", table->data[i].name, table->data[i].address);
	}
}

