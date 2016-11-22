/**
 * comp2129 - assignment 2
 * <Takashi Hyakutake>
 * <thya6769>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdbool.h>
#include <ctype.h>
#include "snapshot.h"

 // set of connections made by entry_head and tail is the current state
 entry* entry_head = NULL;
 entry* entry_tail = NULL;

 snapshot* snapshot_head = NULL;
 snapshot* snapshot_tail = NULL;

 int snapshotID = 1;
 int numberOfEntries = 0;

/*********************************************
**************HELPER METHODS******************
**********************************************/
/*
* checks if the string contains only numbers
*/
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
/*
*	Helper method to trim the new line character
*/
void trim (char* s){
	int i = strlen(s) - 1;
	if(*(s + i) == '\n'){
		*(s + i) = '\0';
	}
}
/*
*   Helper method to clean all the memory
*/
void freeAll() {
    entry *p = entry_head;
    if(numberOfEntries != 0) {
        p = entry_head->next;
        entry* temp;
        while(p != entry_tail) {
            temp = p->next;
            free(p->values);
            free(p);
            p = temp;
        }
    }
    // clear all the memory inside the snapshot
    if(snapshotID != 1) {
        snapshot *ptr = snapshot_head;
        ptr = snapshot_head->next;
        snapshot *tempB;
        while(p != entry_tail) {
            tempB = ptr->next;
            for(int i = 0; i < ptr->numberEntries; i ++){
                entry* entry = ptr->entries + i;
                free(entry->values);
            }
            free(ptr->entries);

            free(ptr);
            ptr = tempB;
        }
    }
}
/*
* Helper method to initialize starting condition
*/
void initialize(){
    entry_head = (entry* )malloc(sizeof(entry));
    entry_tail = (entry* )malloc(sizeof(entry));
    entry_head->next = entry_tail;
    entry_head->prev = entry_tail;
    entry_tail->next = entry_head;
    entry_tail->prev = entry_head;

    snapshot_head = (snapshot* )malloc(sizeof(snapshot));
    snapshot_tail = (snapshot* )malloc(sizeof(snapshot));
    snapshot_head->next = snapshot_tail;
    snapshot_head->prev = snapshot_tail;
    snapshot_tail->next = snapshot_head;
    snapshot_tail->prev = snapshot_head;
}
/*
build new snapshot with current entries
*/
snapshot* buildSnapshot(){
    snapshot* list = (snapshot* ) malloc(sizeof(snapshot));
    entry* snapEntry = malloc(10*sizeof(entry));

    list->id = snapshotID;
    // list of entries that will be stored inside snapshot
    // create new memory for values of each entry
    
    // save entries to snapshot
    entry* start = entry_head; 
    start = start->next;
    int i = 0; 
    while(start != entry_tail){
        entry* listEntry = snapEntry + i;
        int* value = (int*)malloc(20*sizeof(int));

        // make deep copy of current state
        memcpy(listEntry->key, start->key, MAX_KEY*sizeof(char));
        listEntry->length = start->length;
        memcpy(value, start->values, start->length*sizeof(int));
        listEntry->values = value;

        i++;

        start = start ->next;
    }
    list->entries = snapEntry;
    list->numberEntries = i;
    return list;
}
/*
* Helper method to look for entry with the same key
*/
entry* findEntry(char *key){
    entry* start = entry_head;
    if(numberOfEntries == 0){
    	return NULL;
    }
    start = start->next;
    while(start != entry_tail){
        if(strcmp(start->key, key) == 0){
            return start;
        }
        start = start->next;
    }
    return NULL;
}
/*
* find the last entry in current state
*/
entry* findLastEntry(){
    entry* next;
    entry* entry = entry_head;
    next = entry->next;
    while(next != entry_tail){
        entry = entry->next;
        next = entry->next;
    }
    return entry;
}
/*
* Helper method to look for snapshot with the same key
*/
snapshot* findSnapshot(int id){
    snapshot* start = snapshot_head;
    start = start->next;

    while(start != snapshot_tail){
        if(start->id == id){
            return start;
        }
        start = start->next;
    }
    return NULL;
}
/*
*   Helper method to drop snapshot
*/
void help_drop(snapshot* list){
    list->prev->next = list->next;
    list->next->prev = list->prev;
    for(int i = 0; i < list->numberEntries; i ++){
        entry* entry = list->entries + i;
        free(entry->values);
    }
    free(list->entries);    
    free(list);
}
/*
*	Helper method to delete entry
*/
void help_delete(entry* entry){
    entry->prev->next = entry->next;
    entry->next->prev = entry->prev;
    free(entry->values);
    free(entry);
}
/*
* Helper method to free current states and assign snapshot entries
*/
void assign_snapshot(int id){
	// free current state
	entry* temp = entry_head;
	entry* next = temp->next;
	for(int i = 0; i < numberOfEntries; i++){
		temp = next;
		next = temp->next;
		help_delete(temp);
	}
	// update current state
	snapshot* snap = findSnapshot(id);
	numberOfEntries = snap->numberEntries;
	entry* start = entry_head;
	for(int i = 0; i < numberOfEntries; i++){
		entry* entry = malloc(10*sizeof(entry));

		*entry = *(snap->entries + i);

		start->next = entry;
		entry->prev = start;   

		start = start->next;
	}
	start->next = entry_tail;
	entry_tail->prev = start;
}

