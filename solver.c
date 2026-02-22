#include "solver.h"

int is_solve(grille *g)
{
    return g->nb_unset_cell == 0 ? GRILLE_SOLVED : GRILLE_UNSOLVED;
}

/* 
Priorité à la cellule avec le moins de candidats
À égalité de candidats, priorité à la cellule avec le plus de voisins vides (degree)
    = contraindra le plus de cellules et réduira l'espace de recherche
*/
int find_best_supposition(grille g, int *x_best_supp, int *y_best_supp, __uint16_t *candidat_best_supp)
{
    int best_nb_candidats = 10;
    int best_degree       = -1;
    *x_best_supp          = -1;
    *y_best_supp          = -1;
    *candidat_best_supp   = 0;

    for (int y = 0; y < GRILLE_DIM; y++)
    {
        for (int x = 0; x < GRILLE_DIM; x++)
        {
            if (g.cells[get_cell_index(x, y)].state != CELL_UNSET)
                continue;

            /* compte les candidats disponibles */
            int candidats[GRILLE_DIM];
            int nb_candidats = 0;
            for (int c = 1; c <= GRILLE_DIM; c++)
            {
                if (is_candidat(&g, x, y, c))
                    candidats[nb_candidats++] = c;
            }

            if (nb_candidats == 0)
                continue;

            /*  on calcule le degree que si nb_candidats <= best */
            if (nb_candidats <= best_nb_candidats)
            {
                int degree = 0;

                for (int x_neighbor = 0; x_neighbor < GRILLE_DIM; x_neighbor++)
                    if (x_neighbor != x && g.cells[get_cell_index(x_neighbor, y)].state == CELL_UNSET)
                        degree++;

                for (int y_neighbor = 0; y_neighbor < GRILLE_DIM; y_neighbor++)
                    if (y_neighbor != y && g.cells[get_cell_index(x, y_neighbor)].state == CELL_UNSET)
                        degree++;

                for (int y_square = 0; y_square < 3; y_square++)
                {
                    for (int x_square = 0; x_square < 3; x_square++)
                    {
                        int y_neighbor = y - y % 3 + y_square;
                        int x_neighbor = x - x % 3 + x_square;
                        if (x_neighbor != x && y_neighbor != y && g.cells[get_cell_index(x_neighbor, y_neighbor)].state == CELL_UNSET)
                            degree++;
                    }
                }

                if (nb_candidats < best_nb_candidats ||
                    (nb_candidats == best_nb_candidats && degree > best_degree))
                {
                    best_nb_candidats   = nb_candidats;
                    best_degree         = degree;
                    *x_best_supp        = x;
                    *y_best_supp        = y;
                    *candidat_best_supp = candidats[0];
                }
            }
        }
    }

    return *x_best_supp != -1 && *y_best_supp != -1;
}

__uint16_t next_candidat(grille *g, int x, int y, __uint16_t candidat_precedent)
{
    for (__uint16_t i = candidat_precedent + 1; i <= GRILLE_DIM; i++)
    {
        if(is_candidat(g,x,y,i))
        {
            return i;
        }
    }
    return 0;
}

int try_to_solve(grille *g)
{
    __uint16_t candidat;

    for (int y = 0; y < GRILLE_DIM && is_solve(g) == GRILLE_UNSOLVED; y++)
    {
        for (int x = 0; x < GRILLE_DIM && is_solve(g) == GRILLE_UNSOLVED; x++)
        {
            if (g->cells[get_cell_index(x, y)].state == CELL_UNSET)
            {
                switch (have_single_candidat(g, x, y, &candidat))
                {
                    case SINGLE_CANDIDAT:
                        if (set_cell(g, x, y, candidat) == GRILLE_IMPOSSIBLE)
                            return GRILLE_IMPOSSIBLE;
                        break;
                    case MULTI_CANDIDAT:
                        break;
                    case NO_CANDIDAT:
                    default:
                        return GRILLE_IMPOSSIBLE;
                }
            }
        }
    }
    return is_solve(g);
}

