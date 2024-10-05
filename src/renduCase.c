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

extern int texture( int argc, char **argv);
extern int creeMap(void);
extern int actualiserMap(void);

extern int nouvelleSalle(int longueur, int largeur, int num_salle, int cote);
extern salle room;

extern void lancerBoucleDeJeu(int argc, char **argv);

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
    SDL_Log("Texte 'Entrez la graine :' créé avec succès");

    // Créer une texture à partir du texte
    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_FreeSurface(textSurface);

    if (!textTexture) {
        SDL_Log("Erreur lors de la création de la texture pour le texte : %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }
    SDL_Log("Texture pour le texte créée avec succès");

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

    SDL_Log("Saisie de la graine...");

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
                        SDL_Log("Entrée de la graine validée : %s", input);
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

    SDL_Log("Message de validation affiché");

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
    SDL_Log("Graine : %d", graine);

    // Initialiser la graine aléatoire
    srand(graine);

    // Effacer le rendu avant la génération de la map
    SDL_RenderClear(renderer);

    // Appeler la fonction pour générer la map
    if (creeMap() != EXIT_SUCCESS) {
        SDL_Log("Erreur lors de la création de la map");
        return EXIT_FAILURE;
    }

    SDL_Log("Map générée avec succès");

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
