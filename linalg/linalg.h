#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>

float dot(float *a, float *b, int n);
float dot_t(float *a, float *b, int n, int num_threads);
void *_dot_t(void *arg);

void print_mat(float *A, int rows, int cols);

float *transpose(float *A, int rows, int cols);
float *transpose_t(float *A, int rows, int cols, int num_threads);
void *_transpose_t(void *arg);

float *matmul(float *A, int A_rows, int A_cols, float *B, int B_rows, int B_cols);
float *matmul_t(float *A, int A_rows, int A_cols, float *B, int B_rows, int B_cols, int num_threads);
void *_matmul_t(void *arg);

float *matadd(float *A, int A_rows, float *B, int n_entries);
float *matadd_t(float *A, float *B, int n_entries, int num_threads);
void *_matadd_t(void *arg);

float *matscadd(float *A, float scalar, int n_entries);
float *matscadd_t(float *A, float scalar, int n_entries, int num_threads);
void *_matscadd_t(void *arg);

float *matscmul(float *A, float scalar, int n_entries);
float *matscmul_t(float *A, float scalar, int n_entries, int num_threads);
void *_matscmul_t(void *arg);

struct Dot_Thread_Data
{
	pthread_mutex_t *mutex;
	int num_threads;

	int n;
	float *a;
	float *b;

	float *sum;
};
struct Transpose_Thread_Data
{
	int num_threads;
	float *A;
	int rows;
	int cols;

	int tid;

	float *out;
};
struct Matmul_Thread_Data
{
	float *A;
	float *B_transpose;
	float *product;

	int cols;
	int A_rows;
	int B_transpose_rows;

	int tid;
	int num_threads;
};
struct Matadd_Thread_Data
{
	float *A;
	float *B;
	
	int n_entries;
	
	float *output;

	int tid;
	int num_threads;
};
struct Scalar_Op_Thread_Data
{
	float *A;
	float scalar;

	int n_entries;

	float *output;

