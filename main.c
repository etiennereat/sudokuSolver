#include "grille.h"
#include "numeriseur.h"
#include "solveur.h"

int main (int argc, char ** argv)
{
    char * name_file = "grille_test.png";
    if(argc > 1){
        name_file = argv[1];
    }
    grille * grilleREF = read_png_to_grille(name_file);

    print_grille(grilleREF);

    free_grille(grilleREF);
    return 0;
}