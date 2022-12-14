#ifndef GRILLE_H
#define GRILLE_H

#include <png.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

typedef struct grille_{
    int * grille;
}grille;


grille * read_png_to_grille (const char * png_file);

void set_in_grille(grille * grilleREF, int x, int y, int value);
int get_in_grille(grille * grilleREF, int x, int y);
void print_grille(grille * grilleREF);

#endif