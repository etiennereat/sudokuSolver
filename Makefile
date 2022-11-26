CC=clang

sudokuSolver: main.o grille.o
	$(CC) -ldl -lpng -o $@ $^

main.o: main.c grille.h
	$(CC) -o $@ -c $<

grille.o : grille.c grille.h asprise_ocr_api.h
	$(CC) -o $@ -c $<

ocr : test.o 
	$(CC) -ldl -o $@ $^

test.o : test.c asprise_ocr_api.h
	$(CC) -o $@ -c $<

clean:
	rm -rf *.o sudokuSolver
