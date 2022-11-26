#include "grille.h"

int main (int argc, char ** argv)
{
    char * name_file = "grille_test.png";
    if(argc > 1){
        name_file = argv[1];
    }
    read_png_to_grille(name_file);
    return 0;
}