# Sudoku Solver

Un solveur de sudoku écrit en C qui charge une grille depuis une image PNG et la résout par propagation de contraintes et backtracking.

## Fonctionnalités

- 📷 Chargement d'une grille depuis une image PNG via OCR
- 🔄 Propagation de contraintes pour résoudre sans supposition quand c'est possible
- 🎲 Backtracking intelligent avec **heuristique + degré** quand une supposition est nécessaire
- 🖥️ Affichage coloré en terminal montrant l'état de chaque cellule :
  - 🟢 **Vert** — valeurs initiales (données par la grille)
  - 🔵 **Cyan** — valeurs déduites par propagation de contraintes
  - 🟣 **Magenta** — valeurs posées par supposition
  - ⬜ **Blanc** — cellules vides avec leurs candidats affichés en exposant


## Utilisation

```bash
./solver <nom_fichier_png>
```

### Exemples

```bash
./solver example/diabolical_1.png
./solver example/test_detection1.png
```

## Compilation

```bash
make
```

Ou manuellement :

```bash
gcc -o solver main.c grille.c solver.c pileSupp.c ocr_sudoku.c -lm
```

> Adapter les flags de compilation selon les dépendances de ta bibliothèque OCR.

## Fonctionnement

### 1. Chargement
L'image PNG est analysée par `ocr_sudoku` qui extrait les valeurs initiales et remplit la grille.

### 2. Propagation de contraintes (`try_to_solve`)
Parcourt toutes les cellules vides. Quand une cellule n'a qu'un seul candidat possible, elle est remplie immédiatement. `set_cell` propage récursivement ce changement à tous les voisins (ligne, colonne, carré) qui deviennent à leur tour à candidat unique.

### 3. Supposition (`find_best_supposition`)
Quand aucune cellule ne peut être déduite, le solveur choisit la meilleure cellule à supposer grâce à deux heuristiques :
- **MRV** (Minimum Remaining Values) — préfère la cellule avec le moins de candidats
- **Degré** — à candidats égaux, préfère la cellule avec le plus de voisins vides dans sa ligne, colonne et carré

### 4. Backtracking (`backtrack`)
Si une contradiction est atteinte (une cellule n'a plus de candidat), le solveur dépile la dernière supposition, restaure l'état de la grille sauvegardé, et tente le candidat suivant pour cette cellule. S'il n'en reste plus, il remonte d'un niveau supplémentaire.

## États des cellules

| État                    | Signification                                  |
|-------------------------|------------------------------------------------|
| `CELL_UNSET`            | Cellule vide, pas encore remplie               |
| `CELL_INITIAL_SET`      | Valeur donnée par la grille originale          |
| `CELL_DEDUCTION_SET`    | Remplie par propagation de contraintes         |
| `CELL_SUPPOSITION_SET`  | Remplie par une supposition                    |
