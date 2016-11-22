#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
	
	float* number = (float *) malloc(1000);
	printf("Enter data set:\n");
	float mean = 0;
	float variance = 0;
	float sd = 0;
	char input[100];
	int i = 0;
	char* token;
	while(fgets(input, 100, stdin) != NULL) {
		token = strtok(input, " ");
		while(token != NULL){
			*(number + i) = atof(token);
			token = strtok(NULL, " ");
			i++;
		}
	}

	float sum = 0;
	for (int j = 0; j < i; j++){
		sum += *(number + j);
	}
	mean = sum / i;
	for (int j = 0; j < i; j++){
		variance += pow(*(number + j) - mean, 2);
	}
	variance /= i;
	sd = sqrt(variance);
	
	printf("Mean = %.4f\n", mean);
	printf("Variance = %.4f\n", variance);
	printf("Standard deviation = %.4f\n", sd);

	free(number);
	return 0;
}