	int tid;
	int num_threads;
};
void *_dot_t(void *arg)
{
	/*Thread data*/
	struct Dot_Thread_Data *data;
	pthread_mutex_t *mutex;
	int n;
	int num_threads;
	float *a;
	float *b;
	float *sum;
	
	int i;
	float local_sum;

	data = (struct Dot_Thread_Data *)arg;
	mutex = data->mutex;
	n = data->n;
	num_threads = data->num_threads;
	a = data->a;
	b = data->b;
	sum = data->sum;

	local_sum = 0; /*Compute a fragment of the dot product*/
	for(i = 0; i < n; i += num_threads)
		local_sum += a[i] * b[i];

	/*Add it to the total sum*/
	pthread_mutex_lock(mutex);
	*sum += local_sum;
	pthread_mutex_unlock(mutex);
	
	pthread_exit(NULL);
}
float dot_t(float *a, float *b, int n, int num_threads)
{
	struct Dot_Thread_Data *thread_data;
	pthread_mutex_t *mutex;
	float *sum;
	float sum_temp;

	pthread_t *threads;

	int i;

	threads = (pthread_t *)malloc(sizeof(pthread_t) * num_threads);
	if(!threads)
	{
		fprintf(stderr, "dot: malloc error\n");
		exit(4);
	}
	sum = (float *)malloc(sizeof(float));
	if(!sum)
	{
		fprintf(stderr, "dot: malloc error\n");
		exit(4);
	}
	thread_data = (struct Dot_Thread_Data *)malloc(sizeof(struct Dot_Thread_Data) * num_threads);
	if(!thread_data)
	{
		fprintf(stderr, "dot: malloc errorn\n");
		exit(4);
	}

	mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
	if(!mutex)
	{
		fprintf(stderr, "dot: malloc error\n");
		exit(4);
	}
	if(pthread_mutex_init(mutex, NULL))
	{
		fprintf(stderr, "dot: failed to create mutex\n");
		exit(4);
	}
	*sum = 0;
	for(i = 0; i < num_threads; i++)
	{
		thread_data[i].mutex = mutex;
		thread_data[i].num_threads = num_threads;
		thread_data[i].a = a + i;
		thread_data[i].b = b + i;
		thread_data[i].n = n - i;
		thread_data[i].sum = sum;

		pthread_create(threads + i, NULL, _dot_t, (void *)(thread_data + i));
	}
	for(i = 0; i < num_threads; i++)
		pthread_join(threads[i], NULL);
	free(threads);
	free(mutex);
	free(thread_data);
	
	sum_temp = *sum;
	free(sum);
	return sum_temp;

}
float dot(float *a, float *b, int n)
{
	int i;
	float sum;
       
	sum = 0;
	for(i = 0; i < n; i++)
		sum += a[i] * b[i];
	return sum;
}
void print_mat(float *A, int rows, int cols)
{
	int i;
	int j;

	for(i = 0; i < rows; i++)
	{
		printf("| ");
		for(j = 0; j < cols; j++)
			printf("%7.3f ", A[i * cols + j]);
		printf("|\n");
	}
	printf("\n");
}
float *transpose(float *A, int rows, int cols)
{
	int i, j;
	float *out = (float *)malloc(sizeof(float) * rows * cols);
	if(!out)
	{
		fprintf(stderr, "transpose: malloc error\n");
		exit(4);
	}
	for(i = 0; i < rows; i++)
		for(j = 0; j < cols; j++)
			out[j * rows + i] = A[i * cols + j];
	
	return out;
}
void *_transpose_t(void *arg)
{
	struct Transpose_Thread_Data *data;
	int num_threads;
	float *A;
	int rows;
	int cols;
	float *out;
	int tid;

	int i, j;

	data = (struct Transpose_Thread_Data *)arg;
	num_threads = data->num_threads;
	A = data->A;
	rows = data->rows;
	cols = data->cols;
	out = data->out;
	num_threads = data->num_threads;
	tid = data->tid;
	
	for(i = tid; i < rows; i += num_threads)
		for(j = 0; j < cols; j++)
			out[j * rows + i] = A[i * cols + j];
	pthread_exit(NULL);
}
float *transpose_t(float *A, int rows, int cols, int num_threads)
{
	float *out;
	pthread_t *threads;
	struct Transpose_Thread_Data *thread_data;
	int i;

	out = (float *)malloc(sizeof(float) * rows * cols);
	if(!out)
	{
		fprintf(stderr, "transpose_t: malloc error\n");
		exit(4);
	}
	threads = (pthread_t *)malloc(sizeof(pthread_t) * num_threads);
	if(!threads)
	{
		fprintf(stderr, "transpose_t: malloc error\n");
		exit(4);
	}
	thread_data = (struct Transpose_Thread_Data *)malloc(sizeof(struct Transpose_Thread_Data) * num_threads);
	if(!thread_data)
	{
		fprintf(stderr, "transpose_t: malloc error\n");
		exit(4);
	}
	for(i = 0; i < num_threads; i++)
	{
		thread_data[i].A = A;
		thread_data[i].out = out; 
		thread_data[i].rows = rows;
		thread_data[i].cols = cols;
		thread_data[i].num_threads = num_threads;
		thread_data[i].tid = i;
		pthread_create(threads + i, NULL, _transpose_t, (void *)(thread_data + i));
	}
	for(i = 0; i < num_threads; i++)
		pthread_join(threads[i], NULL);

	free(threads);
	free(thread_data);
	return out;
}
float *matmul(float *A, int A_rows, int A_cols, float *B, int B_rows, int B_cols)
{
	float *B_trans;
	float *product;
	int i, j;

	product = (float *)malloc(sizeof(float) * A_rows * B_cols);
	if(!product)
	{
		fprintf(stderr, "matmul: malloc error\n");
		exit(4);
	}
       	B_trans = transpose(B, B_rows, B_cols); /*We take the transpose in order to reduce stride to 1, improves performance due to caching*/
	
	for(i = 0; i < A_rows; i++)
		for(j = 0; j < B_cols; j++)
			product[i * A_cols + j] = dot(A + (A_cols * i), B_trans + (B_rows * j), A_cols);
	free(B_trans);
	return product;
}
float *matmul_t(float *A, int A_rows, int A_cols, float *B, int B_rows, int B_cols, int num_threads)
{
	struct Matmul_Thread_Data *thread_data;
	pthread_t *threads;

	float *B_transpose;
	float *product;

	int i;

	threads = (pthread_t *)malloc(sizeof(pthread_t) * num_threads);
	if(!threads)
	{
		fprintf(stderr, "matmul_t: malloc error\n");
		exit(4);
	}
	thread_data = (struct Matmul_Thread_Data *)malloc(sizeof(struct Matmul_Thread_Data) * num_threads);
	if(!thread_data)
	{
		fprintf(stderr, "matmul_t: malloc error\n");
		exit(4);
	}
	product = (float *)malloc(sizeof(float) * A_rows * B_cols);
	if(!product)
	{
		fprintf(stderr, "matmul_t: malloc error\n");
		exit(4);
	}
	B_transpose = transpose(B, B_rows, B_cols);
	
	for(i = 0; i < num_threads; i++)
	{
		thread_data[i].A = A;
		thread_data[i].B_transpose = B_transpose;
		thread_data[i].product = product;
		
		thread_data[i].cols = A_cols;
		thread_data[i].A_rows = A_rows;
		thread_data[i].B_transpose_rows = B_cols;
		
		thread_data[i].tid = i;
		thread_data[i].num_threads = num_threads;
			
		pthread_create(threads + i, NULL, _matmul_t, (void *)(thread_data + i));
	}

	for(i = 0; i < num_threads; i++)
		pthread_join(threads[i], NULL);

	free(threads);
	free(thread_data);
	free(B_transpose);

	return product;
}
void *_matmul_t(void *arg)
{
	struct Matmul_Thread_Data *thread_data;
	
	float *A;
	float *B_transpose;
	float *product;

	int cols;
	int A_rows;
	int B_transpose_rows;

	int tid;
	int num_threads;

	int i, j;
	
	thread_data = (struct Matmul_Thread_Data *)arg;
	
	A = thread_data->A;
	B_transpose = thread_data->B_transpose;
	product = thread_data->product;

	cols = thread_data->cols;
	A_rows = thread_data->A_rows;
	B_transpose_rows = thread_data->B_transpose_rows;

	tid = thread_data->tid;
	num_threads = thread_data->num_threads;

	for(i = tid; i < A_rows * B_transpose_rows; i += num_threads)
	{
		int product_row;
		int product_col;

		float dot_product;

		product_row = i / B_transpose_rows;
		product_col = i % B_transpose_rows;
		
		dot_product = 0;
		for(j = 0; j < cols; j++)
			dot_product += A[product_row * cols + j] * B_transpose[product_col * cols + j];
		printf("%d\n", i);
		product[i] = dot_product;
	}

	pthread_exit(NULL);
}

