#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>
#include <math.h>

#include "CosmicYonder.h"

#define OBJ_MAX 9

#define LARGEUR_TAB 5
#define LONGUEUR_TAB 5

#define LIGNES 5
#define COLONNES 5

int generation(int longueur, int largeur, int num_salle, int cote);
salle room;

extern int mouvementHaut(void);
extern int mouvementGauche(void);
extern int mouvementBas(void);
extern int mouvementDroite(void);

int epeePlacee = 0; // Indique si l'épée a été placée
int porteLibre = 0; 
int grandesMachinesPlacees = 0;
extern personnage perso;
extern personnage persoPast;
extern Boss boss;

extern int graine;
int entreeX;
int entreeY;

extern salle room;

unsigned int aleatoire(int min, int max) {
    // Générer une valeur pseudo-aléatoire dans l'intervalle [min, max]
    return (rand() % (max - min + 1)) + min;
}

void freeRoomCases(tile **cases, int largeur) {
    if (cases) {
        for (int i = 0; i < largeur; i++) {
            free(cases[i]);
        }
        free(cases);
    }
}

void ajouterSalle(void);

int nouvelleSalle(int longueur, int largeur, int num_salle, int cote) {
    // Réinitialisation room
    room.num = 0;
    room.largeur = 0;
    room.longueur = 0;
    room.posX = 0;
    room.posY = 0;

    if (room.cases != NULL) {
        freeRoomCases(room.cases, room.largeur); // Utiliser la fonction de libération
        room.cases = NULL;
    }

    // Génération de la nouvelle salle
    if (generation(longueur, largeur, num_salle, cote) != EXIT_SUCCESS) {
        printf("Erreur generation\n");
        return EXIT_FAILURE;
    }

    // Calcul de la position de la salle
    if (num_salle == 1) {
        room.posX = perso.posX - 2;
        room.posY = perso.posY - 2;
    } else {
        switch (cote) {
            case 0: // Haut
                room.posX = perso.posX - largeur / 2; // Centrer la salle horizontalement par rapport au joueur
                room.posY = perso.posY - longueur;    // Positionner la salle au-dessus
                break;
            case 3: // Droite
                room.posX = perso.posX + 1;           // Positionner la salle à droite
                room.posY = perso.posY - longueur / 2; // Centrer la salle verticalement
                break;
            case 2: // Bas
                room.posX = perso.posX - largeur / 2; // Centrer la salle horizontalement
                room.posY = perso.posY + 1;           // Positionner la salle en dessous
                break;
            case 1: // Gauche  
                room.posX = perso.posX - largeur;     // Positionner la salle à gauche
                room.posY = perso.posY - longueur / 2; // Centrer la salle verticalement
                break;
            default:
                break;
        }
    }

    // Vérification des limites de la carte
    if (room.posX < 0 || room.posX + room.largeur >= DIMENSION_MAP || room.posY < 0 || room.posY + room.longueur >= DIMENSION_MAP) {
        map[perso.posX][perso.posY].contenu = -1;

        switch (cote) {
            case 0:
                mouvementBas();
                break;
            case 1:
                mouvementDroite();
                break;
            case 2:
                mouvementHaut();
                break;
            case 3:
                mouvementGauche();
                break;
            default:
                break;
        }
    }

    // Ajout de la salle à la carte
    ajouterSalle();
    map[perso.posX][perso.posY].contenu = 1;

    return EXIT_SUCCESS;
}

int verifierPlacementSalle(int longueur, int largeur, int posX, int posY) {
    // Vérification des limites de la carte
    if (posX < 0 || posY < 0 || posX + largeur > DIMENSION_MAP || posY + longueur > DIMENSION_MAP) {
        return 0; // Emplacement invalide
    }

    // Vérification des collisions
    for (int i = posX; i < posX + largeur; i++) {
        for (int j = posY; j < posY + longueur; j++) {
            // Si une case n'est ni un mur (-2) ni une porte (-1) ni vide (-5), il y a collision
            if (map[i][j].contenu != -5 && map[i][j].contenu != -2 && map[i][j].contenu != -1) {
                return 0;
            }
        }
    }
    return 1; // Emplacement valide
}

