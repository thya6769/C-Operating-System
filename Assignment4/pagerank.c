#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <immintrin.h>
#include <stdbool.h>

#include "pagerank.h"

static size_t nothreads = 1;
static size_t nopages = 1;


#define NELEMS(a,b) ((double) a / b)
#define IDX(x, y) ((y) * nopages + (x))

typedef struct worker_args worker_args;
typedef struct out_index out_index;
// no spinlock in mac os X
// typedef int pthread_spinlock_t;

// int pthread_spin_init(pthread_spinlock_t *lock, int pshared) {
//     __asm__ __volatile__ ("" ::: "memory");
//     *lock = 0;
//     return 0;
// }

// int pthread_spin_destroy(pthread_spinlock_t *lock) {
//     return 0;
// }

// int pthread_spin_lock(pthread_spinlock_t *lock) {
//     while (1) {
//         int i;
//         for (i=0; i < 10000; i++) {
//             if (__sync_bool_compare_and_swap(lock, 0, 1)) {
//                 return 0;
//             }
//         }
//         sched_yield();
//     }
// }
// int pthread_spin_unlock(pthread_spinlock_t *lock) {
//     __asm__ __volatile__ ("" ::: "memory");
//     *lock = 0;
//     return 0;
// }
struct worker_args {
	node* node;
	double* result;
	size_t id;
	size_t start;
	size_t end;
	double dampener;
	char padding[16];
};
struct out_index{
	size_t outindexes[100];
	size_t length;
	char padding[56];
};
typedef struct {
	double initial;
	size_t id;
	double* matrix;
	char padding[40];
} worker_initial;

typedef struct {
	double* matrix_a;
	double* matrix_b;
	double* result;
	size_t id;
	char padding[32];
} work_mul;

typedef struct {
	double* matrix_a;
	double* matrix_b;
	size_t id;
	char padding[40];
} work_conv;

pthread_spinlock_t lock;
int pshared;
static 	out_index* outidx;
static double convergence;

/**
 * Returns new matrix with all elements set to zero.
 */
double* new_matrix(size_t size) {
	return calloc(size, sizeof(double));
}

int cmpfunc(const void* a, const void* b){
	return ((*(int*) a - *(int*)b ));
}

void* worker_calc_M(void* args){
	worker_args* wargs = (worker_args*) args;
	size_t id = wargs->id;
	size_t start = wargs->start;
	size_t end = id == nothreads - 1 ?  nopages : wargs->end; 
	
	double dampener = wargs->dampener;
	double constant = (double) (1.0 / nopages);
	double val = (double) (1.0 - dampener) / nopages;

	size_t number_out;
	size_t index;

	node* current = wargs->node;

	size_t j, k;

	for(size_t y = start; y < end; y++){
		number_out = current->page->noutlinks;
		if(number_out == 0){
			for(j = 0; j < nopages; j++){
				wargs->result[IDX(y,j)] = constant;
			}
			current = current->next;
			continue;
		}
		index = current->page->index;

		for(k = 0; k < nopages; k++){
			// its the same value as initial
			if((size_t*) bsearch(&k, outidx[index].outindexes
					, outidx[index].length, sizeof(size_t), cmpfunc) != NULL) {
				wargs->result[IDX(y,k)] = (double) (dampener / number_out) + val;
			} else {
				wargs->result[IDX(y,k)] = (double) val;
			} 
		}
		current = current->next;
	}
	return NULL;
}
double* calc_M(node* list, size_t npages, size_t nthreads, double dampener){
	register double* result = new_matrix(npages*npages);	
	
	outidx = calloc(npages, sizeof(out_index));

	node* current_page_in;
	node* current = list;
 	size_t j = 0;
 	size_t k;
 	// initialize the outidx for reference
	while(current != NULL){
		// loop through the inlinks save the indexes
		current_page_in = current->page->inlinks;
	 	while(current_page_in != NULL){
	 		k = current_page_in->page->index;
	 		outidx[k].outindexes[outidx[k].length] = j;
	 		++outidx[k].length;
	 		current_page_in = current_page_in->next;
	 	}
		current = current->next;
		j++;
	}

	pthread_t thread_ids[nthreads];
	worker_args args[nthreads];
	size_t i; 
	size_t start;
	size_t temp;
	size_t chunk = npages / nthreads;

	current = list;
	for(i = 0; i < nthreads; i++){
		temp = i * chunk;
		start = (size_t) (i+1) * chunk;

		args[i] = (worker_args){
			.node = current,
			.dampener = dampener,
			.result = result,
			.start = temp,
			.end = start,
			.id = i
		};
		for(size_t l = 0; l < start-temp; l++){
			current = current->next;
		}

	}
	for(i = nthreads; i--; ){
		pthread_create(thread_ids + i, NULL, worker_calc_M, args + i);
	}
	for(i = nthreads; i--; ){
		pthread_join(thread_ids[i], NULL);
	}	
	
	free(outidx);
	return result;
}

