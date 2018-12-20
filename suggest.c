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
  SQLCHAR text[512];
  SQLBIGINT number;
	char query[512];
  table_t *table = NULL;
	long pos = 0;
	int *score = NULL;
	char *name = NULL;
	char *comment = NULL;
	int score_input;

  /* CONNECT */
  ret = odbc_connect(&env, &dbc);
  if (!SQL_SUCCEEDED(ret))
    return EXIT_FAILURE;

  /* Less than two arguments on input */
  if (argc < 2) {
    printf("Usa ./suggest <score> \n");
    return 1;
  }

	/* Opens the score table (file has to be already written) */
	table = table_open("score.dat");
	if (!table) {
		printf("Error opening table\n");
		return 1;
	}

	score_input = atoi(argv[1]);

	/* Reads all records, from first_pos (1st record) to last_pos (can't be any)*/
	pos = table_first_pos(table);
	while(pos < table_last_pos(table)){
		/* Reads record and updates the position */
		pos = table_read_record(table, pos);
		/* Obtains the score from the record */
		score = (int *)table_column_get(table, 2);
		if (!score){
			printf("Cannot read score from record\n");
			table_close(table);
			return 1;
		}
		/* If score > score_input, we obtain the rest of the information */
		if ((*score) > score_input){
			name = (char *)table_column_get(table, 1);
			if(!name){
				printf("Cannot read name from record\n");
				free(score);
				table_close(table);
				return 1;
			}
			comment = (char *)table_column_get(table, 3);
			if(!comment){
				printf("Cannot read comment from record\n");
				free(score);
				free(name);
				table_close(table);
				return 1;
			}
			/************* Obtains tweets & number of tweets (query) ****************/
			SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
			sprintf(query, "SELECT COUNT(userwriter) AS numberoftweets FROM tweets WHERE userwriter IN (SELECT user_id FROM users WHERE screenname LIKE '%s')", name);
			SQLExecDirect(stmt, (SQLCHAR*) query, SQL_NTS);
			SQLBindCol(stmt, 1, SQL_C_SBIGINT, &number, sizeof(number), NULL);
			while (SQL_SUCCEEDED(ret = SQLFetch(stmt)));
			SQLCloseCursor(stmt);
			sprintf(query, "SELECT tweettext FROM tweets WHERE userwriter IN (SELECT user_id FROM users WHERE screenname LIKE '%s') LIMIT 5", name);
			SQLExecDirect(stmt, (SQLCHAR*) query, SQL_NTS);
			SQLBindCol(stmt, 1, SQL_C_CHAR, text, sizeof(text), NULL);
			printf("%s %d\n%s\n%ld\n", name, *score, comment, number);
			while (SQL_SUCCEEDED(ret = SQLFetch(stmt))) printf("%s\n", text);
			printf("\n\n");
			SQLCloseCursor(stmt);
			SQLFreeHandle(SQL_HANDLE_STMT, stmt);
			/************************************************************************/
			free(score);
			free(name);
			free(comment);
		}
	}

  table_close(table);
  return 0;
}
