#include <sql.h>
#include <sqlext.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "odbc.h"
#include "table.h"

int main(int argc, char const *argv[]) {
  SQLHENV env;
  SQLHDBC dbc;
  SQLHSTMT stmt;
  SQLRETURN ret; /* ODBC API return status */
  char query[512];
	long number;
	char *text = NULL;
	char *text2 = NULL;
  table_t *table = NULL;
	long pos = 0;
	int *score = NULL;
	char *name = NULL;
	char *comment = NULL;

  /* CONNECT */
  ret = odbc_connect(&env, &dbc);
  if (!SQL_SUCCEEDED(ret))
    return EXIT_FAILURE;

  /* Less than two arguments on input */
  if (argc < 2) {
    printf("Usa ./suggest <score> \n");
    return 1;
  }
  /* argv[1]=screenname. argv[2]= score */

	/* Abre la tabla score (tiene que haber sido escrita antes)*/
	table = table_open("score.dat");
	if (!table) {
		printf("Error opening table\n");
		return 1;
	}

	/* Lee tabla desde el primer registro, y va guardando en las var. locales */
	pos = table_first_pos(table);
	printf("1\n");
	while(pos < table_last_pos(table)){
		printf("2\n");
		/* Lee registro para almacenarlo y actualiza posicion. Luego debo liberar */
		pos = table_read_record(table, pos);
		/* Obtiene el score del registro */
		score = (int *)table_column_get(table, 2);
		if (!score){
			printf("Cannot read score from record\n");
			return 1;
		}
		printf("3\n");
		/* Si score > argv[2](score introducido), obtiene todo lo demas */
		if (*score > atoi(argv[2]))
			name = (char *)table_column_get(table, 1);
			if(!name){
				printf("Cannot read name from record\n");
				free(score);
				return 1;
			}
			comment = (char *)table_column_get(table, 3);
			if(!comment){
				printf("Cannot read comment from record\n");
				free(score);
				free(name);
				return 1;
			}
			printf("4\n");
			/************* Obtiene tweets y numero de tweets (consulta) *************/
			SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
			sprintf(query, "SELECT COUNT(userwriter) AS numberoftweets FROM tweets WHERE userwriter IN (SELECT user_id FROM users WHERE screenname LIKE '%s')", name);
			SQLExecDirect(stmt, (SQLCHAR*) query, SQL_NTS);
			SQLBindCol(stmt, 1, SQL_C_SBIGINT, &number, sizeof(number), NULL);
			while (SQL_SUCCEEDED(ret = SQLFetch(stmt)));
			SQLCloseCursor(stmt);
			sprintf(query, "SELECT tweet_id, TO_CHAR(tweettimestamp, 'YYYY-MM-DD') tweettimestamp, tweettext FROM tweets WHERE userwriter IN (SELECT user_id FROM users WHERE screenname LIKE '%s') ORDER BY tweettimestamp DESC", name);
			SQLExecDirect(stmt, (SQLCHAR*) query, SQL_NTS);/*PONER UN LIMIT A LOS TWEETS QUE VAN A APARECER*/
			SQLBindCol(stmt, 1, SQL_C_SBIGINT, &number, sizeof(number), NULL);
			SQLBindCol(stmt, 2, SQL_C_CHAR, text, sizeof(text), NULL);
			SQLBindCol(stmt, 3, SQL_C_CHAR, text2, sizeof(text2), NULL);
			printf("\n\nLISTA DE TWEETS (TOTAL: %ld) \n", number);
			while (SQL_SUCCEEDED(ret = SQLFetch(stmt))) printf("    %ld | %s | %s\n", number, text, text2);
			SQLCloseCursor(stmt);
			SQLFreeHandle(SQL_HANDLE_STMT, stmt);
			/************************************************************************/
			printf("5\n");
			free(score);
			free(name);
			free(comment);
	}

  table_close(table);
  return 0;
}
