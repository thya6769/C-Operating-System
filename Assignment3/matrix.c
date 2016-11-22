#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <string.h>
#include <math.h>
#include "matrix.h"


static int g_seed = 0;

static ssize_t g_width = 0;
static ssize_t g_height = 0;
static ssize_t g_elements = 0;

static ssize_t g_nthreads = 1;
static pthread_mutex_t mylock = PTHREAD_MUTEX_INITIALIZER;

static float maximum;
static float minimum;

#define IDX(x, y) ((y) * g_width + (x))
#define NELEMS	(g_elements / g_nthreads)
#define CHUNK (g_width / g_nthreads)
#define COMPARE(a,b) (a == b)
#define MIN_MAX(a,b) ((a < b) ? true : false)
#define ADDITION(a,b) (a+b)
#define MULTIPLY(a,b) (a*b)
#define END(a) (a == g_nthreads - 1 ? g_width : (a + 1) * CHUNK)


typedef struct {
	size_t id;
	size_t count;
	float* result;
	float value;
	const float* a;
	const float* b;
} worker_args;

typedef struct {
	const float* mat;
	ssize_t id;
	ssize_t count;
	float value;
} freq_args;

////////////////////////////////
///     UTILITY FUNCTIONS    ///
////////////////////////////////

/**
 * Returns pseudorandom number determined by the seed.
 */
int fast_rand(void) {

	g_seed = (214013 * g_seed + 2531011);
	return (g_seed >> 16) & 0x7FFF;
}

/**
 * Sets the seed used when generating pseudorandom numbers.
 */
void set_seed(int seed) {

	g_seed = seed;
}

/**
 * Sets the number of threads available.
 */
void set_nthreads(ssize_t count) {
	g_nthreads = count;
}

/**
 * Sets the dimensions of the matrix.
 */
void set_dimensions(ssize_t order) {

	g_width = order;
	g_height = order;

	g_elements = g_width * g_height;
}

/**
 * Displays given matrix.
 */
void display(const float* matrix) {

	ssize_t width = g_width;
	for (ssize_t y = 0; y < width; y++) {
		for (ssize_t x = 0; x < width; x++) {
			if (x > 0){
			 	printf(" ");
			}
			printf("%.2f", matrix[IDX(x,y)]);
		}

		printf("\n");
	}
}

/**
 * Displays given matrix row.
 */
void display_row(const float* matrix, ssize_t row) {

	for (ssize_t x = 0; x < g_width; x++) {
		if (x > 0){
		 	printf(" ");
		}
		printf("%.2f", matrix[row * g_width + x]);
	}

	printf("\n");
}

/**
 * Displays given matrix column.
 */
void display_column(const float* matrix, ssize_t column) {


	for (ssize_t i = 0; i < g_height; i++) {
		printf("%.2f\n", matrix[i * g_width + column]);
	}
}

/**
 * Displays the value stored at the given element index.
 */
void display_element(const float* matrix, ssize_t row, ssize_t column) {

	printf("%.2f\n", matrix[row * g_width + column]);
}

////////////////////////////////
///   MATRIX INITALISATIONS  ///
////////////////////////////////

/**
 * Returns new matrix with all elements set to zero.
 */
float* new_matrix(void) {

	return calloc(g_elements, sizeof(float));
}

/**
 * Returns new identity matrix.
 */
float* identity_matrix(void) {

	float* result = new_matrix();

	for (ssize_t i = 0; i < g_elements; i++) {
		// if its odd
		if(i % (g_width + 1) == 0) {
			result[i] = 1.00;
		} else {
			result[i] = 0.00;
		}

	} 

	return result;
}

/**
 * Returns new matrix with elements generated at random using given seed.
 */
float* random_matrix(int seed) {

	float* result = new_matrix();

	set_seed(seed);

	for (ssize_t i = 0; i < g_elements; i++) {
		result[i] = fast_rand();
	}

	return result;
}

