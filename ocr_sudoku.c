#include "ocr_sudoku.h"

#define FG_DIST_THRESHOLD 255 / 8
#define LUM_THRESHOLD 15.0
#define NB_CHANELS_PNG 4
color_family background_color;

int load_sudoku_file(grille * grille_to_load, const char * png_file_name)
{
    //png variables managers
    png_structp	png_ptr;
    png_infop info_ptr;
    FILE * fp;
    png_uint_32 width;
    png_uint_32 height;
    int bit_depth;
    int color_type;
    int interlace_method;
    int compression_method;
    int filter_method;
    png_bytepp rows;
    int rowbytes;

    //try to open png 
    fp = fopen (png_file_name, "rb");
    if (! fp) {
	    printf("Cannot open '%s': %s\n", png_file_name, strerror (errno));
        return FAIL_TO_OPEN_FILE;
    }
    png_ptr = png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (! png_ptr) {
	    printf ("Cannot create PNG read structure");
        return FAIL_TO_OPEN_FILE;
    }
    info_ptr = png_create_info_struct (png_ptr);
    if (! png_ptr) {
	    printf ("Cannot create PNG info structure");
        return FAIL_TO_OPEN_FILE;
    }

    //hook data from png 
    png_init_io (png_ptr, fp);
    png_read_png (png_ptr, info_ptr, 0, 0);
    png_get_IHDR (png_ptr, info_ptr, & width, & height, & bit_depth,
		  & color_type, & interlace_method, & compression_method,
		  & filter_method);
    rows = png_get_rows (png_ptr, info_ptr);
    rowbytes = png_get_rowbytes (png_ptr, info_ptr);

    //display data
    printf ("Width is %d, height is %d\n", width, height);
    printf ("Row bytes = %d\n", rowbytes);

    grille_detected grille_png;


    //find background color
    if (find_background_color(rows, width, height, NB_CHANELS_PNG) != 1)
    {
        return FAIL_TO_DEFIND_COLOR_FAMILY;
    }

    save_pix_map(rows,width, height);

    //find line
    int nb_line_detected = find_line(rows,width,height, grille_png.lines);
    if(nb_line_detected < GRILLE_DIM + 1)
    {
        return FAIL_TO_FIND_LINE;
    }

    //find column
    int nb_column_detected = find_column(rows,width,height, grille_png.columns);
    if(nb_column_detected < GRILLE_DIM + 1)
    {
        return FAIL_TO_FIND_COLUMN;
    }

    //find cell based on line and column previously detected
    int nb_cell_detected = find_cells(rows,width, height,grille_png.lines,grille_png.columns,grille_png.cells);
    if(nb_cell_detected < GRILLE_DIM * GRILLE_DIM)
    {
        return FAIL_TO_FIND_CELLS;
    }

    int nb_cells_value_detected = find_cells_values(rows,grille_png.cells, grille_to_load);
    if(nb_cells_value_detected < GRILLE_DIM * GRILLE_DIM)
    {
        return FAIL_TO_FIND_VALUE;
    }    

    return SUCCESSFULY_LOAD;
}

void save_pix_map(png_bytepp rows, int width, int height)
{
    FILE * fp = fopen ("test.txt", "w");

    for (int i = 0; i < height; i++) 
    {
        for (int j = 0; j < width; j++) 
        {
            png_byte red      = rows[i][j*NB_CHANELS_PNG];
            png_byte green    = rows[i][j*NB_CHANELS_PNG + 1];
            png_byte blue     = rows[i][j*NB_CHANELS_PNG + 2];

            
            if(isBlack(red,green,blue))
            {
                fprintf(fp,"%c", 'X');
            }
            else if(isWhite(red,green,blue))
            {
                fprintf(fp,"%c", ' ');
            }
            else
            {
                fprintf(fp,"%c", '?');
            }
        }
        fprintf(fp,"\n");
    }

    fclose(fp);
}


void show_sub_png(png_bytepp rows,int x_start, int x_end, int y_start, int y_end)
{
    for (int i = x_start; i < x_end; i++) 
    {
        for (int j = y_start; j < y_end; j++) 
        {
            png_byte red      = rows[i][j*NB_CHANELS_PNG];
            png_byte green    = rows[i][j*NB_CHANELS_PNG + 1];
            png_byte blue     = rows[i][j*NB_CHANELS_PNG + 2];

            if(isBlack(red,green,blue))
            {
                printf("%c", 'X');
            }
            else if(isWhite(red,green,blue))
            {
                printf("%c", ' ');
            }
            else
            {
                printf("%c", '?');
            }
        }
        printf("\n");
    }
}



/**
 * Calcule la distance euclidienne entre deux couleurs RGB
 */
static double color_distance(int r1, int g1, int b1, int r2, int g2, int b2)
{
    int dr = r1 - r2;
    int dg = g1 - g2;
    int db = b1 - b2;
    return sqrt(dr * dr + dg * dg + db * db);
}


/**
 * Vérifie si un pixel fait partie de l'arrière-plan (fond)
 * C'est la couleur MAJORITAIRE
 */
