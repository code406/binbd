#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "index.h"

typedef struct {
  int key;
  long *position; /*Position of each record with the same key  */
  int reps;       /*Number of records with the same key (score)*/
} irecord;

struct index_ {
  FILE *file;
  irecord *record; /*Array of records*/
  int nrecords;
  int type;
};

/*
	 Binary search algorithm
	 Returns 1 if it finds the key, 0 if not, -1 if error
*/
int bbin(int *m, index_t *index, int key){
	int first = 0, last;

	last = index->nrecords - 1;
	*m = (first+last)/2;

	while (first <= last) {
		if (index->record[*m].key > key)
			first = *m + 1;
		else if (index->record[*m].key == key)
			return 1;
		else
			last = *m - 1;

		*m = (first + last)/2;
		if ((first+last) < 0)
			(*m)--;
	}

	return 0;
}


/*
   Creates a file for saving an empty index. The index is initialized
   to be of the specific tpe (in the basic version this is always INT)
   and to contain 0 entries.
 */
int index_create(int type) {
  FILE *f = NULL;
	int cero = 0;

  f = fopen("index.dat", "w");
  if (!f)
    return 1;

  /* If new file, stores header information. If existent, not needed */
  if (ftell(f) == 0) {
    /* number of entries (0 as required) */
    fwrite(&cero, sizeof(int), 1, f);
    /* type of entries (int in this basic version, given value 0) */
    fwrite(&type, sizeof(int), 1, f);
  }

  fclose(f);
  return 0;
}

/*
   Opens a previously created index: reads the contents of the index
   in an index_t structure that it allocates, and returns a pointer to
   it (or NULL if the files doesn't exist or there is an error).

   NOTE: the index is stored in memory, so you can open and close the
   file in this function. However, when you are asked to save the
   index, you will not be given the path name again, so you must store
   in the structure either the FILE * (and in this case you must keep
   the file open) or the path (and in this case you will open the file
   again).
 */
index_t *index_open(char *path) {
  index_t *index = NULL;
  int i, j;

  if (!path)
    return NULL;

  index = (index_t *)calloc(1, sizeof(index_t));
  if (!index)
    return NULL;

  index->file = fopen(path, "r+");
  if (!index->file) {
    free(index);
    return NULL;
  }

  /* Reads header */
  fread(&index->nrecords, sizeof(int), 1, index->file);
  fread(&index->type, sizeof(int), 1, index->file);

  /* Allocs pointers for the records in the index */
  index->record = (irecord *)calloc(index->nrecords, sizeof(irecord));
  if (!index->record) {
    index_close(index);
    return NULL;
  }

  for (i = 0; i < index->nrecords; i++) {
    /* Gets number of times the key is repeated and the key value */
    fread(&(index->record[i].reps), sizeof(int), 1, index->file);
    fread(&(index->record[i].key), sizeof(int), 1, index->file);
    /* Allocs as many positions as times repeated */
    index->record[i].position = (long *)calloc(index->record[i].reps, sizeof(long));
    if (!(index->record[i].position)) {
      index_close(index);
      return NULL;
    }
		/* Stores as many positions as times repeated */
    for (j = 0; j < index->record[i].reps; j++)
      fread(&(index->record[i].position[j]), sizeof(long), 1, index->file);
  }

  return index;
}

/*
   Saves the current state of index in the file it came from. See the
   NOTE to index_open. (WE WON'T USE PATH, as file is stored in the structure)
*/
int index_save(index_t *index, char *path) {
  int i, j;

  if (!index || !path)
    return 1;

	/* Writes in file every record of the index structure */
  for (i = 0; i < index->nrecords; i++) {
    /* Write sample: 3 records with score 50 located in	1029, 2091 and 4922 */
    fwrite(&(index->record[i].reps), sizeof(int), 1, index->file);
    fwrite(&(index->record[i].key), sizeof(int), 1, index->file);
    for (j = 0; j < index->record[i].reps; j++)
      fwrite(&(index->record[i].position[j]), sizeof(long), 1, index->file);
  }

  return 0;
}