/**
 * Returns new matrix with all elements set to given value.
 */
float* uniform_matrix(float value) {

	float* result = new_matrix();

	for (ssize_t i = 0; i < g_elements; i++) {
		result[i] = value;
	}

	return result;
}

/**
 * Returns new matrix with elements in sequence from given start and step
 */
float* sequence_matrix(float start, float step) {

	float* result = new_matrix();

	for (ssize_t i = 0; i < g_elements; i++) {
		result[i] = start + (i * step);
	}

	return result;
}

////////////////////////////////
///     MATRIX OPERATIONS    ///
////////////////////////////////
void* work_clone(void* args){
	worker_args* wargs = (worker_args*) args;

	size_t id = wargs->id;
	const float* matrix = wargs->a;
	const size_t start = id * NELEMS;
	const size_t end = id == g_nthreads - 1 ? g_elements : (id + 1) * NELEMS;
	
	for(size_t i = start; i < end; i++){
		wargs->result[i] = matrix[i];
	}
	
	return NULL;
}
/**
 * Returns new matrix with elements cloned from given matrix.
 */
float* cloned(const float* matrix) {

	
	float* result = new_matrix();
	const size_t nothread = g_nthreads;

	pthread_t thread_ids[nothread];
	worker_args args[nothread];

	for(size_t i = nothread; i--;){
		args[i] = (worker_args) {
			.a = matrix,
			.id = i,
			.result = result,
		};
	}
	for(size_t i = nothread; i--;){
		pthread_create(thread_ids + i, NULL, work_clone, args + i);
	}
	for(size_t i = nothread; i--;){
		pthread_join(thread_ids[i], NULL);
	}
	
	return result;
}
int compare_float (const void * a, const void * b)
{
  float fa = *(const float*) a;
  float fb = *(const float*) b;
  return (fa > fb) - (fa < fb);
}


void* work_sort(void* args){
	worker_args* wargs = (worker_args*) args;
	int size = wargs->count;

	qsort(wargs->result, size, sizeof(float), compare_float);
	
	return NULL;
}

/**
 * Returns new matrix with elements in ascending order.
 */
float* sorted(const float* matrix) {

	float* result = new_matrix();
	float** lists = malloc(sizeof(lists) * g_nthreads);
	worker_args* args = malloc(sizeof(worker_args) * g_nthreads);

	size_t size = NELEMS;
	int indexes[g_nthreads];
	int j = 0;
	pthread_t thread_ids[g_nthreads];

	float temp;

	// loop through the thread
	for(size_t i = 0; i < g_nthreads; i++){
		if(i == g_nthreads - 1){
			size = NELEMS + g_elements % g_nthreads;
		}
		args[i] = (worker_args){
			.id = i,
			.count = size,
		};
		lists[i] = malloc(sizeof(float)*size);
		for(int k = 0; k < size; k++, j++){
			lists[i][k] = (float) matrix[j];
		}

		args[i].result = lists[i];
		indexes[i] = 0;
	}
	
	for(int i= g_nthreads; i--;){
		pthread_create(thread_ids + i, NULL, work_sort, args + i);
	}
	for(int i = g_nthreads; i--;){
		pthread_join(thread_ids[i], NULL);
	}
	//loop through the array
	for(int i = 0; i < g_elements; i++){
		for(int k = 0; k < g_nthreads; k++){
			if(indexes[k] < args[k].count){
				temp = lists[k][indexes[k]];
				j = k;
				break;
			}
		}
		for(int k = j + 1; k < g_nthreads; k++){
			if(indexes[k] < args[k].count && lists[k][indexes[k]] < temp){
				temp = lists[k][indexes[k]];
				j = k;
			}
		}
		indexes[j]++;
		result[i] = temp;
	}

	for(int i = g_nthreads; i--;){
		free(lists[i]);
	}

	free(lists);
	free(args);

	return result;
}