int isWhite(int r, int g, int b)
{
    int dist = color_distance(
        r, g, b,
        background_color.r,
        background_color.g,
        background_color.b
    );

    return (dist < FG_DIST_THRESHOLD);
}

/**
 * Vérifie si un pixel fait partie du premier plan (formes/chiffres)
 */
int isBlack(int r, int g, int b)
{
    double bg_luminosity = 0.299 * background_color.r
                     + 0.587 * background_color.g
                     + 0.114 * background_color.b;

    int dist = color_distance(
        r, g, b,
        0,
        0,
        0
    );

    double lum = 0.299 * r + 0.587 * g + 0.114 * b;
    double lum_diff = fabs(lum - bg_luminosity);

    return (dist < FG_DIST_THRESHOLD * 3) || (lum_diff > LUM_THRESHOLD);
}

/**
 * Détecte automatiquement la couleur de fond
 */
int find_background_color(png_bytepp rows, int width, int height, int channels)
{
    if (!rows || width <= 0 || height <= 0 || channels < 3)
        return 0;

    int histogram[16][16][16] = {0};

    /* ============================
     * histogramme couleurs
     * ============================ */
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            png_byte *px = &rows[i][j * channels];

            int r = px[0] / 16;
            int g = px[1] / 16;
            int b = px[2] / 16;

            histogram[r][g][b]++;
        }
    }

    /* ============================
     * BACKGROUND = couleur dominante
     * ============================ */
    int maxCount = 0;
    int br = 0, bg = 0, bb = 0;

    for (int r = 0; r < 16; r++)
    {
        for (int g = 0; g < 16; g++)
        {
            for (int b = 0; b < 16; b++)
            {
                if (histogram[r][g][b] > maxCount)
                {
                    maxCount = histogram[r][g][b];
                    br = r;
                    bg = g;
                    bb = b;
                }
            }
        }
    }

    background_color = (color_family){
        .r = br * 16,
        .g = bg * 16,
        .b = bb * 16    
    };

    printf("BACKGROUND détecté : RGB(%d,%d,%d) | pixels=%d (%.2f%%)\n",
           background_color.r,
           background_color.g,
           background_color.b,
           maxCount,
           100.0f * maxCount / (width * height));

    return (100.0f * maxCount / (width * height)) > 50;
}



int find_cells_values(png_bytepp rows, cells_detected cells[GRILLE_DIM + 1][GRILLE_DIM + 1], grille * grille_to_load)
{
    int nb_cells_value_detected = 0;

    int grille_result[GRILLE_DIM][GRILLE_DIM];

    //find cells
    for(int i=0; i< GRILLE_DIM; i++)
    {
        for(int j=0; j< GRILLE_DIM; j++)
        {
            cells_detected current_cell = cells[i][j];

            int nb_black_pixel = 0;

            int min_x = current_cell.bottom;
            int max_x = current_cell.top;
            int min_y = current_cell.right;
            int max_y = current_cell.left;

//            show_sub_png(rows,current_cell.top,current_cell.bottom,current_cell.left,current_cell.right);


            //crope la cellule pour avoir le plus petit rectangle contenant le nombre
            for(int x = current_cell.top ; x < current_cell.bottom ; x++)
            {
                for(int y = current_cell.left; y < current_cell.right; y++)
                {
                    png_byte red      = rows[x][y*NB_CHANELS_PNG];
                    png_byte green    = rows[x][y*NB_CHANELS_PNG + 1];
                    png_byte blue     = rows[x][y*NB_CHANELS_PNG + 2];
                    
                    if(isBlack(red,green,blue))
                    {
                        nb_black_pixel++;

                        if(x > max_x)
                        {
                            max_x = x;
                        }
                        if(x < min_x)
                        {
                            min_x = x;
                        }

                        if(y > max_y)
                        {
                            max_y = y;
                        }
                        if( y < min_y)
                        {
                            min_y = y;
                        }
                    }
                }
            }

            if( nb_black_pixel == 0)
            {
                nb_cells_value_detected++;
                continue;
            }

//            show_sub_png(rows,min_x,max_x + 1,min_y,max_y + 1);

            int nb_white_pixel = (max_x + 1 - min_x) * (max_y + 1 - min_y) - nb_black_pixel;


            //passe dans l'ensemble des filtres pour voir celui qui a le plus de correspondance :

            int best_matching_filter = -1; 
            int best_matching_nb = 0;

            int filter_try = 1;
            
            
            for(filter_try = 1; filter_try < 10 ; filter_try++ )
            {
                int current_matching = filter_number(rows,min_x,max_x + 1,min_y,max_y + 1, filter_try);

                if(current_matching > best_matching_nb)
                {
                    best_matching_nb = current_matching;
                    best_matching_filter = filter_try;
                }
            }


            //printf("better matching %d : nb pixel matching %d%% (%d)\n", best_matching_filter, best_matching_nb * 100 / (nb_black_pixel * 2 + nb_white_pixel),best_matching_nb);            
            if(best_matching_nb > 0)
            {
                nb_cells_value_detected++;
                set_initial_value_cell(grille_to_load,j,i,best_matching_filter);
            }
        }
    }
    return nb_cells_value_detected;
}

