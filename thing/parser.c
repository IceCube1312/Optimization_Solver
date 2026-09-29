#include "parser.h"

void init_CSR_1(CSR* csr){
	csr->num_rows=0;
	csr->num_cols=0;
	csr->num_nzv=0;
}

void init_CSR_2(CSR* csr){
	csr->values= malloc(sizeof(double)*csr->num_nzv);
	csr->row_index = malloc(sizeof(int)*csr->num_rows);
	csr->col_index = malloc(sizeof(int)*csr->num_cols);
}

LP_MODEL* parse_into_CSR(FILE* mps){
	if(mps==NULL){
		fprintf(stderr,"ERR: FILE NOT FOUND\n");
		exit(FILE_NOT_FOUND);
	}
	char buffer[MAX_LINE_MPS];
	fgets(buffer,sizeof(buffer),mps);
	printf("%s",buffer);
	int section=0; //0-NAME, 1- ROWS, 2- COLOUMS 3- RHS , 4-BOUNDS

	LP_MODEL* the_equation = malloc(sizeof(LP_MODEL));
	if(the_equation == NULL){
		exit(MEM_ERR);
	}
	the_equation->matrix = malloc(sizeof(CSR));
	if(the_equation->matrix == NULL){
		exit(MEM_ERR);
	}

	char prev_col[MAX_NAME_SIZE] = "";
	char curr_col[MAX_NAME_SIZE] = "";
	int num_entries;
	char temp_array[MAX_NAME_SIZE];
	double temp_value;

	init_CSR_1(the_equation->matrix);

	//Phase 1 getting the dimensions of the matrix
	while(fgets(buffer,sizeof(buffer),mps)){
		if(strncmp(buffer,ROWS,4) == 0 ){
			section = 1;
			continue;
		}else if(strncmp(buffer,COLUMNS,7) == 0){
			section = 2;
			continue;
		}else if(strncmp(buffer,RHS,3) == 0){
			section = 3;
			break;
		}else if(strncmp(buffer,BOUNDS,6) == 0){
			section = 4;
			break;
		}
		switch(section){
			case 0:
				fprintf(stderr,"ERR: NO SECTIONS DETECTED\n");
				exit(-1);
				break;
			case 1:
				the_equation->matrix->num_rows++;
				break;
			case 2:
				//getting the number of no zero entries by seeing all mentions of rows
				//in the coloumns section. Each unique combination of rows and cols mentioned
				//is a new entry. Therefore incrementing once if there are 3 entries per line
				//and incrementing twice if there are 5.

				num_entries = sscanf(buffer," %s %s %lf %s %lf",curr_col,temp_array,&temp_value,temp_array,&temp_value);
				if(num_entries == 3){
					the_equation->matrix->num_nzv+=1;
				}else if(num_entries ==5){
					the_equation->matrix->num_nzv+=2;
				}

				if(strcmp(curr_col,prev_col)!=0){
					the_equation->matrix->num_cols++;
				}
				strcpy(prev_col,curr_col);
				break;
		}
	}

	// --- Phase 2: Memory Allocation & Reset ---
	init_CSR_2(the_equation->matrix);
	the_equation->rhs = malloc(the_equation->matrix->num_rows * sizeof(double));

	char **row_map = malloc(the_equation->matrix->num_rows * sizeof(char*));
	for (int i = 0; i < the_equation->matrix->num_rows; i++) {
		row_map[i] = malloc(MAX_NAME_SIZE* sizeof(char));
		the_equation->rhs[i] = 0.0; // Zero-initialize RHS array
	}

	rewind(mps);
	section = 0;
	int mapped_rows = 0;
	int nzv_count = 0;
	strcpy(prev_col, "");

	char temp_row1[MAX_NAME_SIZE], temp_row2[MAX_NAME_SIZE];
	double temp_val1, temp_val2;

	while(fgets(buffer, sizeof(buffer), mps)) {
		if(strncmp(buffer, ROWS, 4) == 0 ) { 
			section = 1; 
			continue; 
		}
		else if(strncmp(buffer, COLUMNS, 7) == 0) { 
			section = 2; 
			continue; 
		}
		else if(strncmp(buffer, RHS, 3) == 0) { 
			section = 3; 
			continue; 
		}
		else if(strncmp(buffer, BOUNDS, 6) == 0) { 
			break; 
		}

		switch(section) {
			case 1:
				if (sscanf(buffer, " %*c %s", row_map[mapped_rows]) == 1) {
					mapped_rows++;
				}
				break;

			case 2:
				num_entries = sscanf(buffer, " %s %s %lf %s %lf", curr_col, temp_row1, &temp_val1, temp_row2, &temp_val2);

				if(num_entries >= 3) {
					int r_idx = -1;
					for(int i = 0; i < mapped_rows; i++) { 
						if(strcmp(row_map[i], temp_row1) == 0) { r_idx = i; break; }
					}
					if (r_idx != -1) {
						the_equation->matrix->values[nzv_count] = temp_val1;
						the_equation->matrix->row_index[nzv_count] = r_idx;
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
						nzv_count++;
					}
				}
				break;

			case 3:
				if (sscanf(buffer, " %*s %s %lf", temp_row1, &temp_val1) == 2) {
					for(int i = 0; i < mapped_rows; i++) {
						if(strcmp(row_map[i], temp_row1) == 0) {
							the_equation->rhs[i] = temp_val1;
							break;
						}
					}
				}
				break;
		}
	}


	return the_equation;
}
