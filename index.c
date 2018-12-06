#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "index.h"

struct index_ {
  FILE *file;
  char *string1;
  char *string2;
  int key;
  int position;
    /* to be implemented */
};

typedef struct {
} irecord;

int mycmp(const void *kptr, const void *e) {
  /*
  irecord *rec = *((irecord **) e);
  if (key < rec->key)
    return -1;
  else if (key > rec->key)
    return 1;
  else 
    return 0;
  */
}


/* 
   Creates a file for saving an empty index. The index is initialized
   to be of the specific tpe (in the basic version this is always INT)
   and to contain 0 entries.
 */
int index_create(int type) {
  FILE * f_create= NULL;
  index_t *index=NULL;

  f_create=fopen("name.txt","wb o r+");
  if (!f)
    return 1;

  fread(index->, sizeof(type_t),index->, index->);/*leemos la cabecera*/
  fwrite(type,sizeof(int),1,f_create);/*no hace falta escribir en create*/

  /*cerramos el fichero en index_close fclose(f);*/

  return 0;
  /*The index is initialized to be of the specific tpe (in the basic version this is always INT) and to contain 0 entries.*/

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
index_t* index_open(char* path) {
  index_t *index=NULL;
  FILE *f_open;

  if(!path)
    return NULL;
  
  f_open=fopen("name.txt","wb o r+");
  if(!f)
    return NULL;


  index= (index_t *)malloc(sizeof (index_t));
  if(!index)
    return NULL;

  /*Reads the contenets of the index in an index_t structure that it allocates*/
  fread(index->string1, sizeof(index_t), index->);
  fread(index->string2, sizeof(index_t), index->);
  /*return a pointer to it or NULL*/

  fclose(f_open);/* en la funcion index_close*/

  return NULL;
}

/* 
   Saves the current state of index in the file it came from. See the
   NOTE to index_open.
*/
int index_save(index_t* index, char* path) {
  index_t *index=NULL;
  FILE *f_save;
  int i;

  if(!index || !path)
    return 1;

  f_save=fopen(path ,"wb o r+");
  if(!f)
    return 1;

  for(i=0;i<index->;i++){
    /*fwrite(index->string1,sizeof(index_t),index->);
    fwrite(index->string2,sizeof(index_t),index->);*/
    fwrite(index->key,sizeof(int),1,f_save);
    fwrite(index->position,sizeof(int),1,f_save);
  }

  

  fclose(f_save);/*crep que mejor en index_close*/

  return 0;
}


/* 
   Puts a pair key-position in the index. NOTE: that the key may be
   present in the index or not... you must manage both situation. Also
   remember that the index must be kept ordered at all times.
*/
int index_put(index_t *index, int key, long pos) {
  if(!index || index->key<=key || index->position<=pos)
    return 0;

  strcpy(index->key,key);
  strcpy(index->position,pos);

  index->key=key;
  index->position=pos;

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

  if(!index || index->key<=key)
    return NULL;
  int n,found;
  long **poss = index_get(index, key, &n);


  for (i=0; i < &(nposs); i++) {
    /*
    Do something with poss[i]
    if()
      index->key = poss[i];
    */
    if(index->key == key){
      found = 1;
      break;
    }

  }

  if(found==1){

  }

  return NULL;
}

/* 
   Closes the index by freeing the allocated resources 
*/
void index_close(index_t *index) {
  if(!index)
    return;

  fclose(f_open);
  fclose(f_create);
  fclose(f_save);
  free(index);
}