int filter_number(png_bytepp rows,int x_start, int x_end, int y_start, int y_end, int filter)
{
    switch(filter)
    {
        case 1:
            return filter_one(rows,x_start,x_end,y_start,y_end);
        case 2:
            return filter_two(rows,x_start,x_end,y_start,y_end);
        case 3:
            return filter_three(rows,x_start,x_end,y_start,y_end);
        case 4:
            return filter_four(rows,x_start,x_end,y_start,y_end);
        case 5:
            return filter_five(rows,x_start,x_end,y_start,y_end);
        case 6:
            return filter_six(rows,x_start,x_end,y_start,y_end);
        case 7:
            return filter_seven(rows,x_start,x_end,y_start,y_end);
        case 8:
            return filter_eight(rows,x_start,x_end,y_start,y_end);
        case 9:
            return filter_nine(rows,x_start,x_end,y_start,y_end);
        default :
            return 0;
    }
}

int filter_one(png_bytepp rows, int x_start, int x_end, int y_start, int y_end)
{
    // Variante A: 1 avec empattement en haut à gauche
    /* binaire */
    static int pattern_one_a[81] = 
    {
        0,0,0,1,3,1,0,0,0,
        0,0,1,3,3,1,0,0,0,
        0,1,3,1,3,1,0,0,0,
        0,0,0,1,3,1,0,0,0,
        0,0,0,1,3,1,0,0,0,
        0,0,0,1,3,1,0,0,0,
        0,0,0,1,3,1,0,0,0,
        0,0,0,1,3,1,0,0,0,
        3,3,3,3,3,3,3,3,3,
    };
    /* pondere
    static int pattern_one_a[81] = 
    {
        0,0,1,2,3,2,1,0,0,
        0,1,2,3,3,2,1,0,0,
        1,2,3,2,3,2,1,0,0,
        1,1,1,2,3,2,1,0,0,
        0,0,1,2,3,2,1,0,0,
        0,0,1,2,3,2,1,0,0,
        0,0,1,2,3,2,1,0,0,
        2,2,2,2,3,2,2,2,2,
        3,3,3,3,3,3,3,3,3,
    };
    */
    int test_a = filter_with_pattern(rows, x_start, x_end, y_start, y_end, pattern_one_a);

    // Variante B: 1 droit simple (plus strict)
    /* binaire */
    static int pattern_one_b[81] = 
    {
        0,0,0,3,3,3,3,3,3,
        0,0,1,3,3,1,1,3,3,
        1,3,3,1,1,0,1,3,3,
        3,3,1,0,0,0,1,3,3,
        0,0,0,0,0,0,1,3,3,
        0,0,0,0,0,0,1,3,3,
        0,0,0,0,0,0,1,3,3,
        0,0,0,0,0,0,1,3,3,
        0,0,0,0,0,0,1,3,3,
    };
    int test_b = filter_with_pattern(rows, x_start, x_end, y_start, y_end, pattern_one_b);

    return test_a > test_b ? test_a : test_b;
}

int filter_two(png_bytepp rows, int x_start, int x_end, int y_start, int y_end)
{
    // Pattern pour le chiffre 2 - courbe en haut, diagonale, base
    /* binaire */
    static int pattern_two[81] = 
    {
        1,2,3,3,3,3,3,3,3,
        2,3,1,1,0,0,2,3,3,
        1,0,0,0,0,0,3,2,2,
        0,0,0,0,0,3,0,0,0,
        0,0,0,0,3,0,0,0,0,
        0,0,0,3,0,0,0,0,0,
        0,0,3,0,0,0,0,0,1,
        3,3,3,3,0,0,1,3,2,
        3,3,3,3,3,3,3,3,2,
    };
    return filter_with_pattern(rows, x_start, x_end, y_start, y_end, pattern_two);
}

int filter_three(png_bytepp rows, int x_start, int x_end, int y_start, int y_end)
{
    // Pattern pour le chiffre 3 - deux courbes à droite
    /* binaire */
    static int pattern_three[81] = 
    {
        0,0,3,3,3,3,3,0,0,
        0,3,0,0,0,0,2,3,0,
        1,0,0,0,0,0,1,2,3,
        0,0,0,0,0,0,1,2,3,
        0,0,0,0,3,3,3,3,0,
        0,0,0,0,0,0,1,2,3,
        1,0,0,0,0,0,1,2,3,
        0,3,0,0,0,0,2,3,0,
        0,0,3,3,3,3,3,0,0,
    };
    return filter_with_pattern(rows, x_start, x_end, y_start, y_end, pattern_three);
}

int filter_four(png_bytepp rows, int x_start, int x_end, int y_start, int y_end)
{
    // Variante A: 4 avec diagonale classique
    /* binaire */
    static int pattern_four[81] = 
    {
        0,0,0,0,0,3,3,3,0,
        0,0,0,0,3,2,2,3,0,
        0,0,0,3,2,0,2,3,0,
        0,0,3,2,0,0,2,3,0,
        0,3,2,0,0,0,2,3,0,
        3,3,3,3,3,3,3,3,3,
        0,0,0,0,0,0,2,3,0,
        0,0,0,0,0,0,2,3,0,
        0,0,0,0,0,0,2,3,0,
    };
    return filter_with_pattern(rows, x_start, x_end, y_start, y_end, pattern_four);
}