/*
   Puts a pair key-position in the index. NOTE: that the key may be
   present in the index or not... you must manage both situation. Also
   remember that the index must be kept ordered at all times.
*/
int index_put(index_t *index, int key, long pos) {
  int m, found, i;

  if (!index || key < 0 || key > 100)
    return 1;

	/* 1. Index is empty, no checks needed, insertion will follow order */
	if (index->nrecords == 0){
		(index->nrecords)++;
		index->record = calloc(1, sizeof(irecord));
		index->record[0].reps = 1;
		index->record[0].key = key;
		index->record[0].position = (long *)calloc(1, sizeof(long));
		index->record[0].position[0] = pos;
	}

	/* 2. Index has records in it. We check if the key already in the index */
	else{
		/* search for the key using binary search */
		found = bbin(&m, index, key);

		/* 2a. the key is already in the index */
	  if (found == 1) {
			/* 'm' is the index position where the key is found. */
			/* Increase number of times the key is repeated. Insert new position */
	    (index->record[m].reps)++;
	    index->record[m].position = realloc(index->record[m].position, index->record[m].reps * sizeof(long));
	    index->record[m].position[index->record[m].reps - 1] = pos;
	  }

		/* 2b. the key is not present in the index */
		else if (found == 0) {
			/* Make a new index record, move the records with smaller keys to right */
			/* 'm+1' is the position where we want to insert (in order) */
			/* Increase the number of records.   Key is not repeated: reps=1 */
	    (index->nrecords)++;
	    index->record = realloc(index->record, index->nrecords * sizeof(irecord));
	    for (i = index->nrecords-2; i >= m+1; i--)
	      index->record[i+1] = index->record[i];
	    index->record[m+1].reps = 1;
	    index->record[m+1].key = key;
	    index->record[m+1].position = (long *)calloc(1, sizeof(long));
	    index->record[m+1].position[0] = pos;
	  }
	  else {
	    printf("Error in the binary search function\n");
	    return 1;
	  }
	}

  return 0;
}

/*
   Retrieves all the positions associated with the key in the index.

   NOTE: the parameter nposs is not an array of integers: it is
   actually an integer variable that is passed by reference. In it you
   must store the number of elements in the array that you return,
   that is, the number of positions associated to the key. The call
   will be something like this:

   int n
   long **poss = index_get(index, key, &n);

   for (int i=0; i<n; i++) {
       Do something with poss[i]
   }

   ANOTHER NOTE: remember that the search for the key MUST BE DONE
   using binary search.
*/
long **index_get(index_t *index, int key, int* nposs) {
  int found, m;

  if (!index || key < 0 || key > 100 || !nposs)
    return NULL;

	/* Search the key in the index */
  found = bbin(&m, index, key);

	/* If there is no key like that in the index, stop */
	if (found != 1) {
		/* If the key is not present, it is repeated 0 times */
		*nposs = 0;
    return NULL;
  }

	*nposs = index->record[m].reps;
  return &index->record[m].position;
}

/*
   Closes the index by freeing the allocated resources
*/
void index_close(index_t *index) {
  int i;

  if (index) {
    if (index->file)
      fclose(index->file);
    if (index->record) {
      for (i = 0; i < index->nrecords; i++) {
        if (index->record[i].position)
          free(index->record[i].position);
      }
      free(index->record);
    }
    free(index);
  }

  return;
}


/******************************************************************************/
/************************  EXTRA FUNCTIONS  ***********************************/
/******************************************************************************/

/*
	 Prints the state of the index structure, for debug purposes
*/
void index_print(index_t *index) {
	int i, j;

	if (index) {
		printf("\n=================================================\n\nINDEX STATE:\n");
		if (index->record) {
			printf("Nrecords: %d\n", index->nrecords);
      for (i = 0; i < index->nrecords; i++) {
				printf("Key #%d: %d. Reps: %d\n", i, index->record[i].key, index->record[i].reps);
        for (j = 0; j<index->record[i].reps; j++)
					printf("Position #%d: %ld\n", j, index->record[i].position[j]);
  		}
		}
	}
}

/*
	Given the key, it looks for an index record and returns the index position
	(1,2,3) related to it. *nposs = number of positions associated to the key.
*/
int index_get_f1(index_t *index, int key, int* nposs) {
  int found, m;

  if (!index || key < 0 || key > 100 || !nposs)
    return -1;

	/* Search the key in the index */
  found = bbin(&m, index, key);

	/* If there is no key like that in the index, stop */
	if (found != 1) {
		/* If the key is not present, it is repeated 0 times */
		*nposs = 0;
    return -1;
  }

	*nposs = index->record[m].reps;
  return m;
}

/*
	Given an index position (1,2,3) it retrieves all "table" positions associated
	with the key in the index. *nposs = number of positions associated to the key.
*/
long **index_get_f2(index_t *index, int index_number, int *nposs) {
  if (!nposs || index_number < 0 || index_number > 100)
    return NULL;

	*nposs = index->record[index_number].reps;
  return &index->record[index_number].position;
}
