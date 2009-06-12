#include <stdlib.h>
#include <stdio.h>
#include "fann.h"

#define NUM_FEATURES 5
#define NUM_NEURONS_HIDDEN_LAYER_1 11
#define NUM_NEURONS_HIDDEN_LAYER_2 13

int main(int argc, char *argv[]) {
	const unsigned int num_inputs = NUM_FEATURES * 2;
	const unsigned int num_outputs = 1;
	const unsigned int num_layers = 2 + 1 + 1; // 2 Hidden + input + output
	
	struct fann *ann = fann_create_standard(num_layers, num_inputs, NUM_NEURONS_HIDDEN_LAYER_1, NUM_NEURONS_HIDDEN_LAYER_2, num_outputs);

	//fann_set_activation_function_hidden(ann, FANN_SIGMOID);
	fann_set_activation_function_output(ann, FANN_SIGMOID);
	fann_set_training_algorithm(ann, FANN_TRAIN_INCREMENTAL);
	fann_set_learning_rate(ann, 2);

	fann_train_on_file(ann, "training.data", 10000, 100, 0.00001);

	fann_save(ann, "trained.net");

	fann_destroy(ann);

	return 0;
}
