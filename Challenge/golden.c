#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

 //compares if the float f1 is equal with f2 and returns 1 if true and 0 if false
 int compare_float(float f1, float f2){
  	float precision = 0.002;
  	if (((f1 - precision) < f2) && ((f1 + precision) > f2)){
    	return 1;
   	} else{
    	return 0;
   	}
 }
 int isfloat (const char *s)
{
     char *ep = NULL;
     strtod (s, &ep);

     if (!ep  ||  *ep)
         return 0;  // has non-floating digits after number, if any

     return 1;
}
int main(void) {
	printf("Enter two numbers: ");
	char input[100];
	fgets(input, 100, stdin);
	char* token = strtok(input, " ");
	printf("\n");

	float* a = (float* ) malloc(1000);
	int i = 0;
	while(token != NULL) {
		if(isnan(atof(token) / 0.0)){
			printf("Invalid input.\n");
			free(a);
			return 1;
		}
		*(a + i) = atof(token);
		token = strtok(NULL, " ");
		i++;
	}


	float ratio = (*a + *(a + 1)) / *a;
	float ratio2 = (*(a + 1) + *a) / *(a + 1);
	
	if(compare_float(ratio, 1.618) == 1|| compare_float(ratio2, 1.618) == 1) {
		printf("Golden ratio!\n");
	} else {
		printf("Maybe next time.\n");
	}
	free(a);
}