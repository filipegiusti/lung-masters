#include <stdlib.h>
#include <stdio.h>
#include "fann.h"

#define NUM_FEATURES 5

int main(int argc, char *argv[]) {
	fann_type *output;
    fann_type input[NUM_FEATURES * 2];
	int i;
	
	if(argc < NUM_FEATURES * 2 + 2) {
		printf("Faltam argumentos\n");
		exit(1);
	}
	
	struct fann *ann = fann_create_from_file(argv[1]);

	for(i = 0; i < NUM_FEATURES * 2; i++) {
		input[i] = atof(argv[i+2]);
		//printf("%e\n", input[i]);
	}
	
	output = fann_run(ann, input);

	printf("%e",output[0]);
	
	fann_destroy(ann);
	return 0;
}