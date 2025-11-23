#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <limits.h>

#define TAILLE_GRILLE 10
#define INFINI INT_MAX

typedef struct {
    int x;
    int y;
} Position;

typedef struct Noeud {
    Position pos;
    int g;
    int h;
    int f;
    struct Noeud* parent;
} Noeud;

typedef struct {
    Noeud** noeuds;
    int taille;
    int capacite;
} ListeNoeuds;

int grille[TAILLE_GRILLE][TAILLE_GRILLE];

ListeNoeuds* creer_liste() {
    ListeNoeuds* liste = (ListeNoeuds*)malloc(sizeof(ListeNoeuds));
    liste->capacite = 10;
    liste->taille = 0;
    liste->noeuds = (Noeud**)malloc(sizeof(Noeud*) * liste->capacite);
    return liste;
}

void ajouter_noeud(ListeNoeuds* liste, Noeud* noeud) {
    if (liste->taille >= liste->capacite) {
        liste->capacite *= 2;
        Noeud** nouveau = (Noeud**)realloc(liste->noeuds, sizeof(Noeud*) * liste->capacite);
        if (nouveau == NULL) {
            fprintf(stderr, "Erreur d'allocation memoire\n");
            exit(1);
        }
        liste->noeuds = nouveau;
    }
    liste->noeuds[liste->taille++] = noeud;
}

void supprimer_noeud(ListeNoeuds* liste, int index) {
    if (index < 0 || index >= liste->taille) return;
    for (int i = index; i < liste->taille - 1; i++) {
        liste->noeuds[i] = liste->noeuds[i + 1];
    }
    liste->taille--;
}

int est_dans_liste(ListeNoeuds* liste, Position pos) {
    for (int i = 0; i < liste->taille; i++) {
        if (liste->noeuds[i]->pos.x == pos.x && liste->noeuds[i]->pos.y == pos.y) {
            return i;
        }
    }
    return -1;
}

int heuristique(Position a, Position b) {
    return abs(a.x - b.x) + abs(a.y - b.y);
}

int est_valide(Position pos) {
    return pos.x >= 0 && pos.x < TAILLE_GRILLE && 
           pos.y >= 0 && pos.y < TAILLE_GRILLE && 
           grille[pos.x][pos.y] == 0;
}

Noeud* creer_noeud(Position pos, int g, int h, Noeud* parent) {
    Noeud* noeud = (Noeud*)malloc(sizeof(Noeud));
    noeud->pos = pos;
    noeud->g = g;
    noeud->h = h;
    noeud->f = g + h;
    noeud->parent = parent;
    return noeud;
}

int trouver_min_f(ListeNoeuds* liste) {
    if (liste->taille == 0) {
        return -1;
    }
    int min_index = 0;
    int min_f = liste->noeuds[0]->f;
    for (int i = 1; i < liste->taille; i++) {
        if (liste->noeuds[i]->f < min_f) {
            min_f = liste->noeuds[i]->f;
            min_index = i;
        }
    }
    return min_index;
}

void afficher_chemin(Noeud* noeud) {
    if (noeud == NULL) return;
    afficher_chemin(noeud->parent);
    printf("(%d, %d) ", noeud->pos.x, noeud->pos.y);
}

void liberer_liste(ListeNoeuds* liste) {
    for (int i = 0; i < liste->taille; i++) {
        free(liste->noeuds[i]);
    }
    free(liste->noeuds);
    free(liste);
}

