#include "linalg.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
struct Layer
{
	int num_neurons;
	float *weights;
	float *biases;

	float (*activation_function)(float, float);
};

float sigmoid_activation(float output, float bias);
struct Layer layer_init_randomized(int num_neurons, int num_inputs, float (*activation_function)(float, float));

float sigmoid_activation(float output, float bias)
{
	return 1/(1 + powf(M_E, -(output))) + bias; /* Sigmoid function */
}
float relu_activation(float output, float bias)
{
	return (output < 0 ? 0 : output) + bias;
}
