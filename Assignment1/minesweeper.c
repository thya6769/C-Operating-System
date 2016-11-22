/**
 * comp2129 - assignment 1
 * <Takashi Hyakutake>
 * <thya6769>
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>


char intro;
int width, height;
int numberTimesFlagged = 0;
int firstCheck = 0;

// since max width and height = 100 with borders
int table[102][102];
	
/*
	Checks if the input is incorrect and prints error and exit.
*/
void checkError(char correctIntro1, char correctIntro2, int inputWidth, int inputHeight){
	// checks if the first character is an alphabet
	if(!isalpha(intro)){
		printf("error\n");
		exit(1);
	}

	if(intro == 'g' && firstCheck == 0){
		if(inputWidth < 1 || inputWidth > 100 ||
			inputHeight < 1 || inputHeight > 100 || inputWidth * inputHeight < 10) {
			printf("error\n");
			exit(1);
		}
	}else {
		if((intro != correctIntro1 && intro != correctIntro2)  
			|| inputWidth >= width || inputHeight >= height
			|| inputWidth < 0 || inputHeight < 0) {
			printf("error\n");
			exit(1);
		}
	}
	firstCheck = 1;
}


/*
	Prints the grid according to the current state of theh table.
*/
void printGrid(void){

	int actualWidth = width + 2;
	int actualHeight = height + 2;
	// +2 for the margins
	char grid[actualWidth][actualHeight];
		for(int j = 0; j < actualHeight; j++){
			for(int i = 0; i < actualWidth; i++){
			// corners
			if((i == 0 && j == 0) || (i == actualWidth - 1 && j == 0) 
				|| (i == 0 && j == actualHeight - 1) || (i == actualWidth - 1
					&& j == actualHeight - 1)){
				grid[i][j] = '+';
			} else if(i == 0 || i == actualWidth - 1) {
				grid[i][j] = '|';
			} else if(j == 0 || j == actualHeight - 1){
				grid[i][j] = '-';
			} else {
				// if flagged
				if(table[i][j] == 11 || table[i][j] == 12){
					grid[i][j] = 'f';
				// if uncovered set it to number of mines nearby
				} else if(table[i][j] < 9){
					grid[i][j] = table[i][j] + '0';
				} 
				else {
					grid[i][j] = '*';
				}
			}
			printf("%c", grid[i][j]);
		}
		printf("\n");
	}
}
/*
	This method asks for input from the user. 
 	0-8 for number of bombs nearby
 	9 = covered, 10 = bomb, 11 = flagged at right place, 12 flagged at wrong place
*/
void askForInput(int repeatTimes, char inputRequired1, char inputRequired2){
	for (int i = 0; i < repeatTimes; i++){	
		int widthSelected, heightSelected;
		char inst[20];
		fgets(inst, 20, stdin);
		
		sscanf(inst, " %c %d %d", &intro, &widthSelected, &heightSelected);

		checkError(inputRequired1, inputRequired2, widthSelected, heightSelected);

		printf("%c %d %d\n", intro, widthSelected, heightSelected);

		if(intro == 'g'){
			width = widthSelected;
			height = heightSelected;

		}else if(intro == 'b'){
			// if there is a bomb already
			if(table[widthSelected + 1][heightSelected + 1] == 10){
				printf("error\n");
				exit(1);
			}
			table[widthSelected + 1][heightSelected + 1] = 10;

		} else if(intro == 'u'){
			
			int minesNearby = 0;
			// if uncover the bomb or flagged prints lost
			if(table[widthSelected + 1][heightSelected + 1] == 10 
				|| table[widthSelected + 1][heightSelected + 1] == 11){
				printf("lost\n");
				exit(1);
			}
			// if you uncover the uncovered place
			if(table[widthSelected + 1][heightSelected + 1] < 9){
				printf("error\n");
				exit(1);
			}																																													
			for(int j = heightSelected; j < heightSelected + 3; j++){
				for(int i = widthSelected; i < widthSelected + 3; i++){
					if(table[i][j] == 10 || table[i][j] == 11){
						minesNearby++;
					}
				}
			}
			table[widthSelected + 1][heightSelected + 1] = minesNearby;
			printGrid();
		} else if(intro == 'f') {
			numberTimesFlagged++;
			if(numberTimesFlagged > 10){
				printf("error\n");
				exit(1);
			}
			// if flagged to the bomb or covered place then just continue
			if(table[widthSelected + 1][heightSelected + 1] == 10){
				
				table[widthSelected + 1][heightSelected + 1] = 11;
				printGrid();	
			} else if(table[widthSelected + 1][heightSelected + 1] == 9){
				table[widthSelected + 1][heightSelected + 1] = 12;
				printGrid();
				// if flagged to uncovered place print error
			} else {
				printf("error\n");
				exit(1);
			}
	
		} else {
			printf("error\n");
			exit(1);
		}
	} 
}

/*
	Main method
*/
int main(void){
	askForInput(1, 'g', 'g');
	int totalGrid = width * height;
	
	// initialise each table with covered = 9
	for(int j = 0; j < height + 2; j++){
		for(int i = 0; i < width + 2; i++){
			table[i][j] = 9;
		}
	}
	// set the bomb
	askForInput(10, 'b', 'b');
	printGrid();

	// user controls u and f
	askForInput(totalGrid, 'u', 'f');
	// if it reaches the end without error the user won.
	
	printf("won\n");

	return 0;
}
