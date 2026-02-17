#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"



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

void symbolTablePush(SymbolTable* vec, const char* name, int address){
	if(vec->size >= vec->capacity){
		vec->capacity *= 2;
		vec->data = realloc(vec->data, sizeof(Symbol) * vec->capacity);
	}
	vec->data[vec->size].name = strdup(name);
	vec->data[vec->size].address = address;
	
	vec->size++;
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

