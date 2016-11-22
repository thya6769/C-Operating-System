#include <stdio.h>
#include <string.h>

int main(int argc, char* argv[]){
	if (argc < 2){
        printf("No arguments\n");
        return 1;
    }
    if(argc < 3 && argc > 0) {
		printf("Not enough arguments\n");
        return 1;
	} else if (argc > 3) {
		printf("Too many arguments\n");
        return 1;
	} 
	
	if(strlen(argv[1]) != strlen(argv[2])){
		printf("Invalid arguments\n");
	}

    char input[100];
    while(fgets(input, 100, stdin)!= NULL){
        for(int i = 0; i <strlen(input); i++){
            for(int j = 0; j < strlen(argv[1]); j++){

                if(input[i] == argv[1][j]){
                    input[i] = argv[2][j];
                }
            }
        }

    printf("%s", input);
    }
}