/*
* Helper function to print values
*/
void print_values(int* values, int length){
	printf("[");
    for(int i = 0; i < length;i++){
        if(i == length - 1){
            printf("%d", *(values+i));
        } else {
            printf("%d ", *(values+i));
        }
    } 
    printf("]\n");
}
/*
* Helper function to Merge Arrays left and right into A. 
*/
void merge(int *A, int *left,int leftCount,int *right,int rightCount) {
    int i = 0;
    int j = 0;
    int k = 0;

    while(i < leftCount && j < rightCount) {
        if(left[i]  < right[j]){ 
            A[k++] = left[i++];
        } else {
            A[k++] = right[j++];
        }
    }
    while(i < leftCount){
        A[k++] = left[i++];
    }
    while(j < rightCount){ 
        A[k++] = right[j++];
    }
}
// Recursive function to sort an array of integers. 
void mergeSort(int *A,int n) {
    int mid,i, *left, *right;
    if(n < 2){
        return; 
    }

    mid = n/2;  

    left = (int*)malloc(mid*sizeof(int)); 
    right = (int*)malloc((n- mid)*sizeof(int)); 
    
    for(i = 0;i<mid;i++){
        left[i] = A[i]; 
    }
    for(i = mid;i<n;i++){ 
        right[i-mid] = A[i]; 
    }
    mergeSort(left,mid);  
    mergeSort(right,n-mid);  
    merge(A,left,mid,right,n-mid); 
    free(left);
    free(right);
}
// compare method 
int compare(const void * a, const void * b){
 return ( *(int*)a - *(int*)b );
}
/*********************************************
**************Main METHODS******************
**********************************************/
/*
* Method for diff inter and union
* finds the difference, intersection and union of sets
*/
void command_diff_union_inter(char* key, char keys[][100], int length, char* command){
	entry* entry1 = findEntry(key);
    if(entry1 != NULL){
    	int entry1Sorted[1000];
    	memcpy(entry1Sorted, entry1->values, entry1->length*sizeof(int));


    	int* item;
    	int numberlength = entry1->length;
    	mergeSort(entry1Sorted, numberlength);

    	int current[1000];
    	int count;
    	for(int i = 0; i < length; i++){
          entry* entry2 = findEntry(keys[i]);    		
          if(entry2 == NULL){
             printf("no such key\n");
             return;
         }
         int entry2Sorted[1000];
         memcpy(entry2Sorted, entry2->values, entry2->length*sizeof(int));
         mergeSort(entry2Sorted, entry2->length);

         count = 0;
         if(strcasecmp(command, "INTER") == 0){
          
             for(int j = 0; j < numberlength; j++){
		    		//printf("%zu ", entry2->length)
                item = (int*) bsearch(&entry1Sorted[j], entry2Sorted
                   , numberlength, sizeof (int), compare);
		    		// if value is found in entry2
                if(item != NULL){
                   current[count] = entry1Sorted[j];
                   count++;
               }    			
           }    
           numberlength = count;
           memcpy(entry1Sorted, current, numberlength*sizeof(int));
           
       } else if(strcasecmp(command, "DIFF") == 0){
         int* item2;
         for(int j = 0; j < numberlength; j++){
		    		//printf("%zu ", entry2->length);
            item = (int*) bsearch(&entry1Sorted[j], entry2Sorted
               , entry2->length, sizeof (int), compare);
		    		// if value is not found in entry2
            if(item == NULL){
               current[count] = entry1Sorted[j];
               count++;
           }
           
       }

       for(int j = 0; j < entry2->length; j++){
           item2 = (int*) bsearch(&entry2Sorted[j], entry1Sorted
               , entry2->length, sizeof (int), compare);
           if(item2 == NULL){
               current[count] = entry2Sorted[j];
               count++;
           }
       }
       numberlength = count;
       memcpy(entry1Sorted, current, count*sizeof(int));
       
   } else {
     for(int j = 0; j < entry2->length; j++){
		    		//printf("%zu ", entry2->length);
        item = (int*) bsearch(&entry2Sorted[j], entry1Sorted
           , numberlength, sizeof (int), compare);
		    		// if value is not found in entry2
        if(item == NULL){
           entry1Sorted[numberlength] = entry2Sorted[j];
           numberlength++;
       }	
   }
}
}
print_values(entry1Sorted, numberlength);
printf("\n");
} else {
   printf("no such key\n");
}
}

