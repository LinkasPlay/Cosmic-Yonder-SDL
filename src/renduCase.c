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

extern int texture( int argc, char **argv);
extern int creeMap(void);
extern int actualiserMap(void);

extern int nouvelleSalle(int longueur, int largeur, int num_salle, int cote);
extern salle room;

extern void lancerBoucleDeJeu(int argc, char **argv);
extern unsigned int timer_start;
extern unsigned int start_time;
extern int machineFuite;

extern char inventaireIcons[7][64];

TTF_Font *fontUI = NULL;
extern Mix_Music *musique_acceleree;

int graine = -1;  // Graine non initialisée

int jeu(int argc, char **argv) {
    extern SDL_Window *window;
    extern SDL_Renderer *renderer;

    // Initialiser SDL_ttf
    if (TTF_Init() == -1) {
        SDL_Log("Erreur lors de l'initialisation de SDL_ttf : %s\n", TTF_GetError());
        return EXIT_FAILURE;
    }

    // Charger une police avec une taille adaptée à la fenêtre 1500x900
    TTF_Font *font = TTF_OpenFont("./src/fonts/SPACEBAR.ttf", 48);  // Augmentation de la taille de la police
    if (!font) {
        SDL_Log("Erreur lors du chargement de la police : %s\n", TTF_GetError());
        return EXIT_FAILURE;
    }

    // Couleur pour le texte : blanc
    SDL_Color textColor = {255, 255, 255, 255};  // Blanc

    // Variables pour capturer la graine
    char input[100] = {0};
    int input_pos = 0;
    
    SDL_bool entrer_graine = SDL_TRUE;

    // Texte "Entrer la graine :"
    SDL_Surface *textSurface = TTF_RenderText_Solid(font, "Entrez la graine :", textColor);
    if (!textSurface) {
        SDL_Log("Erreur lors de la création du texte : %s\n", TTF_GetError());
        return EXIT_FAILURE;
    }

    // Créer une texture à partir du texte
    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_FreeSurface(textSurface);

    if (!textTexture) {
        SDL_Log("Erreur lors de la création de la texture pour le texte : %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    // Récupérer la taille du texte pour l'afficher au centre
    int text_width = 0, text_height = 0;
    SDL_QueryTexture(textTexture, NULL, NULL, &text_width, &text_height);

    // Rectangle pour le texte
    SDL_Rect textRect;
    textRect.x = (WINDOW_WIDTH - text_width) / 2;  // Centrer horizontalement
    textRect.y = WINDOW_HEIGHT / 3;                // Placer à 1/3 de la hauteur
    textRect.w = text_width;
    textRect.h = text_height;

    // Rectangle pour afficher la saisie
    SDL_Rect inputRect;
    inputRect.x = textRect.x;            // Même alignement horizontal que le texte
    inputRect.y = textRect.y + textRect.h + 20;  // En dessous du texte
    inputRect.w = 200;      // Largeur initiale, ajustée dynamiquement
    inputRect.h = 50;                    // Hauteur de l'encadré de saisie

    // Boucle pour capturer l'entrée de la graine
    while (entrer_graine) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    return EXIT_FAILURE;
                case SDL_TEXTINPUT:
                    // Ajouter les caractères tapés au tableau de saisie
                    if (strlen(event.text.text) == 1 && input_pos < sizeof(input) - 1) {
                        input[input_pos++] = event.text.text[0];
                        input[input_pos] = '\0';
                    }
                    break;
                case SDL_KEYDOWN:
                    // Effacer le dernier caractère
                    if (event.key.keysym.sym == SDLK_BACKSPACE && input_pos > 0) {
                        input[--input_pos] = '\0';
                    }
                    // Appuyer sur Entrée pour confirmer la graine seulement si un nombre a été saisi
                    else if ((event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER) && input_pos > 0) {
                        entrer_graine = SDL_FALSE;
                    }
                    break;
            }

            // Effacer le rendu et définir un fond noir pour le contraste
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  // Couleur noire pour le fond
            SDL_RenderClear(renderer);

            // Dessiner le texte "Entrez la graine :"
            SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

            // Ajuster dynamiquement la taille du rectangle de saisie
            int input_width = input_pos * 24;  // Largeur du texte saisi, environ 24 pixels par caractère
            inputRect.w = (input_width > 200) ? input_width : 200;  // Ne pas rétrécir en dessous de 200px

            // Dessiner l'encadré de saisie
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);  // Couleur blanche pour l'encadré
            SDL_RenderDrawRect(renderer, &inputRect);  // Dessiner le rectangle

            // Afficher le texte saisi
            SDL_Surface *inputSurface = TTF_RenderText_Solid(font, input, textColor);
            if (inputSurface) {
                SDL_Texture *inputTexture = SDL_CreateTextureFromSurface(renderer, inputSurface);
                SDL_FreeSurface(inputSurface);

                if (inputTexture) {
                    // Afficher la saisie à l'intérieur de l'encadré
                    SDL_Rect inputTextRect = {inputRect.x + 5, inputRect.y + 5, input_width, inputRect.h - 10};
                    SDL_RenderCopy(renderer, inputTexture, NULL, &inputTextRect);
                    SDL_DestroyTexture(inputTexture);
                } else {
                    SDL_Log("Erreur lors de la création de la texture pour la saisie : %s\n", SDL_GetError());
                }
            }

            SDL_RenderPresent(renderer);
        }
    }

    // Affichage du message de validation après la première entrée
    SDL_Surface *validationSurface = TTF_RenderText_Solid(font, "Graine validée, génération de la carte...", textColor);
    if (!validationSurface) {
        SDL_Log("Erreur lors de la création du texte de validation : %s\n", TTF_GetError());
        return EXIT_FAILURE;
    }

    SDL_Texture *validationTexture = SDL_CreateTextureFromSurface(renderer, validationSurface);
    SDL_FreeSurface(validationSurface);

    if (!validationTexture) {
        SDL_Log("Erreur lors de la création de la texture pour le message de validation : %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    // Afficher ce texte au centre
    SDL_Rect validationRect;
    validationRect.x = (WINDOW_WIDTH - text_width) / 2;  // Centrer horizontalement
    validationRect.y = WINDOW_HEIGHT / 2;                // Centrer verticalement
    validationRect.w = text_width;
    validationRect.h = text_height;

    // Effacer le rendu précédent
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  // Fond noir
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, validationTexture, NULL, &validationRect);
    SDL_RenderPresent(renderer);

    // Attendre un peu pour que l'utilisateur voie le message
    SDL_Delay(1000);

    // Convertir l'entrée utilisateur en nombre pour la graine
    graine = atoi(input);  // Conversion de l'entrée en entier

    // Initialiser la graine aléatoire
    srand(graine);

    // Effacer le rendu avant la génération de la map
    SDL_RenderClear(renderer);

    // Appeler la fonction pour générer la map
    if (creeMap() != EXIT_SUCCESS) {
        SDL_Log("Erreur lors de la création de la map");
        return EXIT_FAILURE;
    }

    // Nettoyage des textures
    SDL_DestroyTexture(textTexture);
    SDL_DestroyTexture(validationTexture);

    // Nettoyage de la police et de SDL_ttf
    TTF_CloseFont(font);
    TTF_Quit();

    // Après la génération de la map, lancer la boucle de jeu
    lancerBoucleDeJeu(argc,argv);  // Assurez-vous que cette fonction est bien définie

    return EXIT_SUCCESS;
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
