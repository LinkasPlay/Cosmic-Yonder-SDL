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
	Linux : gcc renduCase.c $(sdl2-config __cflags --libs) -o progRenduCase

	Flags render
	SDL_RENDERER_SOFTWARE
	SDL_RENDERER_ACCELERATED
	SDL_RENDERER_PRESENTVSYNC
	SDL_RENDERER_TARGETTEXTURE
*/

tile contenuCase;
SDL_Rect Case;

extern personnage perso;


void SDL_ExitWithError(const char *message);

void SDL_LimitFPS(unsigned int limit);

int camera(int argc, char **argv);

extern int mouvementHaut(void);
extern int mouvementGauche(void);
extern int mouvementBas(void);
extern int mouvementDroite(void);
extern int attaqueEpee(void);
extern void gameOver();

extern unsigned int timer_start;
extern unsigned int start_time;
extern int machineFuite;
extern int graine;

extern int texture( int argc, char **argv);
extern int creeMap(void);
extern int actualiserMap(void);

extern int nouvelleSalle(int longueur, int largeur, int num_salle, int cote);
extern salle room;
extern Boss boss;

extern SDL_bool bossPresent;

extern char inventaireIcons[7][64];

TTF_Font *fontUI = NULL;

extern Mix_Music *musique_acceleree;

void afficherBarreDeVieBoss(SDL_Renderer *renderer, TTF_Font *font, int hp, int hpMax) {
    // Dimensions et position de la barre
    int barreLargeur = 300;  // Largeur totale de la barre
    int barreHauteur = 20;   // Hauteur totale de la barre
    int barreX = (WINDOW_WIDTH - barreLargeur) / 2;  // Centre horizontal
    int barreY = 50;         // Position verticale

    // Calculer la largeur du rectangle rouge représentant la vie
    float ratioVie = (float)hp / (float)hpMax; // Ratio de vie (0.0 à 1.0)
    if (ratioVie < 0.0f) ratioVie = 0.0f;      // Empêcher les ratios négatifs
    if (ratioVie > 1.0f) ratioVie = 1.0f;      // Empêcher les ratios supérieurs à 1.0
    int vieLargeur = (int)(ratioVie * barreLargeur);

    // Dessiner les contours blancs
    SDL_Rect contourRect = {barreX - 2, barreY - 2, barreLargeur + 4, barreHauteur + 4};
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);  // Couleur blanche
    SDL_RenderFillRect(renderer, &contourRect);

    // Dessiner l'intérieur noir
    SDL_Rect fondRect = {barreX, barreY, barreLargeur, barreHauteur};
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  // Couleur noire
    SDL_RenderFillRect(renderer, &fondRect);

    // Dessiner le rectangle rouge représentant la vie restante
    SDL_Rect vieRect = {barreX, barreY, vieLargeur, barreHauteur};
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);  // Couleur rouge
    SDL_RenderFillRect(renderer, &vieRect);

    // Afficher le texte "Boss" juste au-dessus de la barre
    SDL_Surface *texteSurface = TTF_RenderText_Solid(font, "Boss", (SDL_Color){255, 255, 255, 255});  // Blanc
    SDL_Texture *texteTexture = SDL_CreateTextureFromSurface(renderer, texteSurface);
    SDL_FreeSurface(texteSurface);

    if (texteTexture) {
        // Dimensions et position du texte
        int texteLargeur, texteHauteur;
        SDL_QueryTexture(texteTexture, NULL, NULL, &texteLargeur, &texteHauteur);
        SDL_Rect texteRect = {
            .x = barreX + (barreLargeur - texteLargeur) / 2,
            .y = barreY - texteHauteur - 10,
            .w = texteLargeur,
            .h = texteHauteur
        };
        SDL_RenderCopy(renderer, texteTexture, NULL, &texteRect);
        SDL_DestroyTexture(texteTexture);
    }
}