/*
* sorts values in ascending order with mergesort
*/
void command_sort(char *key){
    entry* entry = findEntry(key);
    if(entry != NULL){
      mergeSort(entry->values, entry->length);

      printf("ok\n");
  } else {
      printf("no such key\n");
  }	
}
/*
* Removes repeated adjacent values
*/
void command_uniq(char *key){
    entry* entry = findEntry(key);
    if(entry != NULL){
       int lastRead = -1;
       int* inpp = entry->values;
       int* outp = entry->values;
       int length = 0;
       for(int i = 0; i < entry->length; i++){
	        //printf("%d\n ", length);
           if (*inpp != lastRead) {
	            //printf("outp: %d ", *outp);
               *outp++ = lastRead = *inpp;
               length++;
           } 
	        //printf("inpp: %d last: %d\n", *inpp, lastRead);
           inpp++;
       }
       *outp = 0;
       int * value = malloc(10*sizeof(int));
       memcpy(value, entry->values, length*sizeof(int));
       free(entry->values);
       entry->values = value;
       entry->length = length;

       printf("ok\n");
   } else {
      printf("no such key\n");
  }
}
/*
* reverses order of values
*/
void command_rev(char *key){
    entry* entry = findEntry(key);
    if(entry != NULL){
       int* values = malloc(entry->length*sizeof(int));
       int j = 0;
       for(int i = entry->length - 1; i >=0; i--){
           *(values + j) = *(entry->values + i);
           j++;
       }
       free(entry->values);
       entry->values = values;
       printf("ok\n");
   } else {
      printf("no such key\n");
  }
}
/*
* displays number of values
*/
void command_len(char *key){
    entry* entry = findEntry(key);
    if(entry!= NULL){
    	printf("%zu\n", entry->length);
    } else {
    	printf("no such key\n");
    }
}
/*
* displays a sum inside the *key
*/
void command_sum(char *key){
    entry* entry = findEntry(key);
    if(entry != NULL){

       int sum = 0;
       for(int i = 0; i < entry->length; i++){
           sum += *(entry->values + i);
       }
       printf("%d\n", sum);
   }else {
       printf("no such key\n");
   }
}
/*
* displays a maximum value inside the *key
*/
void command_max(char *key){
    entry* entry = findEntry(key);
    if(entry != NULL){

       int maximum = *entry->values;
       for(int i = 1; i < entry->length; i++){
           if(*(entry->values + i) > maximum){
               maximum = *(entry->values + i);
           }
       }
       printf("%d\n", maximum);
   } else {
       printf("no such key\n");
   }
}
/*
* displays a minimum value inside the *key
*/
void command_min(char *key){
    entry* entry = findEntry(key);
    if(entry != NULL){
       int minimum = *entry->values;
       for(int i = 1; i < entry->length; i++){
           if(*(entry->values + i) < minimum){
               minimum = *(entry->values + i);
           }
       }
       printf("%d\n", minimum);
   } else {
      printf("no such key\n");
  }
}
/*
* deletes snapshot
*/
void command_drop(int id){
    snapshot* list = findSnapshot(id);
    if(list != NULL){
        help_drop(list);
        printf("ok\n");

    } else {
        printf("no such snapshot\n");
    }
}
/*
* replaces current state with a copy of snapshot
*/
void command_checkout(int id){
    snapshot* snap = findSnapshot(id);

    if(snap == NULL){
        printf("no such snapshot\n");
    } else {
    	assign_snapshot(id);
        printf("ok\n");
    }
}
/*
* saves the current state as a snapshot
*/
void command_snapshot() {
    snapshot* list = buildSnapshot();
    // link snapshots
    snapshot* temp;
    if(snapshotID == 1){
        temp = snapshot_head;
    } else {
        temp = findSnapshot(snapshotID - 1);
    } 

    list->next = temp->next;
    temp->next->prev = list;
    temp->next = list;
    list->prev = temp;
    
    printf("saved as snapshot %d\n", snapshotID);
    snapshotID++;
}
/*
*  Deletes entry from current state
*/
void command_delete(char* key){
    entry* entry = findEntry(key);
    if(entry != NULL){
    	numberOfEntries--;
        help_delete(entry);
        printf("ok\n");
    } else {
        printf("no such key\n");

    }
}
/*
* restores to snapshot with that id and deletes newer snapshots
*/
void command_rollback(int id) {
    if(findSnapshot(id) == NULL){
        printf("no such snapshot\n");
    } else {
        // find snapshots that is newer than id and deletes it
        for(int i = id + 1; i < snapshotID; i++){
            snapshot* list = findSnapshot(i);
            help_drop(list);
        }
        assign_snapshot(id);
        
        snapshotID = id + 1;
        printf("ok\n");
    }
}
/*
*displays and removes the front value
*/
void command_pop(char *key){
    entry* start = findEntry(key);
    
    if(start == NULL){
       printf("no such key\n");
   }
   else {
       if(start->length == 0){
           printf("nil\n");
           return;
       } 
       int *temp = (int* )malloc(start->length*sizeof(int));
            // new memory allocated with new values            
       for(int i = 0; i < start->length; i++){
        *(temp + i) = *(start->values + i + 1);
    }
    
    printf("%d\n", *start->values);
    free(start->values);
    start->values = temp;
    start->length--;
}    
}
/*
*   displays entry values
*/
void command_get(char *key){
    entry* start = findEntry(key);
    if(start != NULL){
        print_values(start->values, start->length);
    }
    else{
        printf("no such key\n");
    }
}
/*
*   sets entry values
*/
void command_set(char *key, int* values, int length){   
    // if the entry with that key exists replace that entry with 
    // new values
    entry* temp = findEntry(key);
    if(temp != NULL){
    	free(temp->values);
        temp->values = values;
        temp->length = length;
    } else {
        entry* node = (entry* )malloc(sizeof(entry));            
        node->values = values;
        node->length = length;
        memcpy(node->key, key, sizeof(char) * MAX_KEY);
        // find the last entry and add it there
        temp = findLastEntry();
        
        node->next = temp->next;
        node->prev = temp;
        temp->next->prev = node;
        temp->next = node;

        numberOfEntries++;
    }
    printf("ok\n");
}
/*
* Deletes entry from current state and snapshots
*/
void command_purge(char *key){
    entry* entryA = findEntry(key);
    if(entryA == NULL){
        printf("ok\n");
    }else{
        entry* entryB;
        entry* node;
        int* value;
        // deletes entry with the same *key in the snapshot
        for(int i = 1; i < snapshotID; i++){
            snapshot* snap = findSnapshot(i);
            entry* entry = malloc(10*sizeof(entry));
            // loop until the end of entries
            int k = 0;
            for(int j = 0; j < snap->numberEntries; j++){
                entryB = entry + k;
                node = snap->entries + j;
                value = malloc(sizeof(int) * node->length);

                // if the entry has the key to delete skip
                if(strcasecmp(node->key, key) == 0){
                    // free the above value
                    free(value);
                    // free the value contained within
                    free(node->values);
                    continue;
                } else {
                    // copy the values
                    memcpy(value, node->values, sizeof(int) * node->length);
                    // copy the key
                    memcpy(entryB->key, node->key, sizeof(char) * MAX_KEY);
                    // copy the length
                    entryB->length = node->length;

                    entryB->values = value;
                    free(node->values);
                    k++;
                }
            }
            free(snap->entries);
            snap->entries = entry;
            snap->numberEntries = snap->numberEntries - 1;
        }
        command_delete(key);    
    }
}
/*
*   Deletes value at index and prints that value
*/
void command_pluck(char *key, int* index){
    entry* node = findEntry(key);
    if(node != NULL){
        if(*index < 1 || *index > node->length){
            printf("index out of range\n");
            free(index);
            return;
        } else {
            int*temp = (int* ) malloc(node->length*sizeof(int));
            for(int i = 0; i < node->length; i++){
                if(*index - 1 == i){
                    continue;
                }
                *(temp + i) = *(node->values + i);
            }
            printf("%d\n", *(node->values + *index - 1));
            node->length--;
            free(node->values);
            node->values = temp;
        }
    } else {
        printf("no such key\n");
    }
    free(index);
}
/*
*	picks a key and returns a value at the index
*/
void command_pick(char *key, int* index) {
    entry* node = findEntry(key);
    if(node != NULL){
        if(*index < 1 || *index > node->length) {
            printf("index out of range\n");
            free(index);
            return;
        } else {
            printf("%d\n", *(node->values + (*index - 1)));
        }
    } else {
        printf("no such key\n");
    }
    free(index);
}
/*
*   appends values to the back
*/
void command_append(char *key, int* values, int length){
    entry* start = findEntry(key);
    if(start != NULL){
        int j = start->length;
        for(int i = 0; i < length; i++){
            *(start->values + j) = *(values + i);
            j++;
        }
        // update length
        start->length = start->length + length;
        printf("ok\n");
    }else {
    	printf("no such key\n");
    } 
    free(values);
}
/*
*   pushes values to the front
*/
void command_push(char *key, int* values, int length){
    entry* start = findEntry(key);
    if(start != NULL){
        // save new values into new memory    
        int j = length - 1;
        int i;
        int num[MAX_LINE];
        // push the values into array
        for(i = 0; i < length; i++){
            num[i] = *(values + j); 
            j--;
        }
        // assigning it back to values
        for(i = 0; i < length; i++){
            *(values + i) = num[i];
        }
        // append the rest of values
        for(j = 0; j < start->length; j++){
            *(values + i) = *(start->values + j);
            i++;
        }
        free(start->values);
        start->values = values;
        start->length = i;

        printf("ok\n");
    } else {
    	printf("no such key\n");
    	free(values);
    }
}
/*
* Method to show help
*/
void command_help() {
    printf("%s", HELP);
}
/*
* prints bye and exit the program
*/
void command_bye () {
    printf("bye\n");
    freeAll();
    exit(1);
}
/*
*   displays all snapshots in the database
*/
void list_snapshots() {
    if(snapshotID == 1){
        printf("no snapshots\n");
    } else {
        for(int i = 1; i < snapshotID; i++){
            printf("%d\n", i);
        }
    }
}
/*
*   displays all entries in current state
*/
void list_entries() {
    if(numberOfEntries == 0){
        printf("no entries\n");
    } else{
        entry* entry = entry_tail;

        for(int i = numberOfEntries - 1; i>= 0; i--){
            entry = entry->prev;
            printf("%s ", entry->key);
            command_get(entry->key);
        }
    }
}
/*
* displays all keys in current state
*/
void list_keys() {
    if(numberOfEntries == 0){
        printf("no keys\n");
    } else {
        entry* entry = entry_tail;
        for(int i = numberOfEntries - 1; i >= 0; i--){
            entry = entry->prev;
            printf("%s\n", entry->key);
        }
    }    
}