int filter_five(png_bytepp rows, int x_start, int x_end, int y_start, int y_end)
{
    // Pattern pour le chiffre 5 - barre horizontale en haut, courbe en bas

    static int pattern_five_a[81] = 
    {
        0,0,3,3,3,3,3,0,0,
        0,3,3,2,2,2,2,0,0,
        3,2,1,0,0,0,0,0,0,
        3,2,1,1,0,0,0,0,0,
        3,3,3,3,3,3,3,3,2,
        0,0,0,0,0,1,1,2,3,
        0,0,0,0,0,0,1,2,3,
        0,3,1,0,0,1,1,3,0,
        0,0,3,3,3,3,3,0,0,
    };
        int result_a = filter_with_pattern(rows, x_start, x_end, y_start, y_end, pattern_five_a);

    static int pattern_five_b[81] = 
    {
        3,3,3,3,3,3,3,3,0,
        3,2,2,1,1,1,1,1,0,
        3,2,0,0,0,0,0,0,0,
        3,2,0,0,0,0,0,0,0,
        3,3,3,3,3,3,3,3,2,
        0,0,0,0,0,1,1,2,3,
        0,0,0,0,0,0,1,2,3,
        0,3,1,0,0,1,1,3,0,
        0,0,3,3,3,3,3,0,0,
    };


    int result_b = filter_with_pattern(rows, x_start, x_end, y_start, y_end, pattern_five_b);

    return result_a > result_b ? result_a : result_b;
}

int filter_six(png_bytepp rows, int x_start, int x_end, int y_start, int y_end)
{
    // Pattern pour le chiffre 6 - courbe fermée en bas
    static int pattern_six[81] = 
    {
        0,0,3,3,3,3,3,0,0,
        0,3,2,1,0,0,3,3,0,
        3,2,1,0,0,0,0,0,0,
        3,2,1,0,0,0,0,0,0,
        3,2,1,3,3,3,3,3,2,
        3,2,1,1,0,1,1,2,3,
        3,2,1,0,0,0,1,2,3,
        0,3,1,1,0,1,1,3,0,
        0,0,3,3,3,3,3,0,0,
    };
    return filter_with_pattern(rows, x_start, x_end, y_start, y_end, pattern_six);
}

int filter_seven(png_bytepp rows, int x_start, int x_end, int y_start, int y_end)
{
    static int pattern_seven_a[81] = 
    {
        3,3,3,3,3,3,3,3,3,
        1,1,1,1,1,1,1,3,1,
        0,0,0,0,0,0,3,0,0,
        0,0,0,0,0,3,0,0,0,
        0,0,0,0,3,0,0,0,0,
        0,0,0,3,0,0,0,0,0,
        0,0,3,0,0,0,0,0,0,
        0,3,0,0,0,0,0,0,0,
        3,0,0,0,0,0,0,0,0,
    };
    int result_a = filter_with_pattern(rows, x_start, x_end, y_start, y_end, pattern_seven_a);
    static int pattern_seven_b[81] = 
    {
        3,3,3,3,3,3,3,3,3,
        1,1,1,1,1,1,1,3,3,
        0,0,0,0,0,0,1,1,3,
        0,0,0,0,0,0,1,3,0,
        0,0,0,0,0,1,3,0,0,
        0,0,0,0,1,3,0,0,0,
        0,0,0,0,3,1,0,0,0,
        0,0,0,3,1,0,0,0,0,
        0,0,3,1,1,0,0,0,0,
    };
    int result_b = filter_with_pattern(rows, x_start, x_end, y_start, y_end, pattern_seven_b);

    return result_a > result_b ? result_a : result_b;
}

int filter_eight(png_bytepp rows, int x_start, int x_end, int y_start, int y_end)
{
    // Pattern pour le chiffre 8 - deux boucles superposées
    static int pattern_eight[81] = 
    {
        0,0,3,3,3,3,3,0,0,
        0,3,2,1,0,1,2,3,0,
        3,2,1,0,0,0,1,2,3,
        2,2,1,1,0,1,1,2,2,
        1,3,3,3,3,3,3,3,1,
        2,2,1,1,0,1,1,2,2,
        3,2,1,0,0,0,1,2,3,
        0,3,1,1,0,1,1,3,0,
        0,0,3,3,3,3,3,0,0,
    };
 
    return filter_with_pattern(rows, x_start, x_end, y_start, y_end, pattern_eight);
}

