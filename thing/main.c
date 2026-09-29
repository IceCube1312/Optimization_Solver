#include "parser.h"

int main(int argc, char** argv){
	if(argc != 2){
		fprintf(stderr,"Usage : ./solve [INSERT_FILE_PATH_HERE]\n");
		exit(-1);
	}
	FILE *mps = fopen(argv[1],"r");
	if(mps==NULL){
		fprintf(stderr,"ERR: FILE NOT FOUND, TERMINATING EXECUTION SEQUENCE\n");
	}
	LP_MODEL* eqn = parse_into_CSR(mps);
	fprintf(stdout,"num_rows: %i\nnum_cols: %i\nnum_nzv: %i\n",eqn->matrix->num_rows,eqn->matrix->num_cols,eqn->matrix->num_nzv);
	verify_parsed_data(eqn);
	return 0;
}
