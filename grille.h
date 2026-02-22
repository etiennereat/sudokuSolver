#ifndef GRILLE_H
#define GRILLE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define GRILLE_DIM      9
#define EMPTY_CELL_VALUE 0

/* états possibles d'une cellule */
#define CELL_UNSET           0
#define CELL_INITIAL_SET     1
#define CELL_SUPPOSITION_SET 2
#define CELL_DEDUCTION_SET   3

/* masques candidats */
#define MASK_FILTER_1   0b0000000010
#define MASK_FILTER_2   0b0000000100
#define MASK_FILTER_3   0b0000001000
#define MASK_FILTER_4   0b0000010000
#define MASK_FILTER_5   0b0000100000
#define MASK_FILTER_6   0b0001000000
#define MASK_FILTER_7   0b0010000000
#define MASK_FILTER_8   0b0100000000
#define MASK_FILTER_9   0b1000000000
#define MASK_ALL_FILTER 0b0000000000
#define MASK_NO_FILTER  0b1111111110

/* états du solver */
#define GRILLE_OK         0
#define GRILLE_SOLVED     1
#define GRILLE_UNSOLVED   2
#define GRILLE_IMPOSSIBLE 3

/* états candidat */
#define NO_CANDIDAT     0
#define SINGLE_CANDIDAT 1
#define MULTI_CANDIDAT  2

/* ── Structures ─────────────────────────────────────────────────────────── */

typedef struct _cell
{
    __uint16_t current_value;
    __uint8_t  state;           /* CELL_UNSET, CELL_INITIAL_SET, CELL_SUPPOSITION_SET, CELL_DEDUCTION_SET */
} cell;

typedef struct _line
{
    __uint16_t free_values;
} line;

typedef struct _collomn
{
    __uint16_t free_values;
} collomn;

typedef struct _carre
{
    __uint16_t free_values;
} carre;

typedef struct _grille
{
    cell    cells[GRILLE_DIM * GRILLE_DIM];
    line    lines[GRILLE_DIM];
    collomn collomn[GRILLE_DIM];
    carre   carres[GRILLE_DIM];
    int     nb_unset_cell;
} grille;

/* ── Prototypes ──────────────────────────────────────────────────────────── */

int         get_carre_index(int x, int y);
int         get_cell_index(int x, int y);

grille     *create_grille(void);
void        delete_grille(grille *g);
void        copy_grille(grille *destination, grille *source);

__uint16_t  get_mask_candidat(grille *g, int x, int y);
__uint16_t  is_candidat(grille *g, int x, int y, int chiffre);
int         have_single_candidat(grille *g, int x, int y, __uint16_t *candidat);

void        set_initial_value_cell(grille *g, int x, int y, __uint16_t value);
int         set_cell(grille *g, int x, int y, __uint16_t value);
int         set_supposition(grille *g, int x, int y, __uint16_t value);

void        print_grille(grille *g);

#endif /* GRILLE_H */