/* Remonte la pile jusqu'à trouver une cellule qui a encore un candidat à essayer.
   Retourne 1 si une supposition a été trouvée, 0 si la pile est épuisée. */
int backtrack(pileSupp *supps, grille *g, int *x_out, int *y_out, __uint16_t *candidat_out)
{
    while (!pileSupp_empty(supps))
    {
        supposition *last_supp = pileSupp_pop(supps);
        printf("↺  Backtrack : annulation supposition (%d,%d) -> %d\n",
               last_supp->x, last_supp->y, last_supp->candidat);

        copy_grille(g, last_supp->grille_before_suppose);

        /* cherche le prochain candidat pour cette cellule dans la grille restaurée */
        __uint16_t next = next_candidat(g, last_supp->x, last_supp->y, last_supp->candidat);

        int x = last_supp->x;
        int y = last_supp->y;
        delete_supposition(last_supp);

        if (next != 0)
        {
            *x_out        = x;
            *y_out        = y;
            *candidat_out = next;
            return 1;
        }
        /* plus de candidat à ce niveau → on remonte encore */
    }
    return 0;
}

int solve(grille *g)
{
    pileSupp *supps = pileSupp_create();

    printf("\ngrille initial :\n");
    print_grille(g);

    int nb_try = 0;
    int state  = GRILLE_UNSOLVED;

    do
    {
        nb_try++;

        printf("\n📍 Step n°%d :\n", nb_try);

        state = try_to_solve(g);

        if (state == GRILLE_IMPOSSIBLE)
        {
            if (pileSupp_empty(supps))
            {
                printf("💀 la grille est de base impossible\n");
            }
            else
            {
                printf("⛔ Grille impossible en l'etat\n");

                int x_new_supp;
                int y_new_supp;
                __uint16_t candidat_new_supp;

                if (backtrack(supps, g, &x_new_supp, &y_new_supp, &candidat_new_supp) == 1)
                {
                    printf("💡 Nouvelle supposition : (%d,%d) -> %d\n", x_new_supp, y_new_supp, candidat_new_supp);
                    pileSupp_push(supps, x_new_supp, y_new_supp, candidat_new_supp, g);
                    set_supposition(g, x_new_supp, y_new_supp, candidat_new_supp);
                    state = is_solve(g);
                }
                else
                {
                    printf("💀 Grille definitivement impossible\n");
                }
            }
        }
        else if (state == GRILLE_UNSOLVED)
        {
            int x_new_supp;
            int y_new_supp;
            __uint16_t candidat_new_supp;

            if (find_best_supposition(*g, &x_new_supp, &y_new_supp, &candidat_new_supp) == 0)
            {
                printf("⛔ Grille impossible en l'etat\n");

                if (backtrack(supps, g, &x_new_supp, &y_new_supp, &candidat_new_supp) == 1)
                {
                    printf("💡 Nouvelle supposition apres backtrack : (%d,%d) -> %d\n", x_new_supp, y_new_supp, candidat_new_supp);
                    pileSupp_push(supps, x_new_supp, y_new_supp, candidat_new_supp, g);
                    set_supposition(g, x_new_supp, y_new_supp, candidat_new_supp);

                    state = is_solve(g);
                }
                else
                {
                    printf("💀 Grille definitivement impossible\n");
                    state = GRILLE_IMPOSSIBLE;
                }
            }
            else
            {
                printf("💡 Nouvelle supposition : (%d,%d) -> %d\n", x_new_supp, y_new_supp, candidat_new_supp);
                pileSupp_push(supps, x_new_supp, y_new_supp, candidat_new_supp, g);
                set_supposition(g, x_new_supp, y_new_supp, candidat_new_supp);

                state = is_solve(g);
            }
        }

        print_grille(g);
    }
    while (state == GRILLE_UNSOLVED);

    pileSupp_delete(supps);

    if(state == GRILLE_SOLVED)
    {
        printf("\n✅ Solve after %d tries : \n\n", nb_try);
        print_grille(g);
    }
    else
    {
        printf("\n❌ Impossible to solve after %d tries : \n\n", nb_try);
        print_grille(g);
    }

    return state;
}