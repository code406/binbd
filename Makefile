CC = gcc -ansi -pedantic
CFLAGS = -Wall -std=c99 -Wextra
LDLIBS = -lodbc
EXE = test test_table table_test score

all : $(EXE)

clean :
	rm -f *.o *.txt *.dat core $(EXE)

$(EXE) : % : %.o type.o table.o odbc.o
	@echo "#---------------------------"
	@echo "# Generando $@ "
	@echo "# Depende de $^"
	@echo "# Ha cambiado $<"
	$(CC) $(CFLAGS) -o $@ $@.o type.o table.o odbc.o $(LDLIBS)

type.o : type.c type.h
	@echo "#---------------------------"
	@echo "# Generando $@"
	@echo "# Depende de $^"
	@echo "# Ha cambiado $<"
	$(CC) $(CFLAGS) -c $<

table.o : table.c table.h
	@echo "#---------------------------"
	@echo "# Generando $@"
	@echo "# Depende de $^"
	@echo "# Ha cambiado $<"
	$(CC) $(CFLAGS) -c $<

odbc.o : odbc.c odbc.h
	@echo "#---------------------------"
	@echo "# Generando $@"
	@echo "# Depende de $^"
	@echo "# Ha cambiado $<"
	$(CC) $(CFLAGS) -c $< $(LDLIBS)

runvtest:
	valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all --track-origins=yes ./test 20

runvtest_table:
	valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all --track-origins=yes ./test_table

runvtable_test:
	valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all --track-origins=yes ./table_test

runvscore:
	valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all --track-origins=yes ./score furilo 76 Parecequefunciona