void* work_rotate(void* args){
	worker_args* wargs = (worker_args*) args;

	size_t id = wargs->id;
	const float* matrix = wargs->a;
	const size_t start = id * NELEMS;
	const size_t end = id == g_nthreads - 1 ? g_elements : (id + 1) * NELEMS;

	size_t col, row;
	
	for(size_t i = start; i < end; i++){
		col = i;

		if(i >= g_width){
			while(col >= g_width){
				col -= g_width;
			}
		}
		row = (i - col)/g_width;

		wargs->result[IDX(col, row)] = matrix[IDX(row,(g_width-col-1))];
	}
	
	return NULL;
}
/**
 * Returns new matrix with elements rotated 90 degrees clockwise.
 */
float* rotated(const float* matrix) {

	float* result = new_matrix();
	const size_t nothread = g_nthreads;

	pthread_t thread_ids[nothread];
	worker_args args[nothread];

	for(size_t i = nothread; i--;){
		args[i] = (worker_args) {
			.a = matrix,
			.id = i,
			.result = result,
		};
	}
	for(size_t i = nothread; i--;){
		pthread_create(thread_ids + i, NULL, work_rotate, args + i);
	}
	for(size_t i = nothread; i--;){
		pthread_join(thread_ids[i], NULL);
	}
	
	return result;
}
void* work_rev(void *args){

	worker_args* wargs = (worker_args*) args;

	size_t id = wargs->id;
	const float* matrix = wargs->a;

	const size_t start = id * NELEMS;
	const size_t end = id == g_nthreads - 1 ? g_elements : (id + 1) * NELEMS;

	size_t index = g_elements - start - 1;

	for (size_t i = start; i < end; i++) {
		wargs->result[i] = matrix[index];
		index--;	
	}

	return NULL;
}
/*
 * Returns new matrix with elements ordered in reverse.
 */
float* reversed(const float* matrix) {

	float* result = new_matrix();
	const size_t nothread = g_nthreads;

	pthread_t thread_ids[nothread];
	worker_args args[nothread];

	for(size_t i = nothread; i--;){
		args[i] = (worker_args) {
			.a = matrix,
			.id = i,
			.result = result,
		};
	}
	for(size_t i = nothread; i--;){
		pthread_create(thread_ids + i, NULL, work_rev, args + i);
	}
	for(size_t i = nothread; i--;){
		pthread_join(thread_ids[i], NULL);
	}
	
	return result;
}
void* work_trans(void *args){
	worker_args* wargs = (worker_args*) args;

	size_t id = wargs->id;
	const float* matrix = wargs->a;

	const size_t start = id * NELEMS;
	const size_t end = id == g_nthreads - 1 ? g_elements : (id + 1) * NELEMS;

	size_t col, row;
	for (size_t i = start; i < end; i++) {
		col = i;
		if(i >= g_width){
			while(col >= g_width){
				col -= g_width;
			}
		}
		row = (i - col) / g_width;

		wargs->result[IDX(row, col)] = matrix[IDX(col,row)];
	}

	return NULL;
}
/**
 * Returns new transposed matrix.
 */
