/* AN ALTERNATE VERSION FOR SUGGEST_INDEX.C         */
/* USES A SPLIT "GET" FUNCTION TO BE MORE EFFICIENT */

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
  table_t *table = NULL;
	index_t *index = NULL;
	long pos = 0, pos_inicio = 0;
	int *score = NULL;
	char *name = NULL;
	char *comment = NULL;
	int score_input;
	int n = 0, i = 0, j = 0, limit = 0;
	long **poss = NULL;

  /* CONNECT */
  ret = odbc_connect(&env, &dbc);
  if (!SQL_SUCCEEDED(ret))
    return EXIT_FAILURE;

  /* Less than two arguments on input */
  if (argc < 2) {
    printf("Usa ./suggest_alt <score> \n");
    return 1;
  }

	/* Opens the score table (file has to be already written) */
	table = table_open("score.dat");
	if (!table) {
		printf("Error opening table\n");
		return 1;
	}

	/* Create and open index. Uses index_create(0) as we only use integers */
  index_create(0);
  index = index_open("index.dat");
  if (!index) {
    printf("Error opening index\n");
		table_close(table);
    return 1;
  }

	score_input = atoi(argv[1]);

	/* Reads all records, and inserts them in the index */
  pos = table_first_pos(table);
  while (pos < table_last_pos(table)) {
		/* Saves the position where the record starts, needed for index_put */
		pos_inicio = pos;
    /* Reads record to obtain its score, and updates the position */
    pos = table_read_record(table, pos);
    score = (int *)table_column_get(table, 2);
    if (!score) {
      printf("Cannot read score from record\n");
			table_close(table);
			index_close(index);
      return 1;
    }
    /* Inserts in index */
    index_put(index, *score, pos_inicio);
		free(score);
  }

	/* Obtains the next score greater than score_input. Will act as a limit */
	for (i = score_input, n=0; i < 100 && n == 0; i++){
		/* Gets the position in the index of the record that contains that score */
		limit = index_get_f1(index, i, &n);
	}

	/* Gets information of all records in the index that are before the limit */
	for(i = 0; i <= limit; i++){
		/* Gets positions from the index record with index position "i". */    /* Gets positions the index record with score "i". */
		/* "n" is the number of positions that we obtain */
		poss = index_get_f2(index, i, &n);
		/* For each position, we get the record information */
		for (j = 0; j < n; j++) {
			table_read_record(table, (*poss)[j]);
			score = (int *)table_column_get(table, 2);
			if (!score){
				printf("Cannot read score from record\n");
				table_close(table);
				index_close(index);
				return 1;
			}
			name = (char *)table_column_get(table, 1);
			if(!name){
				printf("Cannot read name from record\n");
				free(score);
				table_close(table);
				index_close(index);
				return 1;
			}
			comment = (char *)table_column_get(table, 3);
			if(!comment){
				printf("Cannot read comment from record\n");
				free(score);
				free(name);
				table_close(table);
				index_close(index);
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
			printf("\n%s %d\n%s\n%ld\n", name, *score, comment, number);
			while (SQL_SUCCEEDED(ret = SQLFetch(stmt))) printf("%s\n", text);
			printf("\n");
			SQLCloseCursor(stmt);
			SQLFreeHandle(SQL_HANDLE_STMT, stmt);
			/************************************************************************/
			free(score);
			free(name);
			free(comment);
		}
	}

	/* Saves index in file */
	index_save(index, "index.dat");

	/* Free memory */
  table_close(table);
	index_close(index);

  /* DISCONNECT */
  ret = odbc_disconnect(env, dbc);
  if (!SQL_SUCCEEDED(ret)) {
    return EXIT_FAILURE;
  }
  /* DISCONNECT */
  return 0;
}
