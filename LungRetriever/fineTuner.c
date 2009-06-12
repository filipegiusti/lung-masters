#include <stdlib.h>
#include <stdio.h>
#include "fann.h"

#define NUM_FEATURES 5

int main(int argc, char *argv[]) {
	fann_type *output;
    fann_type input[NUM_FEATURES * 2];
	int i;
	
	struct fann *ann = fann_create_from_file(argv[1]);
	
	if(argc < NUM_FEATURES * 2 + 3) {
		printf("Faltam argumentos\n");
		exit(1);
	}

	for(i = 0; i < NUM_FEATURES * 2; i++) {
		input[i] = atof(argv[i+2]);
	}
	*output = atof(argv[i+2]);
	
	fann_train(ann, input, output);

	if (fann_save(ann, argv[1])) {
		printf("Erro: Socorro!!");
	}
	
	fann_destroy(ann);
	return 0;
}
