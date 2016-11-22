#ifndef SNAPSHOT_H
#define SNAPSHOT_H

#define MAX_KEY 16
#define MAX_LINE 1024
#define MAX_COMMAND 32

typedef struct entry entry;
typedef struct snapshot snapshot;

struct entry 
{
	char key[MAX_KEY];
	int* values;
	size_t length;
	entry* next;
	entry* prev;
};

struct snapshot 
{
	int id;
	entry* entries;
	int numberEntries;
	snapshot* next;
	snapshot* prev;
};

const char* HELP =
	"BYE   clear database and exit\n"
	"HELP  display this help message\n"
	"\n"
	"LIST KEYS       displays all keys in current state\n"
	"LIST ENTRIES    displays all entries in current state\n"
	"LIST SNAPSHOTS  displays all snapshots in the database\n"
	"\n"
	"GET <key>    displays entry values\n"
	"DEL <key>    deletes entry from current state\n"
	"PURGE <key>  deletes entry from current state and snapshots\n"
	"\n"
	"SET <key> <value ...>     sets entry values\n"
	"PUSH <key> <value ...>    pushes values to the front\n"
	"APPEND <key> <value ...>  appends values to the back\n"
	"\n"
	"PICK <key> <index>   displays value at index\n"
	"PLUCK <key> <index>  displays and removes value at index\n"
	"POP <key>            displays and removes the front value\n"
	"\n"
	"DROP <id>      deletes snapshot\n"
	"ROLLBACK <id>  restores to snapshot and deletes newer snapshots\n"
	"CHECKOUT <id>  replaces current state with a copy of snapshot\n"
	"SNAPSHOT       saves the current state as a snapshot\n"
	"\n"
	"MIN <key>  displays minimum value\n"
	"MAX <key>  displays maximum value\n"
	"SUM <key>  displays sum of values\n"
	"LEN <key>  displays number of values\n"
	"\n"
	"REV <key>   reverses order of values\n"
	"UNIQ <key>  removes repeated adjacent values\n"
	"SORT <key>  sorts values in ascending order\n"
	"\n"
	"DIFF <key> <key ...>   displays set difference of values in keys\n"
	"INTER <key> <key ...>  displays set intersection of values in keys\n"
	"UNION <key> <key ...>  displays set union of values in keys\n";
/*********************************************
**************HELPER METHODS******************
**********************************************/
/*
* checks if the string contains only numbers
*/
bool isNumber(const char *s);
/*
*   Helper method to clean all the memory
*/
void freeAll();
/*
* Helper method to initialize starting condition
*/
void initialize();
/*
build new snapshot with current entries
*/
snapshot* buildSnapshot();
/*
* Helper method to look for entry with the same key
*/
entry* findEntry(char *key);	
/*
* find the last entry in current state
*/
entry* findLastEntry();
/*
* Helper method to look for snapshot with the same key
*/
snapshot* findSnapshot(int id);
/*
*   Helper method to drop snapshot
*/
void help_drop(snapshot* list);
/*
*	Helper method to delete entry
*/
void help_delete(entry* entry);
/*
* Helper method to free current states and assign snapshot entries
*/
void assign_snapshot(int id);
/*
* Helper function to print values
*/
void print_values(int* values, int length);
/*
* Helper function to Merge Arrays left and right into A. 
*/
void merge(int *A,int *left,int leftCount,int *right,int rightCount);
// Recursive function to sort an numbers. 
void mergeSort(int *A,int n);
// compare method 
int compare(const void * a, const void * b);
/*********************************************
**************Main METHODS******************
**********************************************/
/*
* Method for diff inter and union
* finds the difference, intersection and union of sets
*/
void command_diff_union_inter(char* key, char keys[][100], int length, char* command);
/*
* sorts values in ascending order with mergesort
*/
void command_sort(char *key);
/*
* Removes repeated adjacent values
*/
void command_uniq(char *key);
/*
* reverses order of values
*/
void command_rev(char *key);
/*
* displays a sum inside the *key
*/
void command_sum(char *key);
/*
* displays a maximum value inside the *key
*/
void command_max(char *key);
/*
* displays a minimum value inside the *key
*/
void command_min(char *key);
/*
* deletes snapshot
*/
void command_drop(int id);
/*
* replaces current state with a copy of snapshot
*/
void command_checkout(int id);
/*
* saves the current state as a snapshot
*/
void command_snapshot();
/*
*  Deletes entry from current state
*/
void command_delete(char* key);
/*
* restores to snapshot with that id and deletes newer snapshots
*/
void command_rollback(int id);
/*
*displays and removes the front value
*/
void command_pop(char *key);
/*
*   displays entry values
*/
void command_get(char *key);
/*
*   sets entry values
*/
void command_set(char *key, int* values, int length);
/*
* Deletes entry from current state and snapshots
*/
void command_purge(char *key);
/*
*   Deletes value at index and prints that value
*/
void command_pluck(char *key, int* index);
/*
*	picks a key and returns a value at the index
*/
void command_pick(char *key, int* index);
/*
*   appends values to the back
*/
void command_append(char *key, int* values, int length);
/*
*   pushes values to the front
*/
void command_push(char *key, int* values, int length);
/*
* Method to show help
*/
void command_help();
/*
* prints bye and exit the program
*/
void command_bye ();
/*
*   displays all snapshots in the database
*/
void list_snapshots();
/*
*   displays all entries in current state
*/
void list_entries();
/*
* displays all keys in current state
*/
void list_keys();
/*
* Asks the user for input and calls the method according to the
* command
*/
void askInput();



#endif