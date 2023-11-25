#include "./plot/plot.h"
#include "./videos/videos.h"
#include "./linalg/linalg.h"
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
		X[i] = 10 * (float)rand()/(float)RAND_MAX;
		Y[i] = sin(X[i]) + 2 * (float)rand()/(float)RAND_MAX - 1 + 10;
	}

	printf("%f, %f\n", X[0], Y[0]);
	printf("%f, %f\n", X[1], Y[1]);
	
	plot_line(X, Y, 2, "plot/Arial.ttf", 24, 2);

	return 0;
}
