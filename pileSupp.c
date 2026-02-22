#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pileSupp.h"

pileSupp *pileSupp_create(void)
{
    pileSupp *pile = malloc(sizeof(pileSupp));
    if (!pile)
    {
        fprintf(stderr, "Erreur malloc pileSupp\n");
        return NULL;
    }
    pile->top = NULL;
    return pile;
}

int pileSupp_empty(pileSupp *pile)
{
    return pile == NULL || pile->top == NULL;
}

int pileSupp_push(pileSupp *pile, int x, int y,
                  __uint16_t candidat, grille *grille_before)
{
    if (!pile)
        return -1;

    grille *grille_copy = malloc(sizeof(grille));
    if (!grille_copy)
    {
        fprintf(stderr, "Erreur malloc grille (push)\n");
        return -1;
    }
    copy_grille(grille_copy, grille_before);

    supposition *supp = malloc(sizeof(supposition));
    if (!supp)
    {
        fprintf(stderr, "Erreur malloc supposition\n");
        free(grille_copy);
        return -1;
    }
    supp->x                     = x;
    supp->y                     = y;
    supp->candidat              = candidat;
    supp->grille_before_suppose = grille_copy;

    maillonSupp *maillon = malloc(sizeof(maillonSupp));
    if (!maillon)
    {
        fprintf(stderr, "Erreur malloc maillonSupp\n");
        free(grille_copy);
        free(supp);
        return -1;
    }
    maillon->supp = supp;
    maillon->next = pile->top;
    pile->top     = maillon;

    return 0;
}

supposition *pileSupp_pop(pileSupp *pile)
{
    if (pileSupp_empty(pile))
        return NULL;

    maillonSupp *maillon = pile->top;
    supposition *supp    = maillon->supp;

    pile->top = maillon->next;
    free(maillon);

    return supp;
}

supposition *pileSupp_peek(pileSupp *pile)
{
    if (pileSupp_empty(pile))
        return NULL;
    return pile->top->supp;
}

void delete_supposition(supposition *supp)
{
    if (!supp)
        return;
    delete_grille(supp->grille_before_suppose);
    free(supp);
}

void pileSupp_clear(pileSupp *pile)
{
    if (!pile)
        return;

    maillonSupp *cur = pile->top;
    while (cur)
    {
        maillonSupp *next = cur->next;
        delete_supposition(cur->supp);
        free(cur);
        cur = next;
    }
    pile->top = NULL;
}

void pileSupp_delete(pileSupp *pile)
{
    if (!pile)
        return;
    pileSupp_clear(pile);
    free(pile);
}