void afficherInterface(SDL_Renderer *renderer, TTF_Font *font, int selectedSlot) {
    SDL_Surface *image = NULL;
    SDL_Texture *texture = NULL;
    SDL_Rect uiRect;

    // --- Effacer l'ancien affichage du Timer ---
    SDL_Rect timerEraseRect = {WINDOW_WIDTH - 100, 20, 80, 40};
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  // Noir
    SDL_RenderFillRect(renderer, &timerEraseRect);  // Effacer l'ancien timer

    // --- Effacer l'ancien affichage de la graine ---
    SDL_Rect graineEraseRect = {WINDOW_WIDTH - 150, 70, 140, 30};
    SDL_RenderFillRect(renderer, &graineEraseRect);  // Effacer l'ancien texte de la graine

    // --- Effacer l'ancien affichage de l'inventaire ---
    int nbCases = 7;  // Nombre de cases dans l'inventaire
    int caseSize = 64;  // Taille d'une case
    int startX = (WINDOW_WIDTH - (nbCases * caseSize)) / 2;  // Centrer l'inventaire
    SDL_Rect inventaireEraseRect = {startX, WINDOW_HEIGHT - 100, nbCases * (caseSize + 10), caseSize};
    SDL_RenderFillRect(renderer, &inventaireEraseRect);  // Effacer l'inventaire

    // --- Effacer l'ancien affichage du niveau et XP ---
    SDL_Rect xpEraseRect = {20, 70, 200, 30};
    SDL_RenderFillRect(renderer, &xpEraseRect);  // Effacer l'affichage du niveau et XP

    // --- Affichage des points de vie (HP) ---
    for (int i = 0; i < perso.hpMax; i++) {
        if (i < perso.hp) {
            image = SDL_LoadBMP("src/image/hp.bmp");  // Coeur plein
        } else {
            image = SDL_LoadBMP("src/image/nhp.bmp");  // Coeur vide
        }

        if (!image) {
            SDL_Log("Erreur de chargement de l'image de coeur : %s", SDL_GetError());
            return;
        }

        texture = SDL_CreateTextureFromSurface(renderer, image);
        SDL_FreeSurface(image);

        uiRect.x = 20 + i * 40;
        uiRect.y = 20;
        uiRect.w = 32;
        uiRect.h = 32;

        SDL_RenderCopy(renderer, texture, NULL, &uiRect);
        SDL_DestroyTexture(texture);
    }

    // --- Affichage du niveau et XP ---
    char xpText[100];
    sprintf(xpText, "Niveau %d : XP %d/%d", perso.lvl, perso.xp, (int)(100 * (perso.lvl * 0.9)));
    SDL_Surface *xpSurface = TTF_RenderText_Solid(font, xpText, (SDL_Color){255, 255, 255, 255});
    SDL_Texture *xpTexture = SDL_CreateTextureFromSurface(renderer, xpSurface);
    SDL_FreeSurface(xpSurface);
    
    uiRect.x = 20;
    uiRect.y = 70;
    uiRect.w = 200;
    uiRect.h = 30;
    SDL_RenderCopy(renderer, xpTexture, NULL, &uiRect);
    SDL_DestroyTexture(xpTexture);

    // --- Affichage du Timer ---
    unsigned int elapsed_time = SDL_GetTicks() - start_time;
    unsigned int remaining_time = (timer_start > elapsed_time) ? timer_start - elapsed_time : 0;

    if (remaining_time == 0) {
        gameOver();
        return;
    }

    char timerText[10];
    int minutes = remaining_time / 60000;
    int seconds = (remaining_time % 60000) / 1000;
    sprintf(timerText, "%02d:%02d", minutes, seconds);

    SDL_Surface *timerSurface = TTF_RenderText_Solid(font, timerText, (SDL_Color){255, 255, 255, 255});
    SDL_Texture *timerTexture = SDL_CreateTextureFromSurface(renderer, timerSurface);
    SDL_FreeSurface(timerSurface);

    uiRect.x = WINDOW_WIDTH - 100;
    uiRect.y = 20;
    uiRect.w = 80;
    uiRect.h = 40;
    SDL_RenderCopy(renderer, timerTexture, NULL, &uiRect);
    SDL_DestroyTexture(timerTexture);

    // --- Affichage de la graine ---
    char graineText[50];
    sprintf(graineText, "Graine : %d", graine);
    SDL_Surface *graineSurface = TTF_RenderText_Solid(font, graineText, (SDL_Color){255, 255, 255, 255});
    SDL_Texture *graineTexture = SDL_CreateTextureFromSurface(renderer, graineSurface);
    SDL_FreeSurface(graineSurface);

    uiRect.x = WINDOW_WIDTH - 150;
    uiRect.y = 70;
    uiRect.w = 140;
    uiRect.h = 30;
    SDL_RenderCopy(renderer, graineTexture, NULL, &uiRect);
    SDL_DestroyTexture(graineTexture);

    // --- Affichage du nombre de grandes machines réparées (en bas de l'écran) ---
    char machineText[50];
    sprintf(machineText, "Grandes machines : %d", machineFuite);
    SDL_Surface *machineSurface = TTF_RenderText_Solid(font, machineText, (SDL_Color){255, 255, 255, 255});
    SDL_Texture *machineTexture = SDL_CreateTextureFromSurface(renderer, machineSurface);
    SDL_FreeSurface(machineSurface);

    uiRect.x = WINDOW_WIDTH - 200;
    uiRect.y = WINDOW_HEIGHT - 50;  // Position en bas de l'écran
    uiRect.w = 190;
    uiRect.h = 30;
    SDL_RenderCopy(renderer, machineTexture, NULL, &uiRect);
    SDL_DestroyTexture(machineTexture);

    // --- Affichage de l'inventaire ---
    for (int i = 0; i < nbCases; i++) {
        // Charger l'icône correspondant à l'inventaire
        image = SDL_LoadBMP(inventaireIcons[i]);
        if (!image) {
            SDL_Log("Erreur de chargement de l'image d'inventaire pour l'emplacement %d : %s", i, SDL_GetError());
            return;
        }
        texture = SDL_CreateTextureFromSurface(renderer, image);
        SDL_FreeSurface(image);

        // Définir la position et la taille de la case d'inventaire
        uiRect.x = startX + i * (caseSize + 10);
        uiRect.y = WINDOW_HEIGHT - 100;
        uiRect.w = caseSize;
        uiRect.h = caseSize;

        SDL_RenderCopy(renderer, texture, NULL, &uiRect);
        SDL_DestroyTexture(texture);

        // Affichage de la flèche de sélection
        if (i == selectedSlot) {
            SDL_Surface *arrowSurface = SDL_LoadBMP("src/image/fleche.bmp");
            SDL_Texture *arrowTexture = SDL_CreateTextureFromSurface(renderer, arrowSurface);
            SDL_FreeSurface(arrowSurface);

            SDL_Rect arrowRect;
            arrowRect.x = uiRect.x + (caseSize / 2) - 16;
            arrowRect.y = uiRect.y + caseSize + 5;
            arrowRect.w = 32;
            arrowRect.h = 32;

            SDL_RenderCopy(renderer, arrowTexture, NULL, &arrowRect);
            SDL_DestroyTexture(arrowTexture);
        }
    }

     // --- Affichage de la barre de vie du boss si le boss est présent ---
    if (bossPresent) {
        afficherBarreDeVieBoss(renderer, font, boss.hp, boss.max_hp);  // boss.hp et boss.hpMax doivent être accessibles globalement
    }

    SDL_RenderPresent(renderer);  // Mise à jour de l'interface
}