float* transposed(const float* matrix) {

	float* result = new_matrix();

	const size_t nothread = g_nthreads;

	pthread_t thread_ids[nothread];
	worker_args args[nothread];

	for(size_t i = nothread; i--;){
		args[i] = (worker_args) {
			.a = matrix,
			.id = i,
			.result = result,
		};
	}
	for(size_t i = nothread; i--;){
		pthread_create(thread_ids + i, NULL, work_trans, args + i);
	}
	for(size_t i = nothread; i--;){
		pthread_join(thread_ids[i], NULL);
	}
	return result;
}
void* worker_scalar_add(void* args) {

	worker_args* wargs = (worker_args*) args;

	size_t id = wargs->id;
	const float* matrix = wargs->a;

	const size_t start = id * NELEMS;
	const size_t end = id == g_nthreads - 1 ? g_elements : (id + 1) * NELEMS;

	float scalar = wargs->value;
	for (size_t i = start; i < end; i++) {
			wargs->result[i] = ADDITION(matrix[i], scalar);
	}
	return NULL;
}
void* worker_scalar_mul(void* args) {

	worker_args* wargs = (worker_args*) args;

	size_t id = wargs->id;
	const float* matrix = wargs->a;


	const size_t start = id * NELEMS;
	const size_t end = id == g_nthreads - 1 ? g_elements : (id + 1) * NELEMS;

	float scalar = wargs->value;
	for (size_t i = start; i < end; i++) {
			wargs->result[i] = MULTIPLY(matrix[i], scalar);
	}
	return NULL;
}
void help_scalar(const float* matrix, float* result, float scalar, void* func){
	
	const size_t nothread = g_nthreads;

	worker_args args[nothread];
	pthread_t thread_ids[nothread];

	for (size_t i = nothread; i--;) {
		args[i] = (worker_args) {
			.a = matrix,
			.id = i,
			.result = result,
			.value = scalar,
		};
	}
	// Launch threads
	for (size_t i = nothread; i--;) {
		pthread_create(thread_ids + i, NULL, func, args + i);
	}
	// Wait for threads to finish
	for (size_t i = nothread; i--;) {
		pthread_join(thread_ids[i], NULL);
	}
}
/**
 * Returns new matrix with scalar added to each element.
 */
float* scalar_add(const float* matrix, float scalar) {
	float* result = new_matrix();
	help_scalar(matrix, result, scalar, worker_scalar_add);

	return result;
}

/**
 * Returns new matrix with scalar multiplied to each element.
 */
float* scalar_mul(const float* matrix, float scalar) {

	float* result = new_matrix();

	help_scalar(matrix, result, scalar, worker_scalar_mul);

	return result;
}
void* worker_multiply(void* args) {

	worker_args* wargs = (worker_args*) args;

	size_t id = wargs->id;
	const size_t start = MULTIPLY(id,CHUNK);
	const size_t end = END(id);

	//size_t index;
	const ssize_t width = g_width;

	const float* matrix_a = wargs->a;
	const float* matrix_b = wargs->b;

	for (size_t y = start; y < end; y++) {
		for (size_t x = 0; x < width; x++) {
			for (size_t k = 0; k < g_width; k++) {
				wargs->result[IDX(x,y)] += MULTIPLY(matrix_a[IDX(k,y)], matrix_b[IDX(k,x)]);
					//, MULTIPLY(matrix_a[IDX(j, i)], matrix_b[IDX(j,k)]));
			}
		}
	}
	return NULL;
}
void* worker_add(void* args) {

	worker_args* wargs = (worker_args*) args;

	size_t id = wargs->id;

	const size_t start = id * NELEMS;
	const size_t end = id == g_nthreads - 1 ? g_elements : (id + 1) * NELEMS;

	for (size_t i = start; i < end; i++) {
		wargs->result[i] = ADDITION(wargs->a[i], wargs->b[i]);
	}
	return NULL;
}
void help_create(const float* matrix_a
	,const float* matrix_b, float* result, void* func){
	
	const ssize_t nothread = g_nthreads;
	worker_args args[nothread];
	pthread_t thread_ids[nothread];

	float* transpose = transposed(matrix_b);

	for (size_t i = nothread; i--;) {
		args[i] = (worker_args) {
			.a = matrix_a,
			.b = transpose,
			.id = i,
			.result = result,
		};
	}

	// Launch threads
	for (size_t i = nothread; i--;) {
		pthread_create(thread_ids + i, NULL, func, args + i);
	}

	// Wait for threads to finish
	for (size_t i = nothread; i--;) {
		pthread_join(thread_ids[i], NULL);
	}
}
/**
 * Returns new matrix that is the result of
 * adding the two given matrices together.
 */
