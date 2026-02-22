
CC=clang

LDFLAGS = -ldl -lpng -lm -std=c99

solver: main.o solver.o grille.o ocr_sudoku.o pileSupp.o
	$(CC) -o $@ $^ $(LDFLAGS)

main.o: main.c ocr_sudoku.h grille.h
	$(CC) -o $@ -c $<

ocr_sudoku.o: ocr_sudoku.c ocr_sudoku.h grille.h asprise_ocr_api.h
	$(CC) -o $@ -c $<

solver.o: solver.c solver.h grille.h pileSupp.h
	$(CC) -o $@ -c $<

grille.o: grille.c grille.h
	$(CC) -o $@ -c $<

pileSupp.o: pileSupp.c pileSupp.h grille.h
	$(CC) -o $@ -c $<

clean:
	rm -rf *.o solver

