#include <immintrin.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
   printf("Input strand: ");
   char input[100];
   fgets(input, 100, stdin);
   char output[100];
   int length = 0;
   printf("\n");
   for(size_t i = 0; i < strlen(input); i++){
        if(input[i] == 'G'){
            if(input[i + 1] == 'U'){
                if(input[i + 2] == 'G'){
                    if(input[i + 3] == 'U'){
                        i += 4;
                        // loop through the intron
                        for( ; i < strlen(input); i++){
                            if(input[i] == 'A'){
                                if(input[i+1] == 'G'){
                                    if(input[i+2] == 'A'){
                                        if(input[i+3] == 'G'){
                                            i+=3;
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                    }else {
                        output[length] = input[i];
                        length++;
                    }    
                } else {
                    output[length] = input[i];
                    length++;
                } 
            } else {
                output[length] = input[i];
                length++;
            } 
        } else {
            output[length] = input[i];
            length++;
        } 
   }
    printf("Output is ");
   for(size_t i = 0; i < length; i++){
        if(output[i] == '\n'){
            break;
        }
        printf("%c", output[i]);
   }
   printf("\n");
   return 0;
}