int filter_nine(png_bytepp rows, int x_start, int x_end, int y_start, int y_end)
{
    // Variante A: 9 avec boucle fermée en haut
    static int pattern_nine_a[81] = 
    {
        0,0,3,3,3,3,3,0,0,
        0,3,2,1,0,1,2,3,0,
        3,2,1,0,0,0,1,2,3,
        3,2,2,1,0,1,2,2,3,
        0,3,3,3,3,3,3,3,3,
        0,0,0,0,0,0,1,2,3,
        0,0,0,0,0,0,1,2,3,
        0,1,0,0,0,0,1,3,0,
        0,0,3,3,3,3,3,0,0,
    };
    int test_a = filter_with_pattern(rows, x_start, x_end, y_start, y_end, pattern_nine_a);

    // Variante B: 9 avec trait vertical droit
    static int pattern_nine_b[81] = 
    {
        0,0,3,3,3,3,3,0,0,
        0,2,2,1,0,1,2,2,0,
        3,2,1,0,0,0,1,2,3,
        3,2,2,1,0,1,2,2,3,
        0,2,3,3,3,3,3,3,3,
        0,0,0,0,0,0,1,3,0,
        0,0,0,0,0,1,3,0,0,
        0,0,0,0,1,3,0,0,0,
        0,0,3,3,3,3,0,0,0,
    };
    int test_b = filter_with_pattern(rows, x_start, x_end, y_start, y_end, pattern_nine_b);

    return test_a > test_b ? test_a : test_b;
}

int filter_with_pattern(png_bytepp rows, int x_start, int x_end, int y_start, int y_end, int *pattern)
{
    int matching_score = 0;
    int width = x_end - x_start;
    int height = y_end - y_start;
    
    for(int x = x_start; x < x_end; x++)
    {
        //printf("\n");
        for(int y = y_start; y < y_end; y++)
        {
            png_byte red   = rows[x][y*NB_CHANELS_PNG];
            png_byte green = rows[x][y*NB_CHANELS_PNG + 1];
            png_byte blue  = rows[x][y*NB_CHANELS_PNG + 2];
            
            int current_pixel_is_black = isBlack(red, green, blue);
            
            // Mapping du pixel vers le pattern 9x9
            // Calcul de la position dans le pattern (0-8 pour x et y)
            int pattern_x = (x - x_start) * 9 / width;
            int pattern_y = (y - y_start) * 9 / height;
            
            // Index dans le tableau 1D du pattern
            int pattern_index = pattern_x * 9 + pattern_y;
            int current_pattern_value_is_black = pattern[pattern_index];
            
            if(current_pixel_is_black)
            {
                matching_score+=current_pattern_value_is_black;
            }
            else
            {
                matching_score-=current_pattern_value_is_black;
            }
            
            // debug
            /*
            if(current_pixel_is_black)
            {
                if(current_pattern_value_is_black)
                {
                    printf("X");
                }
                else
                {
                    printf("\\");
                }
            }
            else
            {
                if(current_pattern_value_is_black)
                {
                    printf("/");
                }
                else
                {
                    printf(" ");
                }
            }
            */
        }
    }    
    //printf("\n");

    return matching_score;
}