float* matrix_add(const float* matrix_a, const float* matrix_b) {

	float* result = new_matrix();

	help_create(matrix_a, matrix_b, result, worker_add);

	return result;
}


/**
 * Returns new matrix that is the result of
 * multiplying the two matrices together.
 */
float* matrix_mul(const float* matrix_a, const float* matrix_b) {

	float* result = new_matrix();

	help_create(matrix_a, matrix_b, result, worker_multiply);
	
	return result;
}

/**
 * Returns new matrix that is the result of
 * powering the given matrix to the exponent.
 */
float* matrix_pow(const float* matrix, int exponent) {

	float* result;

	ssize_t elements = g_elements;
	if(exponent == 0){
		result = identity_matrix();
	} else {
		result = new_matrix();
		for(int i = elements; i--;){
			result[i] = matrix[i];
		}

		for(int i = 1; i < exponent; i++){
			float* temp = new_matrix();
			help_create(result, matrix, temp, worker_multiply);
			for(int i = 0; i < g_elements; i++){
				result[i] = *(temp + i);
			}	
			free(temp);
		}
	}
	return result;
}

/**
 * Returns new matrix that is the result of
 * convolving given matrix with a 3x3 kernel matrix.
 */
 float* matrix_conv(const float* matrix, const float* kernel) {

 	float* result = new_matrix();

 	// loop through the matrix
 	// i = row, j = col
	for(int i = 0; i < g_width; i++){
		for(int j = 0; j < g_width; j++){
			// top left 
			if(IDX(j, i) == 0){
				result[IDX(j, i)] += matrix[IDX(j, i)] * kernel[0];
			// if top is empty
			} else if(IDX(j, i) < g_width){
				result[IDX(j, i)] += matrix[IDX(j - 1, i)] * kernel[0];
			// if left is empty
			} else if(IDX(j, i) % g_width == 0){
				result[IDX(j, i)] += matrix[IDX(j, i - 1)] * kernel[0];					
			} else {	 
				result[IDX(j, i)] += matrix[IDX(j-1, i - 1)] * kernel[0];
			}

			// top middle 
			// top is empty
			if(IDX(j, i) < g_width){
				result[IDX(j, i)] += matrix[IDX(j,i)] * kernel[1];
			} else {
				result[IDX(j, i)] += matrix[IDX(j,i - 1)] * kernel[1];
			}

			// top right 
			if((IDX(j,i) + 1) == g_width){
				result[IDX(j, i)] += matrix[IDX(j,i)] * kernel[2];
			// right side cases
			} else if((IDX(j, i) +  1) % g_width == 0){
				result[IDX(j, i)] += matrix[IDX(j, i - 1)] * kernel[2];
			// top cases
			} else if(IDX(j, i) < g_width){
				result[IDX(j, i)] += matrix[IDX(j + 1,i)] * kernel[2];					
			} else {
				result[IDX(j, i)] += matrix[IDX(j+1, i -1)] * kernel[2];
			}

			// mid left
			// left is empty
			if((IDX(j, i)) % g_width == 0){
				result[IDX(j, i)] += matrix[IDX(j,i)] * kernel[3];
			} else {
				result[IDX(j, i)] += matrix[IDX(j - 1,i)] * kernel[3];
			} 

			// mid
			result[IDX(j, i)] += matrix[IDX(j,i)] * kernel[4];

			//mid right
			// right side
			if((IDX(j, i) +  1) % g_width == 0){
				result[IDX(j, i)] += matrix[IDX(j,i)] * kernel[5];
			} else {
				result[IDX(j, i)] += matrix[IDX(j + 1,i)] * kernel[5];
			} 

			// bottom left
			if(IDX(j, i) == g_elements - g_width){
				result[IDX(j, i)] += matrix[IDX(j,i)] * kernel[6];
			// left side is empty
			} else if ((IDX(j, i)) % g_width == 0){
				result[IDX(j, i)] += matrix[IDX(j,i + 1)] * kernel[6];
			// bottom side is empty
			} else if ((IDX(j,i + 1) >= g_elements)){
				result[IDX(j, i)] += matrix[IDX(j - 1,i)] * kernel[6];
			} else {
				result[IDX(j, i)] += matrix[IDX(j - 1,i + 1)] * kernel[6];
			} 

			// bottom middle 
			// bottom is empty
			if((IDX(j,i + 1) >= g_elements)){
				result[IDX(j, i)] += matrix[IDX(j,i)] * kernel[7];
			} else {
				result[IDX(j, i)] += matrix[IDX(j,i + 1)] * kernel[7];
			}

			// bottom right side
			if((IDX(j,i) + 1) == g_elements){
				result[IDX(j,i)] += matrix[IDX(j,i)] * kernel[8];
			// right side cases
			} else if((IDX(j, i) +  1) % g_width == 0){
				result[IDX(j, i)] += matrix[IDX(j, i + 1)] * kernel[8];
			// bottom is empty
			} else if ((IDX(j,i + 1) >= g_elements)){
				result[IDX(j, i)] += matrix[IDX(j + 1, i)] * kernel[8];
			} else {				
				result[IDX(j, i)] += matrix[IDX(j + 1, i + 1)] * kernel[8];
			}


			
			//printf("%f\n", matrix[IDX(3, 2)]);
		}
	}
	return result;
}

