CC=clang

sudokuSolver: main.o grille.o numeriseur.o solveur.o
	$(CC) -ldl -lpng -o $@ $^

main.o: main.c numeriseur.h grille.h solveur.h
	$(CC) -o $@ -c $<

numeriseur.o : numeriseur.c numeriseur.h asprise_ocr_api.h
	$(CC) -o $@ -c $<

grille.o : grille.c grille.h 
	$(CC) -o $@ -c $<

solveur.o : solveur.c solveur.h

ocr : test.o 
	$(CC) -ldl -o $@ $^

test.o : test.c asprise_ocr_api.h
	$(CC) -o $@ -c $<

clean:
	rm -rf *.o sudokuSolver