int find_cells(png_bytepp rows, int width, int height, int * line_detected, int * column_detected, cells_detected cells[GRILLE_DIM + 1][GRILLE_DIM + 1])
{
    int nb_cells_detected = 0;

    //find cells
    for(int i=0; i< GRILLE_DIM; i++)
    {
        for(int j=0; j< GRILLE_DIM; j++)
        {
            //verifie qu'on s'inscrit dans un rectangle blanc sinon on le reduit 
            int black_detected = 0;
            int white_detected = 0;

            int cell_top = line_detected[i];
            int cell_bottom = line_detected[i + 1];
            int cell_left = column_detected[j];
            int cell_right = column_detected[j+1];

            do
            {   
                //printf("\n");
                //show_sub_png(rows,cell_top,cell_bottom,cell_left,cell_right);

                black_detected = 0;
                white_detected = 0;
                
                //     _
                // -> |
                //
                for(int x = cell_top; x < cell_bottom -1; x++)
                {
                    png_byte red      = rows[x][cell_left*NB_CHANELS_PNG];
                    png_byte green    = rows[x][cell_left*NB_CHANELS_PNG + 1];
                    png_byte blue     = rows[x][cell_left*NB_CHANELS_PNG + 2];
                    
                    if(isBlack(red,green,blue))
                    {
                        black_detected = 1;
                    }
                    else
                    {
                        white_detected = 1;
                    }
                }

                if(black_detected)
                {
                    //si on a même pas detecté 1 pixel blanc on relance la boucle avec la reduction par la droite
                    if(!white_detected)
                    {
                        cell_left++;
                    }
                }

                black_detected = 0;
                white_detected = 0;
                //    _ <-
                //   |
                //    
                for(int y = cell_left; y < cell_right -1; y++)
                {
                    png_byte red      = rows[cell_top][y*NB_CHANELS_PNG];
                    png_byte green    = rows[cell_top][y*NB_CHANELS_PNG + 1];
                    png_byte blue     = rows[cell_top][y*NB_CHANELS_PNG + 2];
                    
                    if(isBlack(red,green,blue))
                    {
                        black_detected = 1;
                    }
                    else
                    {
                        white_detected = 1;
                    }
                }

                if(black_detected)
                {
                    //si on a même pas detecté 1 pixel blanc on relance la boucle avec la reduction par le haut
                    if(!white_detected)
                    {
                        cell_top++;
                    }
                }

                black_detected = 0;
                white_detected = 0;
                //
                //   _| <-
                //    
                for(int x = cell_top + 1; x < cell_bottom; x++)
                {
                    png_byte red      = rows[x][cell_right*NB_CHANELS_PNG];
                    png_byte green    = rows[x][cell_right*NB_CHANELS_PNG + 1];
                    png_byte blue     = rows[x][cell_right*NB_CHANELS_PNG + 2];
                    
                    if(isBlack(red,green,blue))
                    {
                        black_detected = 1;
                    }
                    else
                    {
                        white_detected = 1;
                    }
                }

                if(black_detected)
                {
                    //si on a même pas detecté 1 pixel blanc on relance la boucle avec la reduction par la gauche
                    if(!white_detected)
                    {
                        cell_right--;
                    }
                }

                black_detected = 0;
                white_detected = 0;
                //
                //  -> _|
                //   
                for(int y = cell_left + 1; y < cell_right; y++)
                {
                    png_byte red      = rows[cell_bottom][y*NB_CHANELS_PNG];
                    png_byte green    = rows[cell_bottom][y*NB_CHANELS_PNG + 1];
                    png_byte blue     = rows[cell_bottom][y*NB_CHANELS_PNG + 2];
                    
                    if(isBlack(red,green,blue))
                    {
                        black_detected = 1;
                    }
                    else
                    {
                        white_detected = 1;
                    }
                }

                if(black_detected)
                {
                    cell_bottom--;
                }
              

            } while(black_detected && cell_bottom > cell_top && cell_left < cell_right);

            if(cell_bottom <= cell_top && cell_left >= cell_right)
            {
                cells[i][j].top     = -1;
                cells[i][j].bottom  = -1;
                cells[i][j].right   = -1;
                cells[i][j].left    = -1;
            }
            else
            {
                
                //printf("cell %d,%d : \ntop left angle = (%d,%d)\nbottom right angle = (%d,%d)\n", i, j, cell_left, cell_top, cell_right,cell_bottom);

                cells[i][j].top     = cell_top + 1;
                cells[i][j].bottom  = cell_bottom - 1;
                cells[i][j].right   = cell_right - 1;
                cells[i][j].left    = cell_left + 1;

                //show_sub_png(rows,cell_top,cell_bottom,cell_left,cell_right);

                nb_cells_detected++;
            }
        }
    }

    return nb_cells_detected;
}


int find_line(png_bytepp rows, int width, int height, int line_detected[GRILLE_DIM + 1])
{
    // find lines
    int *nb_black_by_line = calloc(height, sizeof(int));
    int current_black = 0;
    
    for (int i = 0; i < height; i++) 
    {
        current_black = 0;
        for (int j = 0; j < width; j++) 
        {
            png_byte red   = rows[i][j*NB_CHANELS_PNG];
            png_byte green = rows[i][j*NB_CHANELS_PNG + 1];
            png_byte blue  = rows[i][j*NB_CHANELS_PNG + 2];
            
            if(isBlack(red, green, blue))
            {
                nb_black_by_line[i]++;
            }
        }
    }
    
    // pour les GRILLE_DIM + 1 lignes  
    int max_streak_line[GRILLE_DIM + 1] = {0,0,0,0,0,0,0,0,0,0};
    
    // init line
    for(int i = 0; i < GRILLE_DIM + 1; i++)
    {
        line_detected[i] = -1;
    }
    
    printf("search Ligne\n");
    for (int i = 0; i < height; i++)
    {
        if(nb_black_by_line[i] > width / 2)
        {
            while(i < height - 1 && nb_black_by_line[i] > nb_black_by_line[i + 1] - nb_black_by_line[i] * 5 / 100 &&  nb_black_by_line[i] < nb_black_by_line[i + 1] + nb_black_by_line[i] * 5 / 100)
            {
                i++;
            }
            
            int current_line = i;
            int current_value = nb_black_by_line[i];
            
            for(int j = 0; j < GRILLE_DIM + 1; j++)
            {
                if(current_value > max_streak_line[j])
                {
                    int tmp1 = max_streak_line[j];
                    int tmp2 = line_detected[j];
                    max_streak_line[j] = current_value;
                    line_detected[j] = current_line;
                    // pour enlever le plus petit des GRILLE_DIM + 1 max
                    current_value = tmp1;
                    current_line = tmp2;
                }
            }
        }
    }
    
    free(nb_black_by_line);
    
    // TRI par ordre croissant
    for(int i = 0; i < GRILLE_DIM + 1; i++)
    {
        for(int j = i + 1; j < GRILLE_DIM + 1; j++)
        {
            if(line_detected[i] > line_detected[j] && line_detected[j] != -1)
            {
                // Échanger les positions
                int tmp = line_detected[i];
                line_detected[i] = line_detected[j];
                line_detected[j] = tmp;
                
                // Échanger aussi les valeurs correspondantes
                tmp = max_streak_line[i];
                max_streak_line[i] = max_streak_line[j];
                max_streak_line[j] = tmp;
            }
        }
    }

    printf("les lignes detectees : \n");
    int nb_line_detected = 0;
    for(int i = 0; i < GRILLE_DIM + 1; i++)
    {
        if(line_detected[i] != -1)
        {
            printf("%d (%d)\n", line_detected[i], max_streak_line[i]);
            nb_line_detected++;
        }
    }
    
    return nb_line_detected;
}

