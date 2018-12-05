#include "odbc.h"
#include "table.h"
#include <sql.h>
#include <sqlext.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char const *argv[]) {
  SQLHENV env;
  SQLHDBC dbc;
  SQLHSTMT stmt;
  SQLRETURN ret; /* ODBC API return status */
  SQLBIGINT id;
  char query[512];
  table_t *table = NULL;
  void **rec = NULL;
  /* For this specific case */
  type_t types[] = {LLNG, STR, INT, STR};

  /* CONNECT */
  ret = odbc_connect(&env, &dbc);
  if (!SQL_SUCCEEDED(ret))
    return EXIT_FAILURE;

  /* Less than four arguments on input */
  if (argc < 4) {
    printf("Usa ./score <scrname> <score> \"<comment>\"\n");
    return 1;
  }

  /* Score not between 0 and 100 */
  if (atoi(argv[2]) < 0 || atoi(argv[2]) > 100) {
    printf("Introduce una puntuacion entre 0 y 100");
    return 1;
  }

  /* Get user id, check if user in database */
  SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
  sprintf(query, "SELECT user_id FROM users WHERE screenname='%s'", argv[1]);
  SQLExecDirect(stmt, (SQLCHAR *)query, SQL_NTS);
  SQLBindCol(stmt, 1, SQL_C_SBIGINT, &id, sizeof(id), NULL);
  while (!SQL_SUCCEEDED(ret = SQLFetch(stmt))) {
    printf("\n    El usuario '%s' no existe.\n", argv[1]);
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

  rec = (void **)malloc(table_ncols(table) * sizeof(void *));
  if (!rec) {
    table_close(table);
    fprintf(stderr, "Error in record memory alloc");
    exit(EXIT_FAILURE);
  }

  rec[0] = &id;
  rec[1] = argv[1];
  rec[2] = argv[2];
  rec[3] = argv[3];
  table_insert_record(table, rec);

  /*Free memory*/
  table_close(table);
  free(rec);

  return 0;
}
