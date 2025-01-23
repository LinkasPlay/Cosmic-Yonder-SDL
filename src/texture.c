#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>

#include "CosmicYonder.h"

#define FRAME_IN_ANIMATION 2

/*
	Windows : gcc src\*.c -o bin\progMain.exe -I include -L lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_mixer -mwindows
	Windows sans terminal qui ouvre : gcc src/texture.c -o bin/progTexture -I include -L lib -lmingw32 -lSDL2main -lSDL2 -mwindows
	Linux : gcc texture.c $(sdl2-config __cflags --libs) -o progTexture
    Windows : gcc src/*.c -I include -L lib -lmingw32 -lSDL2main -lSDL2

	Flags render
	SDL_RENDERER_SOFTWARE
	SDL_RENDERER_ACCELERATED
	SDL_RENDERER_PRESENTVSYNC
	SDL_RENDERER_TARGETTEXTURE
*/

extern personnage perso;

extern void SDL_ExitWithError(const char *message);

int texture( int argc, char **argv) {
	
    extern SDL_Window *window;
	extern SDL_Renderer *renderer;

    //Affichage
    SDL_Surface *image = NULL;
    SDL_Texture *texture = NULL;	
	extern tile contenuCase;
	char * nomImage;

    //Chargement image selon son contenu
    switch (contenuCase.contenu) {
		// case avec espace
        case -5:
            image = SDL_LoadBMP("src/image/espace.bmp");
            if(image == NULL){
		        SDL_DestroyRenderer(renderer);
		        SDL_DestroyWindow(window);
		        SDL_ExitWithError("Impossible de charger l'image de l'espace");
	    	}
			break;
        // case avec mur
        case -2:
            image = SDL_LoadBMP("src/image/mur.bmp");
            if(image == NULL){
		        SDL_DestroyRenderer(renderer);
		        SDL_DestroyWindow(window);
		        SDL_ExitWithError("Impossible de charger l'image d'un mur");
	    	}
        	break;
        // case avec porte
        case -1:
            image = SDL_LoadBMP("src/image/porte.bmp");
            if(image == NULL){
		        SDL_DestroyRenderer(renderer);
		        SDL_DestroyWindow(window);
		        SDL_ExitWithError("Impossible de charger l'image d'une porte");
	   	 	}
			break;
        // case vide
        case 0:
            image = SDL_LoadBMP("src/image/sol.bmp");
            if(image == NULL){
		        SDL_DestroyRenderer(renderer);
		        SDL_DestroyWindow(window);
		        SDL_ExitWithError("Impossible de charger l'image d'un sol");
	    	}
			break;
        // case avec personnage
        case 1:
			// choix image selon direction et frame
			switch (perso.direction){
			
			// direction = haut
			case 1:
			
				// frame 1 - 10 => image 1
				if (perso.frameAnimation < FRAME_IN_ANIMATION + 1 ){
					image = SDL_LoadBMP("src/image/personnage/personnageHaut (1).bmp");
				}

				// frame 11 - 20 => image 2
				else if (perso.frameAnimation < FRAME_IN_ANIMATION * 2 + 1 ){
					image = SDL_LoadBMP("src/image/personnage/personnageHaut (2).bmp");
				}

				// frame 21 - 30 => image 3
				else if (perso.frameAnimation < FRAME_IN_ANIMATION * 3 + 1 ){
					image = SDL_LoadBMP("src/image/personnage/personnageHaut (3).bmp");
				}
				// frame 31 - 40 => image 4
				else {
					image = SDL_LoadBMP("src/image/personnage/personnageHaut (4).bmp");
				}
				break;

			// direction = gauche
			case 2:
				
				// frame 1 - 10 => image 1
				if (perso.frameAnimation < FRAME_IN_ANIMATION + 1 ){
					image = SDL_LoadBMP("src/image/personnage/personnageGauche (1).bmp");
				}

				// frame 11 - 20 => image 2
				else if (perso.frameAnimation < FRAME_IN_ANIMATION * 2 + 1 ){
					image = SDL_LoadBMP("src/image/personnage/personnageGauche (2).bmp");
				}

				// frame 21 - 30 => image 3
				else if (perso.frameAnimation < FRAME_IN_ANIMATION * 3 + 1 ){
					image = SDL_LoadBMP("src/image/personnage/personnageGauche (3).bmp");
				}
				// frame 31 - 40 => image 4
				else {
					image = SDL_LoadBMP("src/image/personnage/personnageGauche (4).bmp");
				}
				break;

			// direction = bas
			case 3:
				
				// frame 1 - 10 => image 1
				if (perso.frameAnimation < FRAME_IN_ANIMATION + 1 ){
					image = SDL_LoadBMP("src/image/personnage/personnageBas (1).bmp");
				}

				// frame 11 - 20 => image 2
				else if (perso.frameAnimation < FRAME_IN_ANIMATION * 2 + 1 ){
					image = SDL_LoadBMP("src/image/personnage/personnageBas (2).bmp");
				}

				// frame 21 - 30 => image 3
				else if (perso.frameAnimation < FRAME_IN_ANIMATION * 3 + 1 ){
					image = SDL_LoadBMP("src/image/personnage/personnageBas (3).bmp");
				}
				// frame 31 - 40 => image 4
				else {
					image = SDL_LoadBMP("src/image/personnage/personnageBas (4).bmp");
				}
				break;

			// direction = droite
			case 4:
				
				// frame 1 - 10 => image 1
				if (perso.frameAnimation < FRAME_IN_ANIMATION + 1 ){
					image = SDL_LoadBMP("src/image/personnage/personnageDroite (1).bmp");
				}

				// frame 11 - 20 => image 2
				else if (perso.frameAnimation < FRAME_IN_ANIMATION * 2 + 1 ){
					image = SDL_LoadBMP("src/image/personnage/personnageDroite (2).bmp");
				}

				// frame 21 - 30 => image 3
				else if (perso.frameAnimation < FRAME_IN_ANIMATION * 3 + 1 ){
					image = SDL_LoadBMP("src/image/personnage/personnageDroite (3).bmp");
				}
				// frame 31 - 40 => image 4
				else {
					image = SDL_LoadBMP("src/image/personnage/personnageDroite (4).bmp");
				}
				break;
			
			default:
				break;
			}
            if(image == NULL){
		        SDL_DestroyRenderer(renderer);
		        SDL_DestroyWindow(window);
		        SDL_ExitWithError("Impossible de charger l'image du personnage");
	    	}
			if (perso.frameAnimation >= FRAME_IN_ANIMATION * 4){
				perso.frameAnimation = 0;
			}
			break;
        // case avec monstre
		case 2:
			switch (contenuCase.mstr.type) {
				case 1:
					image = SDL_LoadBMP("src/image/monstreType1.bmp");
					break;
				case 2:
					image = SDL_LoadBMP("src/image/monstreType2.bmp");
					break;
				case 3:
					image = SDL_LoadBMP("src/image/monstreType3.bmp");
					break;
				case 4: // Boss
					image = SDL_LoadBMP("src/image/monstreBoss.bmp");
					break;
				default:
					SDL_DestroyRenderer(renderer);
					SDL_DestroyWindow(window);
					SDL_ExitWithError("Type de monstre invalide");
					break;
			}
			if (image == NULL) {
				SDL_DestroyRenderer(renderer);
				SDL_DestroyWindow(window);
				SDL_ExitWithError("Impossible de charger l'image du monstre");
			}
			break;

		// case avec machine
		case 3:
			switch (contenuCase.spe.type) {
				case 1:
					image = SDL_LoadBMP("src/image/coffre.bmp");
					break;
				case 2:
					image = SDL_LoadBMP("src/image/machine.bmp");
					break;
				case 3:
					image = SDL_LoadBMP("src/image/grandeMachine.bmp");
					break;
				case 4:
					image = SDL_LoadBMP("src/image/grandeMachineReparation.bmp");
					break;
				case 5:
					image = SDL_LoadBMP("src/image/coffreOuvert.bmp");
					break;
				case 6:
					image = SDL_LoadBMP("src/image/machineReparee.bmp");
					break;
				case 7:
					image = SDL_LoadBMP("src/image/grandeMachineReparee.bmp");
					break;
				case 8:
					image = SDL_LoadBMP("src/image/sword.bmp");
					break;
				default:
					SDL_DestroyRenderer(renderer);
					SDL_DestroyWindow(window);
					SDL_ExitWithError("Type de machine invalide");
					break;
			}
			if (image == NULL) {
				SDL_DestroyRenderer(renderer);
				SDL_DestroyWindow(window);
				SDL_ExitWithError("Impossible de charger l'image de la machine");
			}
			break;
	};
    // Création de la texture
    texture = SDL_CreateTextureFromSurface(renderer, image);
    SDL_FreeSurface(image);  // Libération de la surface après utilisation
    if (texture == NULL) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_ExitWithError("Impossible de créer la texture");
    }

    extern SDL_Rect Case;

    if (SDL_QueryTexture(texture, NULL, NULL, &Case.w, &Case.h) != 0) {
        SDL_DestroyTexture(texture);  // Ajout de la libération de la texture
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_ExitWithError("Impossible de charger la texture");
    }

    // Affichage de la texture
    if (SDL_RenderCopy(renderer, texture, NULL, &Case) != 0) {
        SDL_DestroyTexture(texture);  // Ajout de la libération de la texture
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_ExitWithError("Impossible d'afficher la texture");
    }

    // Libération de la texture après affichage
    SDL_DestroyTexture(texture);  // Très important pour éviter la fuite de mémoire

    SDL_RenderPresent(renderer);
    return EXIT_SUCCESS;
}

