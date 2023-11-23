#include "./plot/plot.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
int main(void)
{
	float X[256];
	float Y[256];

	int i;

	srand(time(NULL));
	
	for(i = 0; i < 256; i++)
	{
		X[i] = 1000000 * (float)rand()/(float)RAND_MAX;
		Y[i] = sin(X[i]) + 2 * (float)rand()/(float)RAND_MAX - 1 + 10;
	}

	plot(X, Y, 256, "plot/Arial.ttf", plot_scatter);
	return 0;
}