////////////////////////////////
///       COMPUTATIONS       ///
////////////////////////////////
void* work_sum(void* args){
	worker_args* wargs = (worker_args*) args;

	size_t id = wargs->id;
	const float* matrix = wargs->a;

	const size_t start = id * NELEMS;
	const size_t end = id == g_nthreads - 1 ? g_elements : (id + 1) * NELEMS;

	float sum = 0;
	for (size_t i = start; i < end; i++) {
		sum = ADDITION(sum, matrix[i]);	
	}
	wargs->value = sum;
	return NULL;
}
/**
 * Returns the sum of all elements.
 */
float get_sum(const float* matrix) {

	const size_t nothread = g_nthreads;

	pthread_t thread_ids[nothread];
	worker_args args[nothread];

	for(size_t i = nothread; i--;){
		args[i] = (worker_args) {
			.a = matrix,
			.id = i,
			.value = 0,
		};
	}
	float sum = 0;
	for(size_t i = nothread; i--;){
		pthread_create(thread_ids + i, NULL, work_sum, args + i);
	}
	for(size_t i = nothread; i--;){
		pthread_join(thread_ids[i], NULL);
		sum = ADDITION(sum, args[i].value);
	}
	return sum;
}
/**
 * Returns the trace of the matrix.
 */
float get_trace(const float* matrix) {

	float trace = 0;
	for (ssize_t i = 0; i < g_elements; i += g_width + 1) {
		trace = ADDITION(trace, matrix[i]);
	}

	return trace;
}

static void* help_minimum(void *args){

	worker_args* wargs = (worker_args*) args;
	ssize_t id = wargs->id;


	const size_t start = MULTIPLY(id, NELEMS);
	const size_t end = MULTIPLY((id + 1),NELEMS);

	const float* matrix = wargs->a;
	
	for (size_t i = start; i < end; i++) {
		if(MIN_MAX(matrix[i], minimum)){
			pthread_mutex_lock(&mylock);
			minimum = matrix[i];
			pthread_mutex_unlock(&mylock);
			
		}	
	}
	return NULL;
}
/**
 * Returns the smallest value in the matrix.
 */
float get_minimum(const float* matrix) {

	ssize_t nothread = g_nthreads;

	pthread_t thread_ids[nothread];
	worker_args args[nothread];

	for(size_t i = nothread; i--;){
		args[i] = (worker_args) {
			.a = matrix,
			.id = i,
		};
	}
	minimum = matrix[0];
	for(size_t i = nothread; i--;){
		pthread_create(thread_ids + i, NULL, help_minimum, args + i);
	}
	for(size_t i = nothread; i--;){
		pthread_join(thread_ids[i], NULL);
	}
	return minimum;
}