float *matadd(float *A, int A_rows, float *B, int n_entries)
{
	float *out;
	int i;

       	out = (float *)malloc(sizeof(float) * n_entries);
	if(!out)
	{
		fprintf(stderr, "matadd: malloc error\n");
		exit(4);
	}
	for(i = 0; i < n_entries; i++)
		out[i] = A[i] + B[i];
	return out;
}
float *matadd_t(float *A, float *B, int n_entries, int num_threads)
{
	struct Matadd_Thread_Data *thread_data;
	pthread_t *threads;

	float *output;

	int i;

	threads = (pthread_t *)malloc(sizeof(pthread_t) * num_threads);
	if(!threads)
	{
		fprintf(stderr, "matadd_t: malloc error\n");
		exit(4);
	}
	thread_data = (struct Matadd_Thread_Data *)malloc(sizeof(struct Matadd_Thread_Data) * num_threads);
	if(!thread_data)
	{
		fprintf(stderr, "matadd_t: malloc error\n");
		exit(4);
	}
	output = (float *)malloc(sizeof(float) * n_entries);
	if(!output)
	{
		fprintf(stderr, "matadd_t: malloc error\n");
		exit(4);
	}

	for(i = 0; i < num_threads; i++)
	{
		thread_data[i].A = A;
		thread_data[i].B = B;
		thread_data[i].n_entries = n_entries;
		thread_data[i].output = output;
		
		thread_data[i].tid = i;
		thread_data[i].num_threads = num_threads;
			
		pthread_create(threads + i, NULL, _matadd_t, (void *)(thread_data + i));
	}

	for(i = 0; i < num_threads; i++)
		pthread_join(threads[i], NULL);

	free(threads);
	free(thread_data);

	return output;
}
void *_matadd_t(void *arg)
{
	struct Matadd_Thread_Data *thread_data;

	float *A;
	float *B;
	
	int n_entries;
	
	float *output;

	int tid;
	int num_threads;

	int i;

	thread_data = (struct Matadd_Thread_Data *)arg;

	A = thread_data->A;
	B = thread_data->B;
	
	n_entries = thread_data->n_entries;

	output = thread_data->output;

	tid = thread_data->tid;
	num_threads = thread_data->num_threads;

	for(i = tid; i < n_entries; i += num_threads)
		output[i] = A[i] + B[i];
	
	pthread_exit(NULL);
}

