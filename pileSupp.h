#ifndef PILESUPP_H
#define PILESUPP_H

#include <stdint.h>
#include "grille.h"  /* doit définir le type grille */

/* ── Structures ─────────────────────────────────────────────────────────── */

typedef struct _supposition
{
    int             x;
    int             y;
    __uint16_t      candidat;
    grille         *grille_before_suppose;  /* allouée dynamiquement au push */
} supposition;

typedef struct _maillonSupp
{
    supposition         *supp;
    struct _maillonSupp *next;
} maillonSupp;

typedef struct _pileSupp
{
    maillonSupp *top;
} pileSupp;

/* ── Prototypes ──────────────────────────────────────────────────────────── */

/* Crée et retourne une pile vide (NULL en cas d'erreur) */
pileSupp    *pileSupp_create(void);

/* Retourne 1 si la pile est vide, 0 sinon */
int          pileSupp_empty(pileSupp *pile);

/* Empile une supposition. La grille est allouée et copiée en interne.
   Retourne 0 en cas de succès, -1 en cas d'erreur. */
int          pileSupp_push(pileSupp *pile, int x, int y,
                           __uint16_t candidat, grille *grille_before);

/* Dépile et retourne la supposition au sommet.
   Retourne NULL si la pile est vide.
   Utiliser delete_supposition() pour libérer la supposition retournée. */
supposition *pileSupp_pop(pileSupp *pile);

/* Consulte le sommet sans dépiler (NULL si vide) */
supposition *pileSupp_peek(pileSupp *pile);

/* Libère une supposition et sa grille (à utiliser après pileSupp_pop) */
void         delete_supposition(supposition *supp);

/* Vide la pile et libère tous les maillons, suppositions et grilles */
void         pileSupp_clear(pileSupp *pile);

/* Vide la pile et libère également la structure pileSupp elle-même */
void         pileSupp_delete(pileSupp *pile);

#endif /* PILESUPP_H */