int find_column(png_bytepp rows, int width, int height, int column_detected[GRILLE_DIM + 1])
{
    int *nb_black_by_column = calloc(width, sizeof(int));
    int current_black = 0;
    
    for (int i = 0; i < width; i++) 
    {
        current_black = 0;

        for (int j = 0; j < height; j++) 
        {
            png_byte red   = rows[j][i*NB_CHANELS_PNG];
            png_byte green = rows[j][i*NB_CHANELS_PNG + 1];
            png_byte blue  = rows[j][i*NB_CHANELS_PNG + 2];
            
            if(isBlack(red, green, blue))
            {
                nb_black_by_column[i]++;
            }
        }
    }
    
    // pour les GRILLE_DIM + 1 colonnes
    int max_streak_column[GRILLE_DIM + 1] = {0,0,0,0,0,0,0,0,0,0};
    
    // init column_detected
    for(int i = 0; i < GRILLE_DIM + 1; i++)
    {
        column_detected[i] = -1;
    }
    
    // search max black colonnes
    for (int i = 0; i < width; i++)
    {
        if(nb_black_by_column[i] > height / 2)
        {

            while(i < width - 1 && nb_black_by_column[i] > nb_black_by_column[i + 1] - nb_black_by_column[i] * 5 / 100 && nb_black_by_column[i] < nb_black_by_column[i + 1] + nb_black_by_column[i] * 5 / 100)
            {
                i++;
            }
            
            int current_column = i;
            int current_value = nb_black_by_column[i];
            
            for(int j = 0; j < GRILLE_DIM + 1; j++)
            {
                if(current_value > max_streak_column[j])
                {
                    int tmp1 = max_streak_column[j];
                    int tmp2 = column_detected[j];
                    max_streak_column[j] = current_value;
                    column_detected[j] = current_column;
                    // pour enlever le plus petit des GRILLE_DIM + 1 max
                    current_value = tmp1;
                    current_column = tmp2;
                }
            }
        }
    }
    
    free(nb_black_by_column);
    
    // TRI par ordre croissant
    for(int i = 0; i < GRILLE_DIM + 1; i++)
    {
        for(int j = i + 1; j < GRILLE_DIM + 1; j++)
        {
            if(column_detected[i] > column_detected[j] && column_detected[j] != -1)
            {
                // Échanger les positions
                int tmp = column_detected[i];
                column_detected[i] = column_detected[j];
                column_detected[j] = tmp;
                
                // Échanger aussi les valeurs correspondantes
                tmp = max_streak_column[i];
                max_streak_column[i] = max_streak_column[j];
                max_streak_column[j] = tmp;
            }
        }
    }


    printf("les colonnes detectees : \n");
    int nb_column_detected = 0;
    for(int i = 0; i < GRILLE_DIM + 1; i++)
    {
        if(column_detected[i] != -1)
        {
            printf("%d (%d)\n", column_detected[i], max_streak_column[i]);
            nb_column_detected++;
        }
    }
    
    return nb_column_detected;
}



// USE EXTERNAL OCR

