#include "grille.h"
#include <string.h>

int get_carre_index(int x, int y) {
    return (y / 3) * 3 + (x / 3);
}

int get_cell_index(int x, int y) {
    return y * GRILLE_DIM + x;
}

grille *create_grille()
{
    grille *g = malloc(sizeof(grille));
    if (g == NULL) return NULL;

    for (int x = 0; x < GRILLE_DIM; x++)
    {
        for (int y = 0; y < GRILLE_DIM; y++)
        {
            g->cells[get_cell_index(x, y)].current_value = EMPTY_CELL_VALUE;
            g->cells[get_cell_index(x, y)].state         = CELL_UNSET;
        }
        g->lines[x].free_values   = MASK_NO_FILTER;
        g->collomn[x].free_values = MASK_NO_FILTER;
        g->carres[x].free_values  = MASK_NO_FILTER;
    }
    g->nb_unset_cell = GRILLE_DIM * GRILLE_DIM;
    return g;
}

void delete_grille(grille *g)
{
    if (g != NULL)
        free(g);
}

__uint16_t get_mask_candidat(grille *g, int x, int y)
{
    if (g->cells[get_cell_index(x, y)].state != CELL_UNSET)
        return 0;
    return g->lines[y].free_values
         & g->collomn[x].free_values
         & g->carres[get_carre_index(x, y)].free_values;
}

__uint16_t is_candidat(grille *g, int x, int y, int chiffre)
{
    return get_mask_candidat(g, x, y) & 1 << chiffre;
}

int have_single_candidat(grille *g, int x, int y, __uint16_t *candidat)
{
    *candidat = 0;
    __uint16_t mask_candidat = get_mask_candidat(g, x, y);

    if (mask_candidat == 0)
        return NO_CANDIDAT;

    switch (mask_candidat)
    {
        case MASK_FILTER_1: *candidat = 1; return SINGLE_CANDIDAT;
        case MASK_FILTER_2: *candidat = 2; return SINGLE_CANDIDAT;
        case MASK_FILTER_3: *candidat = 3; return SINGLE_CANDIDAT;
        case MASK_FILTER_4: *candidat = 4; return SINGLE_CANDIDAT;
        case MASK_FILTER_5: *candidat = 5; return SINGLE_CANDIDAT;
        case MASK_FILTER_6: *candidat = 6; return SINGLE_CANDIDAT;
        case MASK_FILTER_7: *candidat = 7; return SINGLE_CANDIDAT;
        case MASK_FILTER_8: *candidat = 8; return SINGLE_CANDIDAT;
        case MASK_FILTER_9: *candidat = 9; return SINGLE_CANDIDAT;
        default:            return MULTI_CANDIDAT;
    }
}

void set_initial_value_cell(grille *g, int x, int y, __uint16_t value)
{
    int idx = get_cell_index(x, y);

    if (g->cells[idx].state == CELL_UNSET)
        g->nb_unset_cell--;

    g->cells[idx].current_value = value;
    g->cells[idx].state         = CELL_INITIAL_SET;

    __uint16_t value_mask = ~(1 << value);
    g->lines[y].free_values                      &= value_mask;
    g->collomn[x].free_values                    &= value_mask;
    g->carres[get_carre_index(x, y)].free_values &= value_mask;
}