// afficher les images de la camera
int camera(int argc, char **argv){
    int Xcase, Ycase;

    // Parcourir la portion de la carte visible à l'écran
    for (Xcase = 1; Xcase < ((WINDOW_WIDTH / 100) - 1); Xcase++) {
        for (Ycase = 1; Ycase < ((WINDOW_HEIGHT / 100) - 1); Ycase++) {

            // Récupérer le contenu de la case à la position caméra
            contenuCase = map[Xcamera + Xcase][Ycamera + Ycase];

            // Définir les coordonnées de la case
            Case.x = Xcase * 100;
            Case.y = Ycase * 100;
            Case.w = WINDOW_WIDTH / (WINDOW_WIDTH / 100);
            Case.h = WINDOW_HEIGHT / (WINDOW_HEIGHT / 100);

            // Dessiner la case (rectangle)
            if (SDL_RenderDrawRect(renderer, &Case) != 0) {
                SDL_ExitWithError("Impossible de dessiner une case");
            }

            // Dessiner la texture associée à la case
            if (texture(argc, argv) != EXIT_SUCCESS) {
                SDL_ExitWithError("Fonction texture interrompue");
            }
        }
    }

    // Mise à jour de l'animation du personnage
    perso.frameAnimation++;

    // Afficher le rendu mis à jour
    SDL_RenderPresent(renderer);

    return EXIT_SUCCESS;
}

//message erreur
void SDL_ExitWithError(const char *message){
	SDL_Log("ERREUR : %s > %s\n",message, SDL_GetError());
	
	/* PROBLEME COMPILATION AUDIO !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! /
	Mix_CloseAudio();
	/* PROBLEME COMPILATION AUDIO !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

	SDL_Quit();
	exit(EXIT_FAILURE);
}

// limite fps
void SDL_LimitFPS(unsigned int limit){
	unsigned int ticks = SDL_GetTicks();

	if (limit< ticks) {
		return;
	}
	else if (limit > ticks + FPS_LIMIT){
		SDL_Delay(FPS_LIMIT);
	}
	else {
		SDL_Delay(limit - ticks);
	}
}
