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
  FILE *file;      /*El file es la columna de indice disco, la de en medio*/
  irecord *record; /*Array of records*/
  int nrecords;
  int type;
};

/*
int mycmp(const void *kptr, const void *e) {
  irecord *rec = *((irecord **)e);
  if (key < rec->key)
    return -1;
  else if (key > rec->key)
    return 1;
  else
    return 0;
}
*/

/* Returns 1 if it finds same key, 0 if not, -1 if error */
int bbin(int *m, index_t *index, int key){
	int last, first=0;

  if(!m || !index)
    return -1;

	last = index->nrecords;
	while (first <= last){
		*m = (first + last) / 2;
		if((index->record[*m]).key == key){
			return 1;
		}
		else if (key < (index->record[*m]).key)
			last = *m + 1;
		else
			first = *m - 1;
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

  f = fopen("index.dat", "a");
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
    /* Allocs as many positions as times repeated. */
    index->record[i].position = (long *)calloc(index->record[i].reps, sizeof(long));
    if (!(index->record[i].position)) {
      index_close(index);
      return NULL;
    }
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

  found = bbin(&m, index, key);

  /* si la clave que me pasan es igual a otra que haya ya en el index,
   * incremento reps en la de irecord */
  /* y guardo la direccion que me pasan desplazando todo -> mediante un
   * realloc()*/
  if (found == 1) {
    (index->record[m].reps)++;
    index->record[m].position = realloc(index->record[m].position, index->record[m].reps * sizeof(long));
    index->record[m].position[index->record[m].reps] = pos;
  }
  /* si la clave no es igual a ninguna, puede ser mayor y menor que las otras.
     tengo que incrementar
        el numero de records en la estruc index y ponerle el reps de la estruc
     irecord a 1 */
  /* realloc a record para que sea de tamanio nrecords * sizeof(irecord) */
  /* mover las menores (dcha) a la derecha 1 posicion */
  else if (found == 0) {
    (index->nrecords)++;
    index->record = realloc(index->record, index->nrecords * sizeof(irecord));
    for (i = m; i < index->nrecords; i++)
      index->record[i] = index->record[i + 1];
    index->record[m].reps = 1;
    index->record[m].key = key;
    index->record[m].position = (long *)calloc(1, sizeof(long));
    index->record[m].position[0] = pos;
  }
  /* si bbin da error me salgo. bbin return -1 en error */
  else {
    printf("Error en la funcion bbin\n");
    return 1;
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
  long **position = NULL;

  if (!index || key < 0 || key > 100 || !nposs)
    return NULL;

  position = (long **)calloc(1, sizeof(long *));
  if (!position)
    return NULL;

  /* miro si la clave que me pasan es igual a otra que haya ya en el index */
  found = bbin(&m, index, key);
  if (found != 1) {
		/* In the array i return there are 0 elements :) */
		*nposs = 0;
    return NULL;
  }

  *position = index->record[m].position;
  *nposs = index->record[m].reps;

  return position;
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
