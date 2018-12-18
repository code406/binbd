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
    printf("Usa ./suggest <score> \n");
    return 1;
  }
  /* argv[1]= score */

	/* Abre la tabla score (tiene que haber sido escrita antes)*/
	table = table_open("score.dat");
	if (!table) {
		printf("Error opening table\n");
		return 1;
	}

	/* Create and open index */
  index_create(0);
  index = index_open("index.dat");
  if (!index) {
    printf("Error opening index\n");
    return 1;
  }

	score_input = atoi(argv[1]);

	printf("\n");
	/* Lee tabla desde el primer registro, y va guardando en las var. locales */
	pos = table_first_pos(table);
	while(pos < table_last_pos(table)){
		pos_inicio = pos;
		/* Lee registro para almacenarlo y actualiza posicion. Luego debo liberar */
		pos = table_read_record(table, pos);
		/* Obtiene el score del registro */
		score = (int *)table_column_get(table, 2);
		if (!score){ /*LIBERAR*/
			printf("Cannot read score from record\n");
			return 1;
		}
		/* Introduce en indice con ese score y posicion */
		index_put(index, *score, pos_inicio);
	}

	printf("\n=============================================\n\n");
	/* index_get para pillar el del score que nos pasan. y luego usare todos los siguientes a ese */
	for (i=score_input, n=0; i<100; i++){
		/* pillo el numerito del registro que define esa clave. se haria con la funcion1*/
		index_get(index, i, &n);
		if (n>0){
			/* guardo la clave en una variable */
			limit = i;
			printf("\nEL BUCLE PARA ENCONTRAR LA CLAVE HA ENCONTRADO: %d\n", i);
			break;
		}
	}

/* funcion1: me busca la clave (limit) y me da el numero de index que es. 1, 2, 3 */
/* funcion2: como get pero dado un "1", "2", o "3" */
/* for(i del registro, 0 (porque somos de rankin), i--) funcion2: que es como get pero dado el i del registro */

	index_print(index);
	/* index_get(index, score_input, nposs) */
	printf("\n=============================================\n\n");
	for(i=100; i>=limit; i--){
		/* Cojo las posiciones en las que esta la clave 'i'. n es cuantas son */
		poss = index_get(index, i, &n);
		for (j=0; j<n; j++) {
			/* Lee registro para almacenarlo. Luego debo liberar */
			table_read_record(table, (*poss)[j]);
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
			printf("\n%s %d\n%s\n%ld\n", name, i, comment, number);
			while (SQL_SUCCEEDED(ret = SQLFetch(stmt))) printf("%s\n", text);
			printf("\n");
			SQLCloseCursor(stmt);
			SQLFreeHandle(SQL_HANDLE_STMT, stmt);
			/************************************************************************/
			free(name);
			free(comment);
		}
	}

	/* Guardo el indice en fichero */
	index_save(index, "estonovaleparanada.dat");

  table_close(table);
	index_close(index);
  return 0;
}