static void* help_maximum(void *args){

	worker_args* wargs = (worker_args*) args;
	ssize_t id = wargs->id;

	const size_t start = MULTIPLY(id, NELEMS);
	const size_t end = MULTIPLY((id + 1),NELEMS);

	const float* matrix = wargs->a;
	
	for (size_t i = start; i < end; i++) {
		if(MIN_MAX(maximum, matrix[i])){
			pthread_mutex_lock(&mylock);
			maximum = matrix[i];
			pthread_mutex_unlock(&mylock);
		}
			
	}
	return NULL;
}
/**
 * Returns the largest value in the matrix.
 */
float get_maximum(const float* matrix) {

	ssize_t nothread = g_nthreads;

	pthread_t thread_ids[nothread];
	worker_args args[nothread];

	for(size_t i = nothread; i--;){
		args[i] = (worker_args) {
			.a = matrix,
			.id = i,
		};
	}
	maximum = matrix[0];
	for(size_t i = nothread; i--;){
		pthread_create(thread_ids + i, NULL, help_maximum, args + i);
	}
	for(size_t i = nothread; i--;){
		pthread_join(thread_ids[i], NULL);
	}
	return maximum;
}

float detmat(float *arr,int order){
	int sign = 1, i, j, k, count;
	float sum = 0;
	float* arr2;
	int newsize, newpos, pos;

	if(order == 1){
		return(arr[0]);
	}

	for(i = 0; i < order; i++, sign *= -1)
	{
		newsize = (order - 1) * (order - 1);
		arr2 = calloc(newsize, 4);
		for(j = 1; j < order; j++)
		{
			for(k=0,count=0;k<order;k++)
			{
				if(k==i)
					continue;

				pos = j * order + k;
				newpos = ( j - 1) * (order - 1) + count;
				arr2[newpos] = arr[pos];
				count++;
			}
		}
		sum=sum+arr[i] * sign * detmat(arr2,order-1);
		free(arr2);
	}
	return(sum); 
}
/**
 * Returns the determinant of the matrix.
 */
float get_determinant(const float* matrix) {

	float* matrix_b = new_matrix();
	for(int i = 0; i < g_elements; i++){
		*(matrix_b + i) = *(matrix + i);
	}
	float sum = detmat(matrix_b, g_width);
	free(matrix_b);
	return sum;
}
void* work_frequency(void *args){

	freq_args* wargs = (freq_args*) args;

	size_t id = wargs->id;
	float value = wargs->value;
	const float* matrix = wargs->mat;
	const size_t start = MULTIPLY(id, NELEMS);
	const size_t end = MULTIPLY((id + 1),NELEMS);

	size_t count = 0;
	for (size_t i = start; i < end; ++i) {
		if(COMPARE(matrix[i],value)) {
			++count;
		}		
	}
	wargs->count = count;
	return NULL;
}
/**
 * Returns the frequency of the given value in the matrix.
 */
ssize_t get_frequency(const float* matrix, float value) {

	const size_t nothread = g_nthreads;

	pthread_t thread_ids[nothread];
	freq_args args[nothread];

	for(size_t i = nothread; i--;){
		args[i] = (freq_args) {
			.mat = matrix,
			.id = i,
			.count = 0,
			.value = value,
		};
	}
	for(size_t i = nothread; i--;){
		pthread_create(thread_ids + i, NULL, work_frequency, args + i);
	}
	for(size_t i = nothread; i--;){
		pthread_join(thread_ids[i], NULL);
	}
	ssize_t counter = args[nothread - 1].count;
	for(size_t i = nothread - 1; i--;){
		counter = ADDITION(counter, args[i].count);
	}

	return counter;
}


