#ifndef PARSER_H
#define PARSER_H

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define ROWS "ROWS"
#define RHS "RHS"
#define COLUMNS "COLUMNS"
#define BOUNDS "BOUNDS"
#define MAX_LINE_MPS 256
#define MAX_NAME_SIZE 64
#define FILE_NOT_FOUND -1
#define MEM_ERR -2

typedef struct compressed_row_matrix{
	int num_rows;
	int num_cols;
	int num_nzv; //non zero variables
	
	double* values;
	int* col_index;
	int* row_index;
}CSR;

typedef struct matx_equation{
	CSR* matrix;
	double* rhs;
	double* costs;
	char* inequality;
}LP_MODEL;

LP_MODEL* parse_into_CSR(FILE* mps);
void init_CSR_1(CSR* csr);
void init_CSR_2(CSR* csr);
void verify_parsed_data(LP_MODEL* eqn);

#endif
