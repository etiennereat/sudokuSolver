CC=clang

sudokuSolver: main.o numeriseur.o
	$(CC) -ldl -lpng -o $@ $^

main.o: main.c numeriseur.h
	$(CC) -o $@ -c $<

numeriseur.o : numeriseur.c grille.h asprise_ocr_api.h
	$(CC) -o $@ -c $<

ocr : test.o 
	$(CC) -ldl -o $@ $^

test.o : test.c asprise_ocr_api.h
	$(CC) -o $@ -c $<

clean:
	rm -rf *.o sudokuSolver
