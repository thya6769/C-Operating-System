#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>


int main(void) {
	int* key = malloc(sizeof(int)*20);
	char input[100];
	printf("Enter key: ");
	fgets(input, 100, stdin);
	sscanf(input, "%d", key);
	if(*key < 0 || *key > 26){
		printf("\nInvalid key!\n");
		free(key);
		exit(1);
	}
	printf("Enter line: ");
	fgets(input, 100, stdin);
	char output[100];
	printf("\n");
	for(size_t i = 0; i < strlen(input); i++){
		if(input[i] == ' '){
			output[i] = ' ';
		} else if(ispunct(input[i])){
			output[i] = input[i];
		} else {
			output[i] = input[i];
			for(size_t j = 0; j < *key; j++){
				if (input[i] + j == (char) 90){
					output[i] = (char) 65;
				} else if(input[i] + j == (char) 122) {
					output[i] = (char) 97;
				}else {
					output[i]++;
				}
			}
		}
	}
	for(size_t i = 0; i < strlen(input) - 1; i++){
		printf("%c", output[i]);
		// printf("%d\n", output[i]);
	}
	printf("\n");
	free(key);
	return 0;
}