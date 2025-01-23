#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>

#include "CosmicYonder.h"


/*
	Windows : gcc src\*.c -o bin\progMain.exe -I include -L lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_mixer -mwindows
	Windows sans terminal qui ouvre : gcc src/generation.c -o bin\progGeneration -mwindows
	Linux : gcc generation.c -o progGeneration

	Flags render
	SDL_RENDERER_SOFTWARE
	SDL_RENDERER_ACCELERATED
	SDL_RENDERER_PRESENTVSYNC
	SDL_RENDERER_TARGETTEXTURE
*/

//message erreur

/*/
typedef struct personnage {
    int direction; //haut = 1, gauche = 2, bas = 3, droite = 4
    int posX;
    int posY;
    int frameAnimation;
} personnage;

typedef struct salle {
    int largeur;
    int longueur;
    int posX;
    int posY;
    int ** cases;
} salle ;
*/

extern personnage perso;
extern personnage persoPast;
extern salle room;
extern int nouvelleSalle(int longueur, int largeur, int num_salle, int cote);
int inv[7];
extern int num_salle;

int Xcamera = (DIMENSION_MAP / 2) - 7;
int Ycamera = (DIMENSION_MAP / 2) - 4;
tile **map;

int liberationMap(void) {
    for (unsigned i = 0; i < DIMENSION_MAP; ++i) {
        free(map[i]);
    }
    free(map);
    return EXIT_SUCCESS;
}

int creeMap(void) {
    perso.posX = (DIMENSION_MAP / 2);
    perso.posY = (DIMENSION_MAP / 2);
    perso.direction = 3;
    perso.frameAnimation = 0;
    perso.hp = 3;
    perso.hpMax = 3;
    perso.inv = inv;
    perso.lvl = 1;
    perso.xp = 0;
    perso.xpMax = 90;
    perso.sword_damage = 50; // Dégâts de base de l'épée
    perso.gun_damage = 20;   // Dégâts de base du pistolet


    // Allocation de mémoire pour map
    map = malloc(sizeof(tile*) * DIMENSION_MAP);
    if (map == NULL) {
        printf("Échec de l'allocation\n");
        return EXIT_FAILURE;
    }

    for (unsigned i = 0; i < DIMENSION_MAP; ++i) {
        map[i] = malloc(sizeof(tile) * DIMENSION_MAP);
        if (map[i] == NULL) {
            printf("Échec de l'allocation\n");
            // Libération de la mémoire allouée avant de retourner l'erreur
            for (unsigned j = 0; j < i; ++j) {
                free(map[j]);
            }
            free(map);
            return EXIT_FAILURE;
        }
    }

    for (unsigned i = 0; i < DIMENSION_MAP; ++i) {
        for (unsigned j = 0; j < DIMENSION_MAP; ++j) {
            map[i][j].contenu = -5;
            if (i == perso.posX && j == perso.posY) {
                map[i][j].contenu = 1;
            }
        }
    }

    if (nouvelleSalle(5, 5, 1, 0) != EXIT_SUCCESS) {
        printf("Erreur generation salle 1");
        liberationMap(); // Libération de la mémoire en cas d'échec
        return EXIT_FAILURE;
    }

    num_salle = 2;

    return EXIT_SUCCESS;
}

extern int actualiserMap(void);