//texture de l'ui

int textureInterface( int argc, char **argv) {
	
    extern SDL_Window *window;
	extern SDL_Renderer *renderer;

    //Affichage
    SDL_Surface *image = NULL;
    SDL_Texture *texture = NULL;	
	extern tile contenuCase;
	char * nomImage;

    //Chargement image selon son contenu
    switch (contenuCase.contenu) {
		// case avec espace
        case -5:
            image = SDL_LoadBMP("src/image/espace.bmp");
            if(image == NULL){
		        SDL_DestroyRenderer(renderer);
		        SDL_DestroyWindow(window);
		        SDL_ExitWithError("Impossible de charger l'image de l'espace");
	    	}
			break;
        // case avec mur
        case -2:
            image = SDL_LoadBMP("src/image/mur.bmp");
            if(image == NULL){
		        SDL_DestroyRenderer(renderer);
		        SDL_DestroyWindow(window);
		        SDL_ExitWithError("Impossible de charger l'image d'un mur");
	    	}
        	break;
        // case avec porte
        case -1:
            image = SDL_LoadBMP("src/image/porte.bmp");
            if(image == NULL){
		        SDL_DestroyRenderer(renderer);
		        SDL_DestroyWindow(window);
		        SDL_ExitWithError("Impossible de charger l'image d'une porte");
	   	 	}
			break;
        // case vide
        case 0:
            image = SDL_LoadBMP("src/image/sol.bmp");
            if(image == NULL){
		        SDL_DestroyRenderer(renderer);
		        SDL_DestroyWindow(window);
		        SDL_ExitWithError("Impossible de charger l'image d'un sol");
	    	}
			break;
        // case avec personnage
        case 1:
			// choix image selon direction et frame
			switch (perso.direction){
			
			// direction = haut
			case 1:
			
				// frame 1 - 10 => image 1
				if (perso.frameAnimation < FRAME_IN_ANIMATION + 1 ){
					image = SDL_LoadBMP("src/image/personnage/personnageHaut (1).bmp");
				}

				// frame 11 - 20 => image 2
				else if (perso.frameAnimation < FRAME_IN_ANIMATION * 2 + 1 ){
					image = SDL_LoadBMP("src/image/personnage/personnageHaut (2).bmp");
				}

				// frame 21 - 30 => image 3
				else if (perso.frameAnimation < FRAME_IN_ANIMATION * 3 + 1 ){
					image = SDL_LoadBMP("src/image/personnage/personnageHaut (3).bmp");
				}
				// frame 31 - 40 => image 4
				else {
					image = SDL_LoadBMP("src/image/personnage/personnageHaut (4).bmp");
				}
				break;

			// direction = gauche
			case 2:
				
				// frame 1 - 10 => image 1
				if (perso.frameAnimation < FRAME_IN_ANIMATION + 1 ){
					image = SDL_LoadBMP("src/image/personnage/personnageGauche (1).bmp");
				}

				// frame 11 - 20 => image 2
				else if (perso.frameAnimation < FRAME_IN_ANIMATION * 2 + 1 ){
					image = SDL_LoadBMP("src/image/personnage/personnageGauche (2).bmp");
				}

				// frame 21 - 30 => image 3
				else if (perso.frameAnimation < FRAME_IN_ANIMATION * 3 + 1 ){
					image = SDL_LoadBMP("src/image/personnage/personnageGauche (3).bmp");
				}
				// frame 31 - 40 => image 4
				else {
					image = SDL_LoadBMP("src/image/personnage/personnageGauche (4).bmp");
				}
				break;

			// direction = bas
			case 3:
				
				// frame 1 - 10 => image 1
				if (perso.frameAnimation < FRAME_IN_ANIMATION + 1 ){
					image = SDL_LoadBMP("src/image/personnage/personnageBas (1).bmp");
				}

				// frame 11 - 20 => image 2
				else if (perso.frameAnimation < FRAME_IN_ANIMATION * 2 + 1 ){
					image = SDL_LoadBMP("src/image/personnage/personnageBas (2).bmp");
				}

				// frame 21 - 30 => image 3
				else if (perso.frameAnimation < FRAME_IN_ANIMATION * 3 + 1 ){
					image = SDL_LoadBMP("src/image/personnage/personnageBas (3).bmp");
				}
				// frame 31 - 40 => image 4
				else {
					image = SDL_LoadBMP("src/image/personnage/personnageBas (4).bmp");
				}
				break;

			// direction = droite
			case 4:
				
				// frame 1 - 10 => image 1
				if (perso.frameAnimation < FRAME_IN_ANIMATION + 1 ){
					image = SDL_LoadBMP("src/image/personnage/personnageDroite (1).bmp");
				}

				// frame 11 - 20 => image 2
				else if (perso.frameAnimation < FRAME_IN_ANIMATION * 2 + 1 ){
					image = SDL_LoadBMP("src/image/personnage/personnageDroite (2).bmp");
				}

				// frame 21 - 30 => image 3
				else if (perso.frameAnimation < FRAME_IN_ANIMATION * 3 + 1 ){
					image = SDL_LoadBMP("src/image/personnage/personnageDroite (3).bmp");
				}
				// frame 31 - 40 => image 4
				else {
					image = SDL_LoadBMP("src/image/personnage/personnageDroite (4).bmp");
				}
				break;
			
			default:
				break;
			}
            if(image == NULL){
		        SDL_DestroyRenderer(renderer);
		        SDL_DestroyWindow(window);
		        SDL_ExitWithError("Impossible de charger l'image du personnage");
	    	}
			if (perso.frameAnimation >= FRAME_IN_ANIMATION * 4){
				perso.frameAnimation = 0;
			}
			break;
        // case avec monstre
        case 2:
            image = SDL_LoadBMP("src/image/monstre.bmp");
            if(image == NULL){
		        SDL_DestroyRenderer(renderer);
		        SDL_DestroyWindow(window);
		        SDL_ExitWithError("Impossible de charger l'image d'un monstre");
	    	}
			break;
        // case avec machine
        case 3:
            image = SDL_LoadBMP("src/image/machine.bmp");
            if(image == NULL){
		        SDL_DestroyRenderer(renderer);
		        SDL_DestroyWindow(window);
		        SDL_ExitWithError("Impossible de charger l'image d'une machine ");
	   		}
        	break;
		default:
			break;
    }

    //creeation texture
    texture = SDL_CreateTextureFromSurface(renderer, image);
    SDL_FreeSurface(image);
    if(texture == NULL){
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_ExitWithError("Impossible de creer la texture");
	}

    extern SDL_Rect Case;

    if(SDL_QueryTexture(texture, NULL, NULL, &Case.w, &Case.h) != 0){
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
        SDL_ExitWithError("Impossible de charger la texture");
    }

    //rectangle.x = (WINDOW_WIDTH - rectangle.w) / 2;
    //rectangle.y = (WINDOW_HEIGHT - rectangle.h) / 2;

    if(SDL_RenderCopy(renderer, texture, NULL, &Case) != 0){
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_ExitWithError("Impossible d'afficher la texture");
	}

	SDL_RenderPresent(renderer);
	//SDL_Delay(5);

    /*effacement rendu
	if(SDL_RenderClear(renderer) != 0){
		SDL_ExitWithError("Efffacement rendu echouee");
	}

	// fin programme / libération mémoire
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
    */
	return EXIT_SUCCESS;
}

/*message erreur
void SDL_ExitWithError(const char *message){
	SDL_Log("ERREUR : %s > %s\n",message, SDL_GetError());
	SDL_Quit();
	exit(EXIT_FAILURE);
}
*/