/*
#include "asprise_ocr_api.h"

// Fonction pour déterminer dans quelle cellule se trouve un point (x,y)
int find_cell_for_position(int x, int y, int *row, int *col, 
                           int line_detected[GRILLE_DIM + 1], 
                           int column_detected[GRILLE_DIM + 1])
{
    *row = -1;
    *col = -1;

    // Trouver la ligne (y correspond aux lignes)
    for (int i = 0; i < GRILLE_DIM; i++) {
        if (y >= line_detected[i] && y <= line_detected[i + 1]) {
            *row = i;
            break;
        }
    }

    // Trouver la colonne (x correspond aux colonnes)
    for (int j = 0; j < GRILLE_DIM; j++) {
        if (x >= column_detected[j] && x <= column_detected[j + 1]) {
            *col = j;
            break;
        }
    }

    return (*row != -1 && *col != -1) ? 1 : 0;
}

// Parser le XML pour extraire les chiffres depuis les balises <block>
// Le XML contient des <cell> avec row/col et des <block> avec le chiffre
int parse_ocr_xml_result(const char *xml, int grille_result[GRILLE_DIM][GRILLE_DIM])
{
    if (xml == NULL) return 0;
    
    int count = 0;
    const char *ptr = xml;
    
    // Chercher les balises <cell> dans le XML
    while ((ptr = strstr(ptr, "<cell")) != NULL) {
        int row = -1, col = -1;
        
        // Extraire row
        const char *row_pos = strstr(ptr, "row=\"");
        if (row_pos && row_pos < ptr + 300) {
            row = atoi(row_pos + 5);
        }
        
        // Extraire col
        const char *col_pos = strstr(ptr, "col=\"");
        if (col_pos && col_pos < ptr + 300) {
            col = atoi(col_pos + 5);
        }
        
        // Trouver la fin de cette cellule
        const char *cell_end = strstr(ptr, "</cell>");
        if (!cell_end) {
            ptr++;
            continue;
        }
        
        // Chercher un <block> dans cette cellule
        const char *block_start = strstr(ptr, "<block");
        if (block_start && block_start < cell_end) {
            // Trouver le contenu du block (entre > et </block>)
            const char *content_start = strchr(block_start, '>');
            const char *content_end = strstr(block_start, "</block>");
            
            if (content_start && content_end && content_end > content_start) {
                content_start++; // Passer le '>'
                
                // Extraire le chiffre
                char digit = '\0';
                while (content_start < content_end) {
                    if (*content_start >= '1' && *content_start <= '9') {
                        digit = *content_start;
                        break;
                    }
                    // La version d'évaluation remplace 0 et 9 par *
                    if (*content_start == '*') {
                        digit = '*';
                        break;
                    }
                    content_start++;
                }
                
                // Si on a trouvé un chiffre avec row/col valides
                if (digit != '\0' && row >= 0 && row < GRILLE_DIM && col >= 0 && col < GRILLE_DIM) {
                    if (digit >= '1' && digit <= '8') {
                        grille_result[row][col] = digit - '0';
                        printf("Cell [%d,%d] = '%c'\n", row, col, digit);
                        count++;
                    } else if (digit == '*') {
                        // Pour la version d'évaluation, on peut marquer comme inconnu
                        // ou essayer de deviner (0 ou 9)
                        grille_result[row][col] = 9;  // Marquer comme inconnu
                        printf("Cell [%d,%d] =  9\n", row, col);
                        count++;
                    }
                }
            }
        }
        
        // Passer à la prochaine cellule
        ptr = cell_end + 1;
    }
    
    return count;
}

void determine_cells_value_external_ocr(const char *png_file_name,
                               int grille_result[GRILLE_DIM][GRILLE_DIM])
{
    const char * libFolder = "/home/etienne/Documents/sudokuSolver/";
    LIBRARY_HANDLE libHandle = dynamic_load_aocr_library(libFolder);

    if (libHandle == NULL) {
        printf("ERROR: Failed to load OCR library\n");
        return;
    }

    printf("OCR Version: %s\n", c_com_asprise_ocr_version());

    int setup = c_com_asprise_ocr_setup(0);
    if (setup != 1) {
        printf("ERROR: OCR setup failed with code: %d\n", setup);
        return;
    }

    // Propriétés pour reconnaître uniquement les chiffres
    const char * startProps = "START_PROP_DICT_SKIP_BUILT_IN_ALL=true";
    
    long long ptrToApi = c_com_asprise_ocr_start("eng", OCR_SPEED_FAST, 
                                                  startProps, ",", "=");
    if (ptrToApi == 0) {
        printf("ERROR: OCR engine failed to start\n");
        return;
    }

    printf("\n=== OCR sur l'image complète ===\n");

    // Initialiser la grille
    for(int i = 0; i < GRILLE_DIM; i++) {
        for(int j = 0; j < GRILLE_DIM; j++) {
            grille_result[i][j] = 0;
        }
    }

    // Essayer d'abord avec le format XML pour avoir les positions
    printf("Tentative avec format XML...\n");
    char * xml_result = c_com_asprise_ocr_recognize(
        ptrToApi, 
        png_file_name, 
        -1,    // Page index
        -1, -1, -1, -1,  // Toute l'image
        OCR_RECOGNIZE_TYPE_TEXT, 
        OCR_OUTPUT_FORMAT_XML,
        NULL,
        NULL,
        NULL
    );

    if (xml_result != NULL && strlen(xml_result) > 0) {
        printf("XML reçu (%zu caractères)\n", strlen(xml_result));
        
        // Sauvegarder le XML pour debug
        FILE *f = fopen("ocr_debug.xml", "w");
        if (f) {
            fprintf(f, "%s", xml_result);
            fclose(f);
            printf("XML sauvegardé dans ocr_debug.xml\n");
        }
        
        // Parser le XML - maintenant directement dans la grille
        int digit_count = parse_ocr_xml_result(xml_result, grille_result);
        
        printf("\n%d chiffres détectés\n", digit_count);
    } else {
        printf("Format XML n'a pas fonctionné, essai avec plaintext...\n");
        
        return;
    }

    // Afficher la grille résultante
    printf("\n=== Grille détectée ===\n");
    printf("   ");
    for(int j = 0; j < GRILLE_DIM; j++) printf("%d ", j);
    printf("\n");
    
    for(int i = 0; i < GRILLE_DIM; i++) {
        printf("%d: ", i);
        for(int j = 0; j < GRILLE_DIM; j++) {
            if(grille_result[i][j] == 0) {
                printf(". ");
            } else if(grille_result[i][j] == -1) {
                printf("? ");
            } else {
                printf("%d ", grille_result[i][j]);
            }
        }
        printf("\n");
    }

    c_com_asprise_ocr_stop(ptrToApi);
    dynamic_unload_aocr_library(libHandle);
}
*/