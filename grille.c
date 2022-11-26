#include "grille.h"


grille * init_grille(){
    grille* grilleREF = malloc(sizeof(grille));
    grilleREF->grille = malloc(sizeof(int)*81);
    grilleREF->nb_element_manquant = 81;
    return grilleREF;
}

void free_grille(grille * grilleREF){
    free(grilleREF->grille);
    free(grilleREF);
}

void set_in_grille(grille * grilleREF, int x, int y, int value){
    if(get_in_grille(grilleREF,x,y)== 0 && value != 0){
        grilleREF->nb_element_manquant = grilleREF->nb_element_manquant - 1;
    }
    if(get_in_grille(grilleREF,x,y)!= 0 && value == 0){
        grilleREF->nb_element_manquant = grilleREF->nb_element_manquant + 1;
    }
    grilleREF->grille[y*9 + x] = value;
}

int get_in_grille(grille * grilleREF, int x, int y){
    return grilleREF->grille[y*9 + x];
}

void print_grille(grille * grilleREF){
    printf("\n\n");
    for(int i=0; i<9;i++){
        for(int j = 0 ; j<9;j++){
            int tmp = get_in_grille(grilleREF,i,j);
            if(tmp == 0){
                printf("- ");
            }
            else{
                printf("%d ",tmp);
            }
        }
        printf("\n");
    }
}