int genererSalleValide(int longueurInitiale, int largeurInitiale, int num_salle, int cote) {
    int longueur = longueurInitiale;
    int largeur = largeurInitiale;

    for (int tentatives = 0; tentatives < 10; tentatives++) {
        // Réduire les dimensions si nécessaire
        if (tentatives > 0) {
            longueur = (longueur > 5) ? longueur - 1 : 5;
            largeur = (largeur > 5) ? largeur - 1 : 5;
        }

        int posX = 0;
        int posY = 0;

        // Calculer la position en fonction du cote
        switch (cote) {
            case 0: // Haut
                posX = perso.posX - largeur / 2; // Centrer horizontalement
                posY = perso.posY - longueur;    // Coller au mur du haut
                break;
            case 1: // Gauche
                posX = perso.posX - largeur;     // Coller au mur de gauche
                posY = perso.posY - longueur / 2; // Centrer verticalement
                break;
            case 2: // Bas
                posX = perso.posX - largeur / 2; // Centrer horizontalement
                posY = perso.posY + 1;           // Coller au mur du bas
                break;
            case 3: // Droite
                posX = perso.posX + 1;           // Coller au mur de droite
                posY = perso.posY - longueur / 2; // Centrer verticalement
                break;
            default:
                printf("Cote invalide pour la generation de salle.\n");
                return -1;
        }

        if (verifierPlacementSalle(longueur, largeur, posX, posY)) {
            return nouvelleSalle(longueur, largeur, num_salle, cote);
        }

    }

    return EXIT_FAILURE;
}

int actualiserMap(void) {
    for (unsigned i = 0; i < DIMENSION_MAP; ++i) {
        for (unsigned j = 0; j < DIMENSION_MAP; ++j) {
            if (i == perso.posX && j == perso.posY) {
                map[i][j].contenu = 1;
            } else if (i == persoPast.posX && j == persoPast.posY) {
                map[i][j].contenu = 0;
            }
        }
    }

    return EXIT_SUCCESS;
}

