#include "table.h"
#include "type.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct table_ {
  FILE *file;
  int ncols;
  type_t *types;
  long first_pos;
  long cur_pos;  /*to insert*/
  long last_pos; /*to read*/
  void **buff;
};

/*
   Creates a file that stores an empty table. This function doesn't
   keep any information in memory: it simply creates the file, stores
   the header information, and closes it. Completed, test pending
*/
void table_create(char *path, int ncols, type_t *types) {
  FILE *f = NULL;

  if (!path || ncols < 1 || !types)
    return;

  f = fopen(path, "w");
  if (!f)
    return;

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
table_t *table_open(char *path) {
  table_t *table = NULL;

  if (!path)
    return NULL;

  table = (table_t *)malloc(sizeof(table_t));
  if (!table)
    return NULL;

  table->file = fopen(path, "r+");
  if (!table->file) {
    free(table);
    return NULL;
  }

  fseek(table->file, 0, SEEK_SET);
  fread(&table->ncols, sizeof(int), 1, table->file);

  table->types = (type_t *)malloc(table->ncols * sizeof(type_t));
  if (!table->types) {
    free(table);
    fclose(table->file);
    return NULL;
  }

  /* Reads header first, assigns first and last pos */
  fread(table->types, sizeof(type_t), table->ncols, table->file);
  table->first_pos = ftell(table->file);
  table->cur_pos = table->first_pos;
  fseek(table->file, 0, SEEK_END);
  table->last_pos = ftell(table->file);

  return table;
}

/*
   Closes a table freeing the alloc'ed resources and closing the file
   in which the table is stored.
*/
void table_close(table_t *table) {
  if (table) {
    fclose(table->file);
    free(table->types);
    free(table);
  }
}

/*
   Returns the number of columns of the table
*/
int table_ncols(table_t *table) {
  if (!table)
    return -1;
  return table->ncols;
}

/*
   Returns the array with the data types of the columns of the
   table. Note that typically this kind of function doesn't make a
   copy of the array, rather, it returns a pointer to the actual array
   contained in the table structure. This means that the calling
   program should not, under any circumstance, modify the array that
   this function returns.
 */
type_t *table_types(table_t *table) {
  if (!table)
    return NULL;
  return table->types;
}

/*
   Returns the position in the file of the first record of the table
*/
long table_first_pos(table_t *table) {
  if (!table)
    return -1L;
  return table->first_pos;
}

/*
   Returns the position in the file in which the table is currently
   positioned.
*/
long table_cur_pos(table_t *table) {
  if (!table)
    return -1L;
  return table->cur_pos;
}

/*
   Returns the position just past the last byte in the file, where a
   new record should be inserted.
*/
long table_last_pos(table_t *table) {
  if (!table)
    return -1L;
  return table->last_pos;
}

/*
   Reads the record starting in the specified position. The record is
   read and stored in memory, but no value is returned. The value
   returned is the position of the following record in the file or -1
   if the position requested is past the end of the file.
*/
long table_read_record(table_t *table, long pos) {
  size_t size = 0;
  int i;

  if (!table)
    return -1L;

  /* If this happens, we get out of "recursion" */
  if (table->last_pos <= pos)
    return -1L;

  /* Place cursor in given pos */
  fseek(table->file, pos, SEEK_SET);

  /* Allocs for the whole record */
  table->buff = (void **)malloc(table->ncols * sizeof(void *));
  if (!table->buff)
    return -1L;

  for (i = 0; i < table->ncols; i++) {
    /* Gets size */
    fread(&size, sizeof(size_t), 1, table->file);

    /* Allocs for column size. Default: malloc returns (void*), uses (void)
     * mult. size */
    table->buff[i] = malloc(size);
    if (!table->buff[i]) {
      for (; i >= 0; i--)
        free(table->buff[i]);
      free(table->buff);
      return -1L;
    }

    fread(table->buff[i], size, 1, table->file);
  }

  table->cur_pos = ftell(table->file);
  return table->cur_pos;
}

/*
  Returns a pointer to the value of the given column of the record
  currently in memory. The value is cast to a void * (it is always a
  pointer: if the column is an INT, the function will return a pointer
  to it).. Returns NULL if there is no record in memory or if the
  column doesn't exist.
*/
void *table_column_get(table_t *table, int col) {
  if (!table || col >= table->ncols || col < 0)
    return NULL;

  if (table->buff)
    return table->buff[col];
  else
    return NULL;
}

/*
   Inserts a record in the last available position of the table. Note
   that all the values are cast to void *, and that there is no
   indication of their actual type or even of how many values we ask
   to store... why?
*/
void table_insert_record(table_t *table, void **values) {
  int i;
  size_t len = 0;

  if (!table || !values)
    return;

  /* Place cursor in the first position we can insert */
  fseek(table->file, table->last_pos, SEEK_SET);

  /* Writes in bin file */
  for (i = 0; i < table->ncols; i++) {
    /* Obtains size, then writes size and value for each one */
    len = value_length(table->types[i], values[i]);
    fwrite(&len, sizeof(size_t), 1, table->file);
    fwrite(values[i], len, 1, table->file);
  }

  /* Update last pos */
  table->last_pos = ftell(table->file);
}
