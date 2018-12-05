#include <stdio.h>
#include "table.h"

#define STORAGE "dummy_table.dat"

int main(void) {
  int ncols = 3;
  type_t types[] = {INT, STR, INT};
  printf("Trying to create a table with %d columns...\n", ncols);

  table_create(STORAGE, ncols, types);

  // if we reach this point the table was created successfully,
  // as we abort() on Error
  printf(" -> SUCCESS: table created successfully!\n");

  printf("Attempting to open the table we just created...\n");
  table_t *table = table_open(STORAGE);
  if (!table) {
    printf("Error openning the table we just created at \"%s\"!\n", STORAGE);
    abort();
  }
  printf(" -> SUCCESS: table opened successfully!\n");

  printf("Checking the table has the correct number of columns...\n");
  if (ncols != table_ncols(table)) {
    printf(" -> ERROR: table does not have the correct number of columns!\n");
    abort(); // TODO: comment why we're not freeing everything here
  }
  printf(" -> SUCCESS: Table has the correct number of columns!\n");

  printf("Checking if the table has the correct types of columns...\n");
  type_t *types_test = table_types(table);
  if (!types_test) {
    printf(" -> ERROR: table did not return anything when asked to provide column types!\n");
    abort();
  }

  for (int i = 0; i < ncols; i++)
  {
    if (types[i] != types_test[i]) {
      printf(" -> ERROR: table hasn't got the correct type for column no. %d!\n", i);
      abort();
    }
  }
  printf(" -> SUCCESS: table has the correct types for every column!\n");

  printf("Checking if table returns the first position correctly...\n");
  long first_pos = (long) (sizeof(ncols) + sizeof(type_t) * ncols);
  if (table_first_pos(table) != first_pos) {
    printf(" -> ERROR: table did not determine the start of the data correctly!");
    abort();
  }
  printf(" -> SUCCESS: correctly returned the first data position in the table!\n");

  printf("Checking if the table returns the current position correctly...\n");
  if (table_cur_pos(table) != first_pos) {
    printf(" -> ERROR: table did not return the first position but there is not any data on the file\n");
    printf(" -> Should have returned %ld but returned %ld instead.\n", first_pos, table_cur_pos(table));
    abort();
  }
  printf(" -> SUCCESS: table correctly returned EOF as the current position!\n");

  printf("Checking if the table returns EOF as the last position of the file...\n");
  if (table_last_pos(table) != EOF) {
    printf(" -> ERROR: did not return EOF as the last position, returned %ld instead\n", table_last_pos(table));
  }
  printf(" -> SUCCESS: correctly returned EOF as the last position\n");


  printf("Trying to insert a record into the table...\n");

  // create dummy data
  int a = 1, c = 3;
  char *b = "Marabini for president! Yes we can, guys!";
  void *record[] = {&a, b, &c};
  printf(" -> Inserting values (%d, \"%s\", %d)\n", a, b, c);

  table_insert_record(table, record);

  printf(" -> WELL? we asume everything worked as we have no way of checking for insertion\n");

  printf("Closing table...\n");

  table_close(table);

  printf("Openning table again for reading...\n");

  table = table_open(STORAGE);
  if (!table) {
    fprintf(stderr, "Could not reopen table for reading\n");
    abort();
  }

  printf("Trying to retrieve the record we just saved from the database...\n");
  table_read_record(table, table_first_pos(table));

  printf(" -> Retrieved values (%d, \"%s\", %d)\n",
    *(int *)table_column_get(table, 0),
    (char *)table_column_get(table, 1),
    *(int *)table_column_get(table, 2));

  table_close(table);

  return 0;
}
