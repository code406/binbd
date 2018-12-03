#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "table.h"
#include "type.h"

struct table_ {
	char* path;
	/*FILE* f;*/
	int ncols;
	type_t* types;
	long first_pos;
	long cur_pos;
	long last_pos;
};

/* 
   Creates a file that stores an empty table. This function doesn't
   keep any information in memory: it simply creates the file, stores
   the header information, and closes it. Completed, test pending
*/
void table_create(char* path, int ncols, type_t* types) {
	FILE * f = NULL;

	if(!path || ncols<1 || !types)	return;

	f = fopen(path, "wb");
	if(!f) return;
	
	/* stores  the header information */
	fwrite(&ncols, sizeof(int), 1, f);
	fwrite(types, sizeof(type_t), ncols, f);
  
	fclose(f);
	return;
}

/* 
   Opens a table given its file name. Returns a pointer to a structure
   with all the information necessary to manage the table. Returns
   NULL is the file doesn't exist or if there is any error.
*/
table_t* table_open(char* path) {
  table_t* table = NULL;
	FILE* f = NULL;

  if(!path) return NULL;
  
  table = (table_t *) malloc(sizeof(table_t));
  if(!table) return NULL;
  
	f = fopen(path, "rb");
  if(!f){
		free(table);
		return NULL;
  }
  
	fread(&table->ncols, sizeof(int), 1, f);
	table->types = (type_t*) malloc(table->ncols * sizeof(type_t));
	if(table->types==NULL){
		free(table);
		return NULL;
	}

	fread(table->types, sizeof(type_t), table->ncols, f);
	table->first_pos = ftell(f);
	fseek(table->file, 0, SEEK_END);
	table->last_pos = ftell(f);
	
	fclose(f);
	return table;
} 
 
  /* To be implemented
  table_ tabla;
  
	malloc(tabla)
	Leer
	fopen(rb)
	fread(loquehescritoantes. numero de columnas y tipos para asignar a variable)
	me quiero quedar con la posició de la cabecera. asigno el first_pos
	inicializamos el lastpos (HAY QUE ACTUALIZARLO CADA VEZ QUE LEO Y MODIFICO UN REGUSTRO)
	fclose()
	return la table;
	
  return NULL; */
}

/* 
   Closes a table freeing the alloc'ed resources and closing the file
   in which the table is stored.
*/
void table_close(table_t* table) {
  /* To be implemented
  fclose ficheri
  free,

  return; */
}

/* 
   Returns the number of columns of the table 
*/
int table_ncols(table_t* table) {
  /* To be implemented
  return table->ncols;
  return 0; */
}

/* 
   Returns the array with the data types of the columns of the
   table. Note that typically this kind of function doesn't make a
   copy of the array, rather, it returns a pointer to the actual array
   contained in the table structure. This means that the calling
   program should not, under any circumstance, modify the array that
   this function returns.
 */
type_t* table_types(table_t* table) {
  /* To be implemented
  return table->types;
  return NULL;*/
}

/* 
   Returns the position in the file of the first record of the table 
*/
long table_first_pos(table_t* table) {
  /* To be implemented 
  return table->first_pos;
  return 0L;*/
}

/* 
   Returns the position in the file in which the table is currently
   positioned. 
*/
long table_cur_pos(table_t* table) {

  return table->cur_pos;
  return 0L;
}

/* 
   Returns the position just past the last byte in the file, where a
   new record should be inserted.
*/
long table_last_pos(table_t* table) {
  /* To be implemented   
  return table->first_pos;
  return 0L;*/
}

/* 
   Reads the record starting in the specified position. The record is
   read and stored in memory, but no value is returned. The value
   returned is the position of the following record in the file or -1
   if the position requested is past the end of the file.
*/
long table_read_record(table_t* table, long pos) {
  /* To be implemented 
  pos=posicion en que voy a leer
  leo una estructura record 
  tendra tantos valores como ncols.
  leo el tamaño de cada valor y el valor concreto
	entero sin malloc, str con malloc ojo. (miro el tamaño (3) hago malloc tam 3)
	INSERT RECORD ES IGUAL QUE ESTA porque lo que he leido e slo que escribo.
  return poscion en que voy a leer el siguiente registro (donde me encuentro ahora: last_pos)
  return -1L;*/
}

/*
  Returns a pointer to the value of the given column of the record
  currently in memory. The value is cast to a void * (it is always a
  pointer: if the column is an INT, the function will return a pointer
  to it).. Returns NULL if there is no record in memory or if the
  column doesn't exist.
*/
void *table_column_get(table_t* table, int col) {
  /* To be implemented
  return NULL; */
}


/* 
   Inserts a record in the last available position of the table. Note
   that all the values are cast to void *, and that there is no
   indication of their actual type or even of how many values we ask
   to store... why?
  */
void table_insert_record(table_t* table, void** values) {
  /* To be implemented
  return; */
}

