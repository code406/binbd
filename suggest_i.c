#include <sql.h>
#include <sqlext.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "odbc.h"
#include "table.h"
#include "index.h"

int main(int argc, char const *argv[]) {
  SQLHENV env;
  SQLHDBC dbc;
  SQLHSTMT stmt;
  SQLRETURN ret; /* ODBC API return status */
  SQLCHAR text[512];
  SQLBIGINT number;
	char query[512];
  index_t *index = NULL;
  table_t *table = NULL;
	long pos = 0;
	int *score = NULL;
	char *name = NULL;
	char *comment = NULL;
	int score_input;
	int n = 0, i = 0, j;
	long **poss = NULL;

  /* CONNECT */
  ret = odbc_connect(&env, &dbc);
  if (!SQL_SUCCEEDED(ret))
    return EXIT_FAILURE;

  /* Less than two arguments on input */
  if (argc < 2){
    printf("Usa ./suggest <score> \n");
    return 1;
  }
  /* argv[1]= score */

	/* Abre la tabla (archivo se ha escrito antes)*/
  table = table_open("score.dat");
  if( !table){
    printf("Error opening table\n");
		return 1;
  }
	/*Tenemos una tabla desordenada*/

	/* Abro index (archivo se ha escrito antes) */
	index = index_open("index.dat");
	if (!index) {
		printf("Error opening index\n");
		return 1;
	}

	score_input = atoi(argv[1]);

	/* Recorro index (desc). Como no puedo acceder a irecord, voy desde 100 */
	for(i=100; i>score_input; i--){
		poss = index_get(index, i, &n);
		for (int j=0; j<n; j++) {
			/* Lee registro para almacenarlo y actualiza posicion. Luego debo liberar */
			/* REVISAR ESTE PASO */
			*poss[j] = table_read_record(table, *poss[j]);
			/* obtengo lo demas. tengo ya score que es "i" */
			name = (char *)table_column_get(table, 1);
			if(!name){
				printf("Cannot read name from record\n");
				return 1;
			}
			comment = (char *)table_column_get(table, 3);
			if(!comment){
				printf("Cannot read comment from record\n");
				free(name);
				return 1;
			}
			/************* Obtiene tweets y numero de tweets (consulta) *************/
			SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
			sprintf(query, "SELECT COUNT(userwriter) AS numberoftweets FROM tweets WHERE userwriter IN (SELECT user_id FROM users WHERE screenname LIKE '%s')", name);
			SQLExecDirect(stmt, (SQLCHAR*) query, SQL_NTS);
			SQLBindCol(stmt, 1, SQL_C_SBIGINT, &number, sizeof(number), NULL);
			while (SQL_SUCCEEDED(ret = SQLFetch(stmt)));
			SQLCloseCursor(stmt);
			sprintf(query, "SELECT tweettext FROM tweets WHERE userwriter IN (SELECT user_id FROM users WHERE screenname LIKE '%s') LIMIT 5", name);
			SQLExecDirect(stmt, (SQLCHAR*) query, SQL_NTS);
			SQLBindCol(stmt, 1, SQL_C_CHAR, text, sizeof(text), NULL);
			printf("%s %d\n%s\n%ld\n", name, i, comment, number);
			while (SQL_SUCCEEDED(ret = SQLFetch(stmt))) printf("%s\n", text);
			printf("\n\n");
			SQLCloseCursor(stmt);
			SQLFreeHandle(SQL_HANDLE_STMT, stmt);
			/************************************************************************/
			free(name);
			free(comment);
		}
	}

  table_close(table);
  return 0;
}
