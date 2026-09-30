#include "parser.h"

#include <pthread.h>
#include <unistd.h>

pthread_mutex_t math_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t math_cond = PTHREAD_COND_INITIALIZER;
int math_sequence_ready = 0;

void* math_worker_routine(void* arg) {
	int thread_id = *(int*)arg;

	pthread_mutex_lock(&math_lock);
	printf("[Thread %d] Initialized. Standby for CSR matrix math sequence.\n", thread_id);

	while (math_sequence_ready == 0) {
		pthread_cond_wait(&math_cond, &math_lock);
	}

	printf("[Thread %d] Math sequence signal received. Executing sparse matrix operations.\n", thread_id);
	pthread_mutex_unlock(&math_lock);

	// Future CSR mathematical operations execute here

	return NULL;
}

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

	int NUM_MATH_THREADS = 4;
	pthread_t workers[NUM_MATH_THREADS];
	int thread_ids[NUM_MATH_THREADS];

	printf("\n--- INITIALIZING MATH THREAD POOL ---\n");
	for (int i = 0; i < NUM_MATH_THREADS; i++) {
		thread_ids[i] = i;
		pthread_create(&workers[i], NULL, math_worker_routine, &thread_ids[i]);
	}

	sleep(2); 

	printf("\n[Main] CSR Matrix Phase 2 complete. Triggering thread broadcast...\n");

	pthread_mutex_lock(&math_lock);
	math_sequence_ready = 1;
	pthread_cond_broadcast(&math_cond);
	pthread_mutex_unlock(&math_lock);

	for (int i = 0; i < NUM_MATH_THREADS; i++) {
		pthread_join(workers[i], NULL);
	}
	printf("[Main] Thread execution terminated. Exiting system.\n");
	return 0;
}
