CC=gcc
CFLAGS= -g -Wall -pedantic

all: table_test

table_test: table_test.o table.o type.o
	gcc $(CFLAGS) -o table_test table_test.o table.o type.o

table_test.o: table_test.c table.h
	gcc $(CFLAGS) -c table_test.c
	
table.o: table.c table.h type.h
	gcc $(CFLAGS) -c table.c

type.o: type.c type.h
	gcc $(CFLAGS) -c type.c
	
clean:
	rm -f *.o table_test