float *matscadd(float *A, float scalar, int n_entries)
{
	float *output;
       	int i;
       	
	output = (float *)malloc(sizeof(float) * n_entries);
	if(!output)
	{
		fprintf(stderr, "matscadd: malloc error\n");
		exit(4);
	}
	for(i = 0; i < n_entries; i++)
		output[i] = A[i] + scalar;
}
float *matscadd_t(float *A, float scalar, int n_entries, int num_threads)
{
	struct Scalar_Op_Thread_Data *thread_data;
	pthread_t *threads;
	float *output;

	int i;

	thread_data = (struct Scalar_Op_Thread_Data *)malloc(sizeof(struct Scalar_Op_Thread_Data) * n_entries);
	if(!thread_data)
	{
		fprintf(stderr, "matscadd_t: malloc error\n");
		exit(4);
	}
	threads = (pthread_t *)malloc(sizeof(pthread_t) * num_threads);
	if(!threads)
	{
		fprintf(stderr, "matscadd_t: malloc error\n");
		exit(4);
	}
	output = (float *)malloc(sizeof(float) * n_entries);
	if(!output)
	{
		fprintf(stderr, "matscadd_t: malloc error\n");
		exit(4);
	}
	
	for(i = 0; i < num_threads; i++)
	{
		thread_data[i].A = A;
		thread_data[i].scalar = scalar;

		thread_data[i].n_entries = n_entries;

		thread_data[i].output = output;

		thread_data[i].tid = i;
		thread_data[i].num_threads = num_threads;

		pthread_create(threads + i, NULL, _matscadd_t, (void *)(thread_data + i));
	}
	for(i = 0; i < num_threads; i++)
		pthread_join(threads[i], NULL);

	free(threads);
	free(thread_data);

	return output;
}
void *_matscadd_t(void *arg)
{
	struct Scalar_Op_Thread_Data *thread_data;

	float *A;
	float scalar;

	int n_entries;

	float *output;

	int tid;
	int num_threads;

	int i;

	thread_data = (struct Scalar_Op_Thread_Data *)arg;

	A = thread_data->A;
	scalar = thread_data->scalar;

	n_entries = thread_data->n_entries;
	
	output = thread_data->output;

	tid = thread_data->tid;
	num_threads = thread_data->num_threads;

	for(i = tid; i < n_entries; i += num_threads)
		output[i] = A[i] + scalar;

	pthread_exit(NULL);
}
float *matscmul(float *A, float scalar, int n_entries)
{
	float *output;
       	int i;
       	
	output = (float *)malloc(sizeof(float) * n_entries);
	if(!output)
	{
		fprintf(stderr, "matscadd: malloc error\n");
		exit(4);
	}
	for(i = 0; i < n_entries; i++)
		output[i] = A[i] + scalar;
}
float *matscmul_t(float *A, float scalar, int n_entries, int num_threads)
{
	struct Scalar_Op_Thread_Data *thread_data;
	pthread_t *threads;
	float *output;

	int i;

	thread_data = (struct Scalar_Op_Thread_Data *)malloc(sizeof(struct Scalar_Op_Thread_Data) * n_entries);
	if(!thread_data)
	{
		fprintf(stderr, "matscadd_t: malloc error\n");
		exit(4);
	}
	threads = (pthread_t *)malloc(sizeof(pthread_t) * num_threads);
	if(!threads)
	{
		fprintf(stderr, "matscadd_t: malloc error\n");
		exit(4);
	}
	output = (float *)malloc(sizeof(float) * n_entries);
	if(!output)
	{
		fprintf(stderr, "matscadd_t: malloc error\n");
		exit(4);
	}
	
	for(i = 0; i < num_threads; i++)
	{
		thread_data[i].A = A;
		thread_data[i].scalar = scalar;

		thread_data[i].n_entries = n_entries;

		thread_data[i].output = output;

		thread_data[i].tid = i;
		thread_data[i].num_threads = num_threads;

		pthread_create(threads + i, NULL, _matscmul_t, (void *)(thread_data + i));
	}
	for(i = 0; i < num_threads; i++)
		pthread_join(threads[i], NULL);

	free(threads);
	free(thread_data);

	return output;
}
void *_matscmul_t(void *arg)
{
	struct Scalar_Op_Thread_Data *thread_data;

	float *A;
	float scalar;

	int n_entries;

	float *output;

	int tid;
	int num_threads;

	int i;

	thread_data = (struct Scalar_Op_Thread_Data *)arg;

	A = thread_data->A;
	scalar = thread_data->scalar;

	n_entries = thread_data->n_entries;
	
	output = thread_data->output;

	tid = thread_data->tid;
	num_threads = thread_data->num_threads;

	for(i = tid; i < n_entries; i += num_threads)
		output[i] = A[i] * scalar;

	pthread_exit(NULL);
}
