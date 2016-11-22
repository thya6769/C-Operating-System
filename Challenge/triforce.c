#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>

/*
*	Helper method to trim the new line character
*/
void trim (char* s){
	int i = strlen(s) - 1;
	if(*(s + i) == '\n'){
		*(s + i) = '\0';
	}
}
bool isNumber(const char *s)
{
 for(int i = 0; i < strlen(s); i++){
   if(isalpha(*(s + i))){
      return false;
  }
  if(isdigit(*(s + i) - '0')){
      return false;
  }
}
return true;
}
int main(void) {
	
	char input[1000];
	int* height = (int* ) malloc(sizeof(int) * 20);
	printf("Enter height: ");
	fgets(input, 1000, stdin);
	printf("\n");
	
	trim(input);
	if(isNumber(input)){
		sscanf(input, "%d", height);
		int actualH = *height * 2;
		int actualW = *height * 4;
		int middle = *height * 2;
		int k = 1;
		
		if(*height < 2 || *height > 20){
			printf("Invalid height.\n");
			return 1;
		}

		for(int i = 0; i < actualH; i++){
			for (int j = 0; j < actualW; j++){

				if(j == middle - i - 1 || (i > *height - 1 
					&& j == middle + i - k)){
					printf("/");
				} else if (j == middle + i || 
					(i > *height - 1 && j == middle - i + k - 1)) {
					if(j == middle + i && i != actualH - 1){
						printf("\\\n");
						break;
					}
					printf("\\");
				} else if (j == actualW - 1) {
					printf("\n");
				} else if ((i == *height - 1|| i == actualH - 1)&& j < middle + i 
						   && j > middle - i - 1){
					printf("_");
				} 
				else {
					printf(" ");
				}
			}
			if(i > *height - 1){
				k += 2;
			}
		}
		printf("\n");
	}else {
		printf("Invalid height.\n");
	}
						   
	free(height);
	
	return 0;
}