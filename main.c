#include <stdio.h>
#include "ocr_sudoku.h"
#include "solver.h"

int main(int argc, char * argv[])
{

    if(argc <= 1)
    {
        printf("usage ./solver <file_png_name>");
        return 1;
    }

    grille * g = create_grille();

    char * name_file = argv[1];

    int loading_state = load_sudoku_file(g,name_file);


    if(loading_state == SUCCESSFULY_LOAD)
    {
        solve(g);
    }
    else
    {
        printf("Fail to load, status = %d\n", loading_state);    
    }

    delete_grille(g);

    return 0;
}