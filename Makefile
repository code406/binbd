CC = gcc -ansi -pedantic
CFLAGS = -Wall -std=c99 -Wextra
LDLIBS = -lodbc
EXE = test test_table table_test score suggest suggest_index suggest_alt

all : $(EXE)

clean :
	rm -f *.o *.txt *.dat core $(EXE)

$(EXE) : % : %.o type.o table.o index.o odbc.o
	$(CC) $(CFLAGS) -o $@ $@.o type.o table.o index.o odbc.o $(LDLIBS)

type.o : type.c type.h
	$(CC) $(CFLAGS) -c $<

table.o : table.c table.h
	$(CC) $(CFLAGS) -c $<

index.o : index.c index.h
	$(CC) $(CFLAGS) -c $<

odbc.o : odbc.c odbc.h
	$(CC) $(CFLAGS) -c $< $(LDLIBS)

runvtable:
	valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all --track-origins=yes ./test 20

	valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all --track-origins=yes ./test_table

	valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all --track-origins=yes ./table_test

runvscore:
	valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all --track-origins=yes ./score Buenafuente 50 \"es un senior mayor\"