/*
* Asks the user for input and calls the method according to the
* command
*/
void askInput(){
    char line[MAX_LINE];
    char* token;
    while (true) {
        printf("> ");
        
        // only commands
        if (fgets(line, MAX_LINE, stdin) == NULL) {
        	freeAll();
            printf("error\n");
            exit(1);
        } else if(strcasecmp(line, "BYE\n") == 0){
            command_bye();
        } else if (strcasecmp(line, "HELP\n") == 0){
            command_help();
        } else if (strcasecmp(line, "LIST KEYS\n") == 0){
            list_keys();
        } else if (strcasecmp(line, "LIST ENTRIES\n") == 0) {
            list_entries();
        } else if (strcasecmp(line, "LIST SNAPSHOTS\n") == 0) {
            list_snapshots();
        } else if (strcasecmp(line, "SNAPSHOT\n") == 0) {
            command_snapshot();
        // with key and values
        } else {
            char command[MAX_COMMAND];
            char key[MAX_KEY];

            trim(line);
            sscanf(line, "%s %s", command, key);
            if(strlen(command) > MAX_COMMAND || strlen(key) > MAX_KEY){
            	freeAll();
            	printf("error\n");
            	exit(1);
            }
            int* values = (int* )malloc(MAX_LINE*sizeof(int));

            token = strtok(line, " ");
            int length = 0;

            // skip the first two because its command and key
            for(int i = 0; i < 2; i++){
                token = strtok(NULL, " ");
            }

            char keys[100][100];
            
            bool onlyValues = true;
            while(token != NULL){
              if(isNumber(token)) {
                  *(values + length) = atoi(token);
              } else {
                onlyValues = false;
                strcpy(keys[length], token);
            }
            length++;
            token = strtok(NULL, " ");
        }

        if(strcasecmp(command, "SET") == 0 && onlyValues){
            command_set(key, values, length);
        } else if(strcasecmp(command, "PUSH") == 0 && onlyValues){
            command_push(key, values, length);
        } else if(strcasecmp(command, "APPEND") == 0 && onlyValues){
            command_append(key, values, length);
        } else if (strcasecmp(command, "PICK") == 0 && onlyValues){
            command_pick(key, values);
        } else if (strcasecmp(command, "PLUCK") == 0 && onlyValues) {
            command_pluck(key, values);
        } else {
           if (strcasecmp(command, "GET") == 0){
               command_get(key);
           } else if (strcasecmp(command, "POP") == 0){
               command_pop(key);
           } else if (strcasecmp(command, "DEL")==0) {
               command_delete(key);
           } else if (strcasecmp(command, "PURGE") == 0) {
               command_purge(key);
           } else if (strcasecmp(command, "DROP") == 0) {
               command_drop(key[0] - '0');
           } else if (strcasecmp(command, "ROLLBACK") == 0) {
               command_rollback(key[0] - '0');
           } else if (strcasecmp(command, "CHECKOUT") == 0) {
               command_checkout(key[0] - '0');
           } else if (strcasecmp(command, "MIN") == 0){
               command_min(key);
           }else if (strcasecmp(command, "MAX") == 0){
               command_max(key);
           }else if (strcasecmp(command, "SUM") == 0){
               command_sum(key);
           }else if (strcasecmp(command, "LEN") == 0){
               command_len(key);
           }else if (strcasecmp(command, "REV") == 0){
               command_rev(key);
           }else if (strcasecmp(command, "UNIQ") == 0){
               command_uniq(key);
           } else if (strcasecmp(command, "SORT") == 0){
               command_sort(key);
           } else if (strcasecmp(command, "DIFF") == 0 
              || strcasecmp(command, "INTER") == 0
              || strcasecmp(command, "UNION") == 0){ 
               command_diff_union_inter(key, keys, length, command);
           }else {
               printf("please input valid commands\n");
           }
           free(values);
       }
   }
   printf("\n");
}
}
/*
*   Main method
*/
int main(void) {
    initialize();
    askInput();
    return 0;
}