int generation(int longueur, int largeur, int num_salle, int cote) {
    // Allocation de mémoire pour la salle
    tile **p = malloc(sizeof(tile *) * largeur);
    if (p == NULL) {
        return EXIT_FAILURE;
    }

    for (int i = 0; i < largeur; i++) {
        p[i] = malloc(sizeof(tile) * longueur);
        if (p[i] == NULL) {
            for (int j = 0; j < i; j++) {
                free(p[j]);
            }
            free(p);
            return EXIT_FAILURE;
        }
    }

    // Initialisation des cases
    for (int i = 0; i < largeur; i++) {
        for (int j = 0; j < longueur; j++) {
            p[i][j].contenu = 0;
            p[i][j].mstr.hp = 0;
            p[i][j].mstr.xp = 0;
            p[i][j].mstr.morceau = 0;
            p[i][j].mstr.type = 0;
            p[i][j].spe.type = 0;
            p[i][j].spe.inv = 0;
        }
    }

    // Initialisation des portes
    int porteLibre = 0;
    int portes[4] = {0}; // Initialiser toutes les portes à 0

    // Si c'est la première salle, positionner les portes au centre des murs
    if (num_salle == 1) {
        portes[0] = 1; // Haut
        portes[1] = 1; // Gauche
        portes[2] = 1; // Bas
        portes[3] = 1; // Droite
    } else {
        portes[cote] = 1; // Activer la porte du côté spécifié

        // Générer des portes aléatoires supplémentaires (60 % de chance), sauf du côté où se trouve le joueur
        for (int i = 0; i < 4; i++) {
            if (portes[i] == 0 && i != cote && aleatoire(1, 100) <= 60) {
                portes[i] = 1;
                porteLibre++;
            }
        }
    }

    // Placement des portes
    for (int i = 0; i < 4; i++) {
        if (portes[i] == 1) {
            int al;
            switch (i) {
                case 0: // Porte haut
                    al = (num_salle == 1) ? longueur / 2 : aleatoire(1, longueur - 2);
                    p[0][al].contenu = -1;
                    if (i == cote) {
                        entreeX = al;
                        entreeY = 0;
                    }
                    break;
                case 1: // Porte gauche
                    al = (num_salle == 1) ? largeur / 2 : aleatoire(1, largeur - 2);
                    p[al][0].contenu = -1;
                    if (i == cote) {
                        entreeX = 0;
                        entreeY = al;
                    }
                    break;
                case 2: // Porte bas
                    al = (num_salle == 1) ? longueur / 2 : aleatoire(1, longueur - 2);
                    p[largeur - 1][al].contenu = -1;
                    if (i == cote) {
                        entreeX = al;
                        entreeY = longueur - 1;
                    }
                    break;
                case 3: // Porte droite
                    al = (num_salle == 1) ? largeur / 2 : aleatoire(1, largeur - 2);
                    p[al][longueur - 1].contenu = -1;
                    if (i == cote) {
                        entreeX = largeur - 1;
                        entreeY = al;
                    }
                    break;
                default:
                    break;
            }
        }
    }

    // Remplissage de la salle avec des monstres et des objets spéciaux
    int obj = 0;
    for (int i = 0; i < largeur; i++) {
        for (int j = 0; j < longueur; j++) {
            if (j == 0 || j == longueur - 1 || i == 0 || i == largeur - 1) {
                if (p[i][j].contenu != -1) {
                    p[i][j].contenu = -2; // Mur
                }
            } else if (num_salle == 1) {
                p[2][2].contenu = 1; // Contenu de la première salle
            } else if (aleatoire(1, 100) <= 40) { // 40% de chance de générer un objet ou un monstre
                if (obj < ((longueur - 2) * (largeur - 2)) / OBJ_MAX) {
                    int proba = aleatoire(1, 100); // Générer un nombre entre 1 et 100
                    if (proba <= 30) {
                        // Monstre type 1 (30 %)
                        p[i][j].contenu = 2;
                        p[i][j].mstr.hp = 80;
                        p[i][j].mstr.xp = 10;
                        p[i][j].mstr.type = 1;
                    } else if (proba <= 50) {
                        // Monstre type 2 (20 %)
                        p[i][j].contenu = 2;
                        p[i][j].mstr.hp = 180;
                        p[i][j].mstr.xp = 20;
                        p[i][j].mstr.type = 2;
                    } else if (proba <= 60) {
                        // Monstre type 3 (10 %)
                        p[i][j].contenu = 2;
                        p[i][j].mstr.hp = 260;
                        p[i][j].mstr.xp = 30;
                        p[i][j].mstr.type = 3;
                    } else if (proba <= 80) {
                        // Coffre (20 %)
                        p[i][j].contenu = 3;
                        p[i][j].spe.type = 1;
                    } else if (proba <= 95) {
                        // Machine (15 %)
                        p[i][j].contenu = 3;
                        p[i][j].spe.type = 2;
                    } else {
                        // Grande Machine (5 %)
                        if (grandesMachinesPlacees < 3) {
                            p[i][j].contenu = 3;
                            p[i][j].spe.type = 3; // Grande machine
                            grandesMachinesPlacees++;
                        } else {
                            p[i][j].contenu = 2;
                            p[i][j].mstr.hp = 260;
                            p[i][j].mstr.xp = 30;
                            p[i][j].mstr.type = 3; // Remplacer par un monstre type 3
                        }
                    }
                    obj++;
                }
            } else if (!epeePlacee && num_salle != 1 && (aleatoire(1, 100) <= 5)) { // 5 % de chance pour l'épée
                p[i][j].contenu = 3;
                p[i][j].spe.type = 8; // Contenu pour l'épée
                epeePlacee = 1; // Marquer que l'épée a été placée
            }
        }
    }

    // Si l'épée n'est pas encore placée, forcer son placement en salle 6
    if (!epeePlacee && num_salle == 6) {
        for (int i = 1; i < largeur - 1; i++) {
            for (int j = 1; j < longueur - 1; j++) {
                if (p[i][j].contenu == 0) {
                    p[i][j].contenu = 3;
                    p[i][j].spe.type = 8; // Contenu pour l'épée
                    epeePlacee = 1;
                    break;
                }
            }
            if (epeePlacee) break;
        }
    }

    // Sauvegarde des informations de la salle
    room.num = num_salle;
    room.largeur = largeur;
    room.longueur = longueur;
    room.cases = p;

    return EXIT_SUCCESS;
}

