#include <stdio.h>
#include <png.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <math.h>
#include <float.h>
#include "grille.h"

#ifndef OCR_SUDOKU
#define OCR_SUDOKU


//load status
#define SUCCESSFULY_LOAD            0
#define FAIL_TO_OPEN_FILE           1
#define FAIL_TO_DEFIND_COLOR_FAMILY 2
#define FAIL_TO_FIND_LINE           3
#define FAIL_TO_FIND_COLUMN         4
#define FAIL_TO_FIND_CELLS          5
#define FAIL_TO_FIND_VALUE          6

int load_sudoku_file(grille * grille_to_load, const char * png_file_name);

void save_pix_map(png_bytepp rows, int height, int width);

typedef struct _cells_detected
{
    int bottom;
    int top;
    int left;
    int right;
} cells_detected;

typedef struct _color_family
{
    int r;
    int g;
    int b;
} color_family;

typedef struct _grille_detected
{
    int columns[GRILLE_DIM + 1];
    int lines[GRILLE_DIM + 1];
    cells_detected cells[GRILLE_DIM + 1][GRILLE_DIM + 1];

} grille_detected;


int find_background_color(png_bytepp rows, int width, int height, int channels);
int find_column(png_bytepp rows, int width, int height, int column_detected[GRILLE_DIM + 1]);
int find_line(png_bytepp rows, int width, int height, int line_detected[GRILLE_DIM + 1]);
int find_cells(png_bytepp rows, int width, int height, int * line_detected, int * column_detected, cells_detected cells_detected[GRILLE_DIM + 1][GRILLE_DIM + 1]);
int find_cells_values(png_bytepp rows, cells_detected cells[GRILLE_DIM + 1][GRILLE_DIM + 1], grille * grille_to_load);


int isWhite(int r, int g, int b);
int isBlack(int r, int g, int b);


/*
    find line
        |
 ____ __|_ ____ ____ ____ ____ ____ ____ ____ 
|    |  | |    |    |    |    |    |    |    |
|    |  | |    |    |    |    |    |    |<------ find column
|____|__v_|____|____|____|____|____|____|____|
|    |    |    |    |    |    |    |    |    |
|    |    |    |    |    |    |    |    |    |
|____|____|____|____|____|____|____|____|____|
|    |    |    |    |    |    |    |    |    |
|    |    |    |    |    |    |    |    |    |
|____|____|____|____|____|____|____|____|____|
|    |    |    |    |    |    |    |    |  ^ |
|    |    |    |    |    |    |    |    |<-|->--- find cells
|____|____|____|____|____|____|____|____|__v_|
|    |    |    |    |    |    |    |    |    |
|    |    |    |    |    |    |    |    |    |
|____|____|____|____|____|____|____|____|____|
|    |    |    |    |    |    |    |    |    |
|    |    |    |    |    |    |    |    |    |
|____|____|____|____|____|____|____|____|____|
|    |    |    |    |    |    |    |    |    |
|    |    |    |    |    |    |    |    |    |
|____|____|____|____|____|____|____|____|____|
|    |    |    |    |    |    |    |    |    |
|    |    |    |    |    |    |    | 5 <-------- find cells_values
|____|____|____|____|____|____|____|____|____|
|    |    |    |    |    |    |    |    |    |
|    |    |    |    |    |    |    |    |    |
|____|____|____|____|____|____|____|____|____|
*/

int filter_number(png_bytepp rows,int x_start, int x_end, int y_start, int y_end, int filter);
int filter_one(png_bytepp rows,int x_start, int x_end, int y_start, int y_end);
int filter_two(png_bytepp rows,int x_start, int x_end, int y_start, int y_end);
int filter_three(png_bytepp rows,int x_start, int x_end, int y_start, int y_end);
int filter_four(png_bytepp rows,int x_start, int x_end, int y_start, int y_end);
int filter_five(png_bytepp rows,int x_start, int x_end, int y_start, int y_end);
int filter_six(png_bytepp rows,int x_start, int x_end, int y_start, int y_end);
int filter_seven(png_bytepp rows,int x_start, int x_end, int y_start, int y_end);
int filter_eight(png_bytepp rows,int x_start, int x_end, int y_start, int y_end);
int filter_nine(png_bytepp rows,int x_start, int x_end, int y_start, int y_end);
int filter_with_pattern(png_bytepp rows,int x_start, int x_end, int y_start, int y_end, int * pattern);


// external OCR :

//void determine_cells_value_external_ocr(const char *png_file_name, int grille_result[GRILLE_DIM][GRILLE_DIM]);


#endif