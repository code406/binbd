#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sql.h>
#include <sqlext.h>
#include "odbc.h"
#include "table.h"

int main(int argc, char const *argv[]) {
  SQLHENV env;
  SQLHDBC dbc;
  SQLHSTMT stmt;
  SQLRETURN ret; /* ODBC API return status */
  SQLBIGINT id;
  char query[512];
  table_t *table = NULL;
  void **rec = NULL;
	int score;
  /* Fills types of the columns for this table */
  type_t types[] = {LLNG, STR, INT, STR};

  /* CONNECT */
  ret = odbc_connect(&env, &dbc);
  if (!SQL_SUCCEEDED(ret))
    return EXIT_FAILURE;

  /* Less than four arguments on input */
  if (argc < 4) {
    printf("Use ./score <scrname> <score> \"<comment>\"\n");
    return 1;
  }

  /* Score not between 0 and 100 */
	score = atoi(argv[2]);
  if (score < 0 || score > 100) {
    printf("Score must be between 0 and 100\n");
    return 1;
  }

  /* Get user id, check if user in database */
  SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
  sprintf(query, "SELECT user_id FROM users WHERE screenname='%s'", argv[1]);
  SQLExecDirect(stmt, (SQLCHAR *)query, SQL_NTS);
  SQLBindCol(stmt, 1, SQL_C_SBIGINT, &id, sizeof(id), NULL);
  while (!SQL_SUCCEEDED(ret = SQLFetch(stmt))) {
    printf("\n    User '%s' does not exist.\n", argv[1]);
    return 1;
  }
  SQLCloseCursor(stmt);

  /* Create and open table */
  table_create("score.dat", 4, types);
  table = table_open("score.dat");
  if (!table) {
    printf("Error opening table\n");
    return 1;
  }

	/* Alloc memory for the record we want to insert */
  rec = (void **)calloc(table_ncols(table), sizeof(void *));
  if (!rec) {
    table_close(table);
    return 1;
  }

	/* Fill the record with the values we have obtained. */
  rec[0] = &id;
	rec[1] = calloc(1, (strlen(argv[1])+1) * sizeof(char));
  strcpy(rec[1], argv[1]);
	rec[2] = &score;
	rec[3] = calloc(1, (strlen(argv[3])+1) * sizeof(char));
	strcpy(rec[3], argv[3]);

	/* Insert the record in the table */
  table_insert_record(table, rec);

  /* Free memory */
  table_close(table);
	free(rec[1]);
	free(rec[3]);
	free(rec);

  /* DISCONNECT */
  ret = odbc_disconnect(env, dbc);
  if (!SQL_SUCCEEDED(ret)) {
    return EXIT_FAILURE;
  }
  /* DISCONNECT */
  return 0;
}