int genererSalleBoss(int cote) {
    int largeur = 11;
    int longueur = 7;
    int posX = perso.posX - largeur / 2;  // Centrer horizontalement
    int posY = perso.posY - longueur;    // Placer au-dessus du joueur

    // Vérifier si la salle peut être générée
    if (!verifierPlacementSalle(longueur, largeur, posX, posY)) {
        SDL_Log("Erreur : Emplacement invalide pour la salle du boss.");
        return 0; // Retourner 0 si la salle ne peut pas être générée
    }

    // Générer une salle vide pour le boss
    for (int i = posX; i < posX + largeur; i++) {
        for (int j = posY; j < posY + longueur; j++) {
            if (i == posX || i == posX + largeur - 1 || j == posY || j == posY + longueur - 1) {
                map[i][j].contenu = -2; // Mur
            } else {
                map[i][j].contenu = 0; // Vide
            }
        }
    }

    // Placer la porte du boss
    map[perso.posX][perso.posY - 1].contenu = 4; // Porte de boss

    // Placer le boss dans un rectangle 2x3 au centre supérieur
    int bossStartX = posX + largeur / 2 - 1;
    int bossStartY = posY + 1; // Une ligne en dessous du mur supérieur
    for (int i = bossStartX; i < bossStartX + 3; i++) {
        for (int j = bossStartY; j < bossStartY + 2; j++) {
            map[i][j].contenu = 2;           // Type boss
            map[i][j].mstr.type = 4;
            map[i][j].mstr.hp = 9999;
            map[i][j].mstr.morceau = (j - bossStartY) * 3 + (i - bossStartX) ; // Identifier chaque morceau
            if (map[i][j].mstr.morceau == 1) { //Initialisation des info du boss
                boss.hp = BOSSSHP;
                boss.max_hp = 1000;
                boss.x = i;
                boss.y = j;
                boss.invulnerable = SDL_FALSE;
                boss.invuln_timer = 0;
            }
        }
    }

    return 1; // Indiquer que la salle a été générée
}

void ajouterSalle(void) {
    int n = 0, m = 0;

    for (unsigned int i = room.posX; i < room.posX + room.largeur; i++) {
        for (unsigned int j = room.posY; j < room.posY + room.longueur; j++) {
            if (i >= DIMENSION_MAP || j >= DIMENSION_MAP) {
                continue; // Ne pas accéder à la mémoire hors limites
            }

            // Transformation conditionnelle : si map[i][j] est un mur (-2) et room.cases[n][m] est une porte (-1), ne rien faire
            if (map[i][j].contenu == -2 && room.cases[n][m].contenu == -1) {
                m++;
                continue;
            }

            // Sinon, copier les données de room.cases dans map
            map[i][j].contenu = room.cases[n][m].contenu;
            map[i][j].mstr.hp = room.cases[n][m].mstr.hp;
            map[i][j].mstr.xp = room.cases[n][m].mstr.xp;
            map[i][j].mstr.morceau = room.cases[n][m].mstr.morceau;
            map[i][j].mstr.type = room.cases[n][m].mstr.type;
            map[i][j].spe.type = room.cases[n][m].spe.type;
            map[i][j].spe.inv = room.cases[n][m].spe.inv;

            m++;
        }
        m = 0;
        n++;
    }
}