int a_etoile(Position depart, Position arrivee) {
    ListeNoeuds* liste_ouverte = creer_liste();
    ListeNoeuds* liste_fermee = creer_liste();
    
    Noeud* noeud_depart = creer_noeud(depart, 0, heuristique(depart, arrivee), NULL);
    ajouter_noeud(liste_ouverte, noeud_depart);
    
    Position directions[4] = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};
    
    while (liste_ouverte->taille > 0) {
        int index_courant = trouver_min_f(liste_ouverte);
        Noeud* noeud_courant = liste_ouverte->noeuds[index_courant];
        
        if (noeud_courant->pos.x == arrivee.x && noeud_courant->pos.y == arrivee.y) {
            printf("Chemin trouve: ");
            afficher_chemin(noeud_courant);
            printf("\n");
            printf("Cout total: %d\n", noeud_courant->g);
            
            liberer_liste(liste_ouverte);
            liberer_liste(liste_fermee);
            return 1;
        }
        
        supprimer_noeud(liste_ouverte, index_courant);
        ajouter_noeud(liste_fermee, noeud_courant);
        
        for (int i = 0; i < 4; i++) {
            Position nouvelle_pos = {
                noeud_courant->pos.x + directions[i].x,
                noeud_courant->pos.y + directions[i].y
            };
            
            if (!est_valide(nouvelle_pos)) continue;
            if (est_dans_liste(liste_fermee, nouvelle_pos) != -1) continue;
            
            int nouveau_g = noeud_courant->g + 1;
            int index_ouverte = est_dans_liste(liste_ouverte, nouvelle_pos);
            
            if (index_ouverte == -1) {
                Noeud* nouveau_noeud = creer_noeud(
                    nouvelle_pos, 
                    nouveau_g, 
                    heuristique(nouvelle_pos, arrivee),
                    noeud_courant
                );
                ajouter_noeud(liste_ouverte, nouveau_noeud);
            } else {
                if (nouveau_g < liste_ouverte->noeuds[index_ouverte]->g) {
                    liste_ouverte->noeuds[index_ouverte]->g = nouveau_g;
                    liste_ouverte->noeuds[index_ouverte]->f = nouveau_g + liste_ouverte->noeuds[index_ouverte]->h;
                    liste_ouverte->noeuds[index_ouverte]->parent = noeud_courant;
                }
            }
        }
    }
    
    printf("Aucun chemin trouve!\n");
    liberer_liste(liste_ouverte);
    liberer_liste(liste_fermee);
    return 0;
}

void initialiser_grille() {
    for (int i = 0; i < TAILLE_GRILLE; i++) {
        for (int j = 0; j < TAILLE_GRILLE; j++) {
            grille[i][j] = 0;
        }
    }
}

void afficher_grille(Position depart, Position arrivee) {
    printf("\nGrille (0 = libre, 1 = obstacle, D = depart, A = arrivee):\n");
    for (int i = 0; i < TAILLE_GRILLE; i++) {
        for (int j = 0; j < TAILLE_GRILLE; j++) {
            if (i == depart.x && j == depart.y) {
                printf("D ");
            } else if (i == arrivee.x && j == arrivee.y) {
                printf("A ");
            } else if (grille[i][j] == 1) {
                printf("1 ");
            } else {
                printf("0 ");
            }
        }
        printf("\n");
    }
    printf("\n");
}

int main() {
    printf("=== Test de l'algorithme A* ===\n\n");
    
    initialiser_grille();
    
    grille[1][1] = 1;
    grille[1][2] = 1;
    grille[1][3] = 1;
    grille[2][3] = 1;
    grille[3][3] = 1;
    grille[4][3] = 1;
    grille[5][3] = 1;
    grille[6][3] = 1;
    
    Position depart = {0, 0};
    Position arrivee = {9, 9};
    
    afficher_grille(depart, arrivee);
    
    printf("Recherche du chemin de (%d, %d) à (%d, %d)...\n\n", 
           depart.x, depart.y, arrivee.x, arrivee.y);
    
    a_etoile(depart, arrivee);
    
    printf("\n=== Test 2: Sans obstacles ===\n");
    initialiser_grille();
    depart.x = 0;
    depart.y = 0;
    arrivee.x = 5;
    arrivee.y = 5;
    
    afficher_grille(depart, arrivee);
    printf("Recherche du chemin de (%d, %d) à (%d, %d)...\n\n", 
           depart.x, depart.y, arrivee.x, arrivee.y);
    a_etoile(depart, arrivee);
    
    printf("\n=== Test 3: Chemin impossible ===\n");
    initialiser_grille();
    for (int i = 0; i < TAILLE_GRILLE; i++) {
        grille[5][i] = 1;
    }
    depart.x = 0;
    depart.y = 0;
    arrivee.x = 9;
    arrivee.y = 9;
    
    afficher_grille(depart, arrivee);
    printf("Recherche du chemin de (%d, %d) à (%d, %d)...\n\n", 
           depart.x, depart.y, arrivee.x, arrivee.y);
    a_etoile(depart, arrivee);
    
    printf("\n=== Fin des tests ===\n");
    
    return 0;
}