int set_cell(grille *g, int x, int y, __uint16_t value)
{
    int idx = get_cell_index(x, y);

    if (g->cells[idx].state == CELL_UNSET)
        g->nb_unset_cell--;

    g->cells[idx].current_value = value;
    g->cells[idx].state         = CELL_DEDUCTION_SET;

    __uint16_t value_mask = ~(1 << value);
    g->lines[y].free_values                      &= value_mask;
    g->collomn[x].free_values                    &= value_mask;
    g->carres[get_carre_index(x, y)].free_values &= value_mask;

    __uint16_t neighbor_candidat;

    /* propagation sur la colonne */
    for (int y_neighbor = 0; y_neighbor < GRILLE_DIM; y_neighbor++)
    {
        if (y_neighbor == y) continue;
        if (g->cells[get_cell_index(x, y_neighbor)].state != CELL_UNSET) continue;

        switch (have_single_candidat(g, x, y_neighbor, &neighbor_candidat))
        {
            case NO_CANDIDAT:
                return GRILLE_IMPOSSIBLE;
            case SINGLE_CANDIDAT:
                if (set_cell(g, x, y_neighbor, neighbor_candidat) == GRILLE_IMPOSSIBLE)
                    return GRILLE_IMPOSSIBLE;
                break;
            default: break;
        }
    }

    /* propagation sur la ligne */
    for (int x_neighbor = 0; x_neighbor < GRILLE_DIM; x_neighbor++)
    {
        if (x_neighbor == x) continue;
        if (g->cells[get_cell_index(x_neighbor, y)].state != CELL_UNSET) continue;

        switch (have_single_candidat(g, x_neighbor, y, &neighbor_candidat))
        {
            case NO_CANDIDAT:
                return GRILLE_IMPOSSIBLE;
            case SINGLE_CANDIDAT:
                if (set_cell(g, x_neighbor, y, neighbor_candidat) == GRILLE_IMPOSSIBLE)
                    return GRILLE_IMPOSSIBLE;
                break;
            default: break;
        }
    }

    /* propagation sur le carré */
    for (int y_square = 0; y_square < 3; y_square++)
    {
        for (int x_square = 0; x_square < 3; x_square++)
        {
            int y_neighbor = y - y % 3 + y_square;
            int x_neighbor = x - x % 3 + x_square;

            if (x_neighbor == x && y_neighbor == y) continue;
            if (g->cells[get_cell_index(x_neighbor, y_neighbor)].state != CELL_UNSET) continue;

            switch (have_single_candidat(g, x_neighbor, y_neighbor, &neighbor_candidat))
            {
                case NO_CANDIDAT:
                    return GRILLE_IMPOSSIBLE;
                case SINGLE_CANDIDAT:
                    if (set_cell(g, x_neighbor, y_neighbor, neighbor_candidat) == GRILLE_IMPOSSIBLE)
                        return GRILLE_IMPOSSIBLE;
                    break;
                default: break;
            }
        }
    }

    return GRILLE_OK;
}

int set_supposition(grille *g, int x, int y, __uint16_t value)
{
    int result = set_cell(g, x, y, value);
    g->cells[get_cell_index(x, y)].state = CELL_SUPPOSITION_SET;
    return result;
}

void copy_grille(grille *destination, grille *source)
{
    memcpy(destination, source, sizeof(grille));
}

/* DEBUG DISPLAY */

const int color_foreground_BLACK         = 30;
const int color_background_GREEN         = 42;
const int color_background_LIGHT_CYAN    = 106;
const int color_background_LIGHT_MAGENTA = 105;

const char *exposants[] = {"⁰", "¹", "²", "³", "⁴", "⁵", "⁶", "⁷", "⁸", "⁹"};

void print_grille(grille *g)
{
    printf("╔═══╤═══╤═══╦═══╤═══╤═══╦═══╤═══╤═══╗\n");

    for (int y = 0; y < GRILLE_DIM; y++)
    {
        /* précalcul des couleurs et valeurs pour chaque cellule de la ligne */
        char       colors[GRILLE_DIM][32];
        __uint16_t values[GRILLE_DIM];

        for (int x = 0; x < GRILLE_DIM; x++)
        {
            values[x] = g->cells[get_cell_index(x, y)].current_value;

            switch (g->cells[get_cell_index(x, y)].state)
            {
                case CELL_SUPPOSITION_SET:
                    sprintf(colors[x], "\033[%d;%dm", color_foreground_BLACK, color_background_LIGHT_MAGENTA);
                    break;
                case CELL_INITIAL_SET:
                    sprintf(colors[x], "\033[%d;%dm", color_foreground_BLACK, color_background_GREEN);
                    break;
                case CELL_DEDUCTION_SET:
                    sprintf(colors[x], "\033[%d;%dm", color_foreground_BLACK, color_background_LIGHT_CYAN);
                    break;
                case CELL_UNSET:
                default:
                    sprintf(colors[x], "\033[m");
                    break;
            }
        }

        /* affichage des 3 sous-lignes */
        for (int i = 0; i < 3; i++)
        {
            for (int x = 0; x < GRILLE_DIM; x++)
            {
                printf("\033[m%s%s", x % 3 == 0 ? "║" : "│", colors[x]);

                for (int j = 0; j < 3; j++)
                {
                    if (j == 1 && i == 1 && values[x] != EMPTY_CELL_VALUE)
                        printf("%d", values[x]);
                    else
                    {
                        int current_candidat = i * 3 + j + 1;
                        printf("%s", is_candidat(g, x, y, current_candidat) ? exposants[current_candidat] : " ");
                    }
                }
            }
            printf("\033[m║\n");
        }

        if (y == GRILLE_DIM - 1)
            printf("╚═══╧═══╧═══╩═══╧═══╧═══╩═══╧═══╧═══╝\n");
        else if (y % 3 == 2)
            printf("╠═══╪═══╪═══╬═══╪═══╪═══╬═══╪═══╪═══╣\n");
        else
            printf("╟───┼───┼───╫───┼───┼───╫───┼───┼───╢\n");
    }
}