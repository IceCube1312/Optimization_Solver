#include "parser.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void init_CSR_1(CSR* csr){
    csr->num_rows=0;
    csr->num_cols=0;
    csr->num_nzv=0;
}

void init_CSR_2(CSR* csr){
    csr->values = malloc(sizeof(double) * csr->num_nzv);
    csr->row_index = malloc(sizeof(int) * csr->num_nzv);
    csr->col_index = malloc(sizeof(int) * csr->num_nzv);
}

LP_MODEL* parse_into_CSR(FILE* mps){
    if(mps==NULL){
        fprintf(stderr,"ERR: FILE NOT FOUND\n");
        exit(FILE_NOT_FOUND);
    }
    char buffer[MAX_LINE_MPS];
    fgets(buffer,sizeof(buffer),mps);
    printf("%s",buffer);
    int section=0; 

    LP_MODEL* the_equation = malloc(sizeof(LP_MODEL));
    if(the_equation == NULL){ exit(MEM_ERR); }
    the_equation->matrix = malloc(sizeof(CSR));
    if(the_equation->matrix == NULL){ exit(MEM_ERR); }

    char prev_col[MAX_NAME_SIZE] = "";
    char curr_col[MAX_NAME_SIZE] = "";
    char temp_row1[MAX_NAME_SIZE], temp_row2[MAX_NAME_SIZE];
    double temp_val1, temp_val2;
    int num_entries;

    init_CSR_1(the_equation->matrix);

    // Phase 1- Dimension extraction
    while(fgets(buffer,sizeof(buffer),mps)){
        if(strncmp(buffer,ROWS,4) == 0 ) { section = 1; continue; }
        else if(strncmp(buffer,COLUMNS,7) == 0) { section = 2; continue; }
        else if(strncmp(buffer,RHS,3) == 0) { section = 3; break; }
        else if(strncmp(buffer,BOUNDS,6) == 0) { section = 4; break; }

        switch(section){
            case 0:
                fprintf(stderr,"ERR: NO SECTIONS DETECTED\n");
                exit(-1);
                break;
            case 1:
                the_equation->matrix->num_rows++;
                break;
            case 2:
                num_entries = sscanf(buffer," %s %s %lf %s %lf", curr_col, temp_row1, &temp_val1, temp_row2, &temp_val2);
                if(num_entries >= 3){
                    the_equation->matrix->num_nzv++;
                    if(strcmp(curr_col, prev_col) != 0){
                        the_equation->matrix->num_cols++;
                        strcpy(prev_col, curr_col);
                    }
                }
                if(num_entries == 5){
                    the_equation->matrix->num_nzv++;
                }
                break;
        }
    }

    // --- Phase 2: Memory Allocation & Reset ---
    init_CSR_2(the_equation->matrix);
    the_equation->rhs = malloc(the_equation->matrix->num_rows * sizeof(double));

    char **row_map = malloc(the_equation->matrix->num_rows * sizeof(char*));
    for (int i = 0; i < the_equation->matrix->num_rows; i++) {
        row_map[i] = malloc(MAX_NAME_SIZE * sizeof(char));
        the_equation->rhs[i] = 0.0; 
    }

    rewind(mps);
    section = 0;
    int mapped_rows = 0;
    int nzv_count = 0;
    int current_col_idx = -1;
    strcpy(prev_col, "");
    strcpy(curr_col, "");

    // --- Phase 2: Data Extraction & Coordinate Mapping ---
    while(fgets(buffer, sizeof(buffer), mps)) {
        if(strncmp(buffer, ROWS, 4) == 0 ) { section = 1; continue; }
        else if(strncmp(buffer, COLUMNS, 7) == 0) { section = 2; continue; }
        else if(strncmp(buffer, RHS, 3) == 0) { section = 3; continue; }
        else if(strncmp(buffer, BOUNDS, 6) == 0) { break; }

        switch(section) {
            case 1:
                if (sscanf(buffer, " %*c %s", row_map[mapped_rows]) == 1) {
                    mapped_rows++;
                }
                break;

            case 2:
                num_entries = sscanf(buffer, " %s %s %lf %s %lf", curr_col, temp_row1, &temp_val1, temp_row2, &temp_val2);

                if(num_entries >= 3) {
                    if(strcmp(curr_col, prev_col) != 0) {
                        current_col_idx++;
                        strcpy(prev_col, curr_col);
                    }
                    int r_idx = -1;
                    for(int i = 0; i < mapped_rows; i++) { 
                        if(strcmp(row_map[i], temp_row1) == 0) { r_idx = i; break; }
                    }
                    if (r_idx != -1) {
                        the_equation->matrix->values[nzv_count] = temp_val1;
                        the_equation->matrix->row_index[nzv_count] = r_idx;
                        the_equation->matrix->col_index[nzv_count] = current_col_idx;
                        nzv_count++;
                    }
                }
                if(num_entries == 5) {
                    int r_idx = -1;
                    for(int i = 0; i < mapped_rows; i++) {
                        if(strcmp(row_map[i], temp_row2) == 0) { r_idx = i; break; }
                    }
                    if (r_idx != -1) {
                        the_equation->matrix->values[nzv_count] = temp_val2;
                        the_equation->matrix->row_index[nzv_count] = r_idx;
                        the_equation->matrix->col_index[nzv_count] = current_col_idx;
                        nzv_count++;
                    }
                }
                break;

            case 3:
                num_entries = sscanf(buffer, " %*s %s %lf %s %lf", temp_row1, &temp_val1, temp_row2, &temp_val2);
                if (num_entries >= 2) {
                    for(int i = 0; i < mapped_rows; i++) {
                        if(strcmp(row_map[i], temp_row1) == 0) {
                            the_equation->rhs[i] = temp_val1;
                            break;
                        }
                    }
                }
                if (num_entries == 4) {
                    for(int i = 0; i < mapped_rows; i++) {
                        if(strcmp(row_map[i], temp_row2) == 0) {
                            the_equation->rhs[i] = temp_val2;
                            break;
                        }
                    }
                }
                break;
        }
    }

    for (int i = 0; i < the_equation->matrix->num_rows; i++) {
        free(row_map[i]);
    }
    free(row_map);

    return the_equation;
}

void verify_parsed_data(LP_MODEL* eqn) {
    if (eqn == NULL || eqn->matrix == NULL) {
        fprintf(stderr, "ERR: NULL POINTER PASSED TO VERIFICATION\n");
        return;
    }

    int nzv = eqn->matrix->num_nzv;
    int rows = eqn->matrix->num_rows;

    printf("--- CSR MATRIX DATA (First & Last 10) ---\n");
    for (int i = 0; i < nzv; i++) {
        if (i < 10 || i >= nzv - 10) {
            if (i == 10) {
                printf("...\n");
            }
            printf("Index %d: Col %d, Row %d, Value %lf\n",
                   i, eqn->matrix->col_index[i], eqn->matrix->row_index[i], eqn->matrix->values[i]);
        }
    }

    printf("\n--- RHS ARRAY DATA (First & Last 10) ---\n");
    if (eqn->rhs != NULL) {
        for (int i = 0; i < rows; i++) {
            if (i < 10 || i >= rows - 10) {
                if (i == 10) {
                    printf("...\n");
                }
                printf("Row %d RHS Value: %lf\n", i, eqn->rhs[i]);
            }
        }
    }
}