void* worker_mul(void* args) {
	work_mul* wargs = (work_mul*) args;
	size_t npages = nopages;

 	size_t chunk = npages / nothreads;
 	size_t id = wargs->id;
 	size_t start = id * chunk;
 	size_t end = id == nothreads - 1 ?  npages : (id + 1) * chunk;

	register double sum[4], *ptemp, *temp;
 	size_t j;

 	double* matrix_a = wargs->matrix_a;
 	double* matrix_b = wargs->matrix_b;
 	double* result = wargs->result;
 	for(size_t i = start; i < end; ++i){
		*sum = 0;
		*(double *)(sum+1) = 0;
		*(double *)(sum+2) = 0;
		*(double *)(sum+3) = 0;

		for(j = 0; j < npages; j+=4){
			ptemp = &matrix_a[IDX(j,i)];
			temp = &matrix_b[j];
			*sum += (*(double *)(ptemp)) * *temp;
			*(double *)(sum+1) += (*(double *)(ptemp+1)) * *((double*)temp+1);
			*(double *)(sum+2) += (*(double *)(ptemp+2)) * *((double*)temp+2);
			*(double *)(sum+3) += (*(double *)(ptemp+3)) * *((double*)temp+3);
		}
		result[i] = sum[0] + sum[1] + sum[2] + sum[3];	
		
	}
	return NULL;
}

void* worker_initial_p(void* args){
	worker_initial* wargs = (worker_initial*) args;

	size_t id = wargs->id;
	size_t chunk = nopages / nothreads;

	size_t start = (size_t) id * chunk;
	size_t end = id == nothreads - 1 ?  nopages : (size_t) (id + 1) * chunk; 

	double* matrix = wargs->matrix;
	double initial = wargs->initial;
	for(size_t i = start; i < end; i++){
		matrix[i] = initial;
	}
	return NULL;
}
double* initial_p(size_t npages, size_t nthreads){

	// register keyword is to hint that the variable will be heavily used
    // and that you recommend it be kept in a processor register if possible
	register double* old = new_matrix(npages); 
	pthread_t thread_ids[nthreads];
	worker_initial args[nthreads];

	double initial = (double) 1.0/npages; 

	for(size_t i = 0; i < nthreads; i++){
		args[i] = (worker_initial) {
			.initial = initial,
			.id = i,
			.matrix = old,
		};
	}
	for(int i = nthreads; i--; ){
		pthread_create(thread_ids + i, NULL, worker_initial_p, args + i);
	}
	for(int i = nthreads; i--; ){
		pthread_join(thread_ids[i], NULL);
	}
	return old;
}

 void* worker_conv(void* args){
 	work_conv* wargs = (work_conv*) args;
 	size_t id = wargs->id;

	size_t chunk = nopages / nothreads;

 	size_t start = (size_t) id * chunk;
 	size_t end = id == nothreads - 1 ?  nopages : (size_t) (id + 1) * chunk;

 	register double conv = 0;
 	double* matrix_a = wargs->matrix_a;
 	double* matrix_b = wargs->matrix_b;
 	for(size_t i = start; i < end; i++){
 		conv += (double) pow(matrix_a[i] - matrix_b[i], 2);
 	}
	
	pthread_spin_lock(&lock);
 	convergence += conv;
 	pthread_spin_unlock(&lock);

 	return NULL;
 }

 bool is_convergence(size_t npages,size_t nthreads, double* mat_a, double* mat_b){
 	pthread_t thread_ids[nthreads];
 	work_conv args[nthreads];

 	pthread_spin_init(&lock, pshared);

 	for(size_t i = 0; i < nthreads; i++){
 		args[i] = (work_conv) {
 			.matrix_a = mat_a,
 			.matrix_b = mat_b,
 			.id = i
 		};
 	}
 	convergence = 0;
 	for(int i = nthreads; i--; ){
 		pthread_create(thread_ids + i, NULL, worker_conv, args + i);
 	}
 	for(int i = nthreads; i--; ){
 		pthread_join(thread_ids[i], NULL);
 	}
 	
 	convergence = sqrt(convergence);
 	pthread_spin_destroy(&lock);
	// printf("%.3f\n", convergence);
 	return (convergence <= EPSILON);
 }
void pagerank(node* list, size_t npages, size_t nedges, size_t nthreads, double dampener) {
	
	if(nthreads > 32) {
		nthreads = 32;
	}
	nothreads = nthreads;
	nopages = npages;

    register double* old = initial_p(npages, nthreads);
    double* mat = calc_M(list, npages, nthreads, dampener);
    
    register double* new = new_matrix(npages); 
	
	size_t i;
	register pthread_t thread_ids[nthreads];
 	register work_mul args[nthreads];
 	size_t size = sizeof(double) * npages;

    while(true){
    	for(i = 0; i < nthreads; i++){
	 		args[i] = (work_mul) {
	 			.matrix_a = mat,
	 			.matrix_b = old,
	 			.result = new,
	 			.id = i
	 		};
	 	}
	 	for(i = nthreads; i--; ){
	 		pthread_create(thread_ids + i, NULL, worker_mul, args + i);
	 	}
	 	for(i = nthreads; i--; ){
	 		pthread_join(thread_ids[i], NULL);
	 	}
	 	if(is_convergence(npages, nthreads, new, old)){
 			free(old);		
 			break;
 		}
		memcpy(old, new, size);
	}

	node* current = list;
	for(i = 0; i < npages; i++) {
    	printf("%s %.8lf\n",current->page->name, new[i]);
    	current = current->next;
    }
	free(mat);
    free(new);
}

/*
######################################
### DO NOT MODIFY BELOW THIS POINT ###
######################################
*/

int main(int argc, char** argv) {

	/*
	######################################################
	### DO NOT MODIFY THE MAIN FUNCTION OR HEADER FILE ###
	######################################################
	*/

	config conf;

	init(&conf, argc, argv);

	node* list = conf.list;
	size_t npages = conf.npages;
	size_t nedges = conf.nedges;
	size_t nthreads = conf.nthreads;
	double dampener = conf.dampener;

	pagerank(list, npages, nedges, nthreads, dampener);

	release(list);

	return 0;
}
