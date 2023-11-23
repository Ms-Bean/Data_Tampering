#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "linalg.h"

//mat_binary_componentwise_op_t(float *A, float *B, int rows, int cols, float (*operation)(float, float), int num_threads);
//float *matmul_t(float *A, int A_rows, int A_cols, float *B, int B_rows, int B_cols, int num_threads);
float compute_activation(float output, float bias)
{
	return 1/(1 + powf(M_E, -(output+bias))); /* Sigmoid function */
}
float addone(float a)
{
	return a+1;
}
int main(void)
{
	int NUM_THREADS = 2;

	float inputs[] = {1,2,3,2.5};
	float weights[] = {
		0.2,0.8,-0.5,1,
		0.5,-0.91,0.26,-0.5,
		-0.26,-0.27,0.17,0.87
	};
	float biases[] = {2,3,0.5};
	
	printf("Inputs \n");
	print_mat(inputs, 1, 4);
	
	printf("Weights \n");
	print_mat(weights, 3, 4);

	printf("Biases \n");
	print_mat(biases, 1, 3);
	
	float *weights_transpose = transpose_t(weights, 3, 4, NUM_THREADS);
	float *outputs = matmul_t(inputs, 1, 4, weights_transpose, 4, 3, NUM_THREADS);
	free(weights_transpose);
	float *activations = mat_binary_componentwise_op_t(outputs, biases, 1, 3, compute_activation, NUM_THREADS);
	free(outputs);

	printf("Activations:\n");
	print_mat(activations, 1, 3);

	free(activations);
	return 0;
}
