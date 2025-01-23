#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>

#include "CosmicYonder.h"

extern int jeu(int argc, char **argv);
int num_salle;

Mix_Chunk *sonMenu = NULL;
Mix_Chunk *sonMove = NULL;
Mix_Chunk *sonDamage = NULL;
Mix_Chunk *sonRoom = NULL;
Mix_Chunk *sonHover = NULL;
Mix_Chunk *gameOverSound = NULL;
Mix_Chunk *swordSound = NULL;
Mix_Chunk *gunSound = NULL;
Mix_Chunk *mobHurtSound1 = NULL;
Mix_Chunk *mobDeadSound1 = NULL;
Mix_Chunk *sonLevelUp = NULL;
Mix_Chunk *sonPotionLife = NULL;
Mix_Chunk *sonPotionXP = NULL;
Mix_Chunk *sonWrench = NULL;
Mix_Chunk *sonKey = NULL;
Mix_Chunk *sonBigKey = NULL;
Mix_Chunk *sonFail = NULL;
Mix_Chunk *sonSwordPickup = NULL;

void initialiserSons() {
    // Allouer les canaux nécessaires
    Mix_AllocateChannels(64);

    // Charger les sons existants
    sonMenu = Mix_LoadWAV("./src/musique/pause.mp3");
    if (!sonMenu) {
        SDL_Log("Erreur chargement son pause: %s", Mix_GetError());
    }

    swordSound = Mix_LoadWAV("./src/musique/sword.mp3");
    if (!swordSound) {
        SDL_Log("Erreur chargement son épée: %s", Mix_GetError());
    }

    gunSound = Mix_LoadWAV("./src/musique/gun.mp3");
    if (!gunSound) {
        SDL_Log("Erreur chargement son pistolet: %s", Mix_GetError());
    }

    mobHurtSound1 = Mix_LoadWAV("./src/musique/mobHurt1.mp3");
    if (!mobHurtSound1) {
        SDL_Log("Erreur chargement son dégât monstre: %s", Mix_GetError());
    }

    mobDeadSound1 = Mix_LoadWAV("./src/musique/mobDead1.mp3");
    if (!mobDeadSound1) {
        SDL_Log("Erreur chargement son mort monstre: %s", Mix_GetError());
    }

    sonHover = Mix_LoadWAV("./src/musique/hover.mp3");
    if (!sonHover) {
        SDL_Log("Erreur chargement son hover: %s", Mix_GetError());
    }

    gameOverSound = Mix_LoadWAV("./src/musique/gameover.mp3");
    if (!gameOverSound) {
        SDL_Log("Erreur chargement son gameover: %s", Mix_GetError());
    }

    sonMove = Mix_LoadWAV("./src/musique/move.mp3");
    if (!sonMove) {
        SDL_Log("Erreur chargement son déplacement: %s", Mix_GetError());
    }

    sonDamage = Mix_LoadWAV("./src/musique/damage.mp3");
    if (!sonDamage) {
        SDL_Log("Erreur chargement son dégâts: %s", Mix_GetError());
    }

    sonRoom = Mix_LoadWAV("./src/musique/room.mp3");
    if (!sonRoom) {
        SDL_Log("Erreur chargement son salle: %s", Mix_GetError());
    }

    sonLevelUp = Mix_LoadWAV("./src/musique/level_up.mp3");
    if (!sonLevelUp) {
        SDL_Log("Erreur chargement son gain niveau: %s", Mix_GetError());
    }

    sonPotionLife = Mix_LoadWAV("./src/musique/potion_life.mp3");
    if (!sonPotionLife) {
        SDL_Log("Erreur chargement son potion vie: %s", Mix_GetError());
    }

    sonPotionXP = Mix_LoadWAV("./src/musique/potion_xp.mp3");
    if (!sonPotionXP) {
        SDL_Log("Erreur chargement son potion XP: %s", Mix_GetError());
    }

    sonWrench = Mix_LoadWAV("./src/musique/wrench.mp3");
    if (!sonWrench) {
        SDL_Log("Erreur chargement son clé à molette: %s", Mix_GetError());
    }

    sonKey = Mix_LoadWAV("./src/musique/key.mp3");
    if (!sonKey) {
        SDL_Log("Erreur chargement son clé: %s", Mix_GetError());
    }

    sonBigKey = Mix_LoadWAV("./src/musique/big_key.mp3");
    if (!sonBigKey) {
        SDL_Log("Erreur chargement son grande clé: %s", Mix_GetError());
    }

    sonFail = Mix_LoadWAV("./src/musique/fail.mp3");
    if (!sonFail) {
        SDL_Log("Erreur chargement son échec: %s", Mix_GetError());
    }

    sonSwordPickup = Mix_LoadWAV("./src/musique/sword_pickup.mp3");
    if (!sonSwordPickup) {
        SDL_Log("Erreur chargement son prise épée: %s", Mix_GetError());
    }
}

void clean_ressources(SDL_Window *w, SDL_Renderer *r, SDL_Texture *t) {
    // Libération des sons
    Mix_FreeChunk(sonMenu);
    Mix_FreeChunk(sonMove);
    Mix_FreeChunk(sonDamage);
    Mix_FreeChunk(sonRoom);
    Mix_FreeChunk(swordSound);
    Mix_FreeChunk(gunSound);
    Mix_FreeChunk(mobHurtSound1);
    Mix_FreeChunk(mobDeadSound1);


    // Autres nettoyages
    if(t != NULL){
        SDL_DestroyTexture(t);
    }
    if(r != NULL){
        SDL_DestroyRenderer(r);
    }
    if(w != NULL){
        SDL_DestroyWindow(w);
    }

    Mix_CloseAudio(); // Fermer l'audio
}


// NE PAS BOUGER WINDOW ET RENDERER OU IMPOSSIBLE DE COMPILER
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL; 
Mix_Music *menuMusic = NULL;
Mix_Music *gameMusic = NULL;
Mix_Music *musique_acceleree = NULL;

int afficherMenuPrincipal(SDL_Renderer *renderer) {
    SDL_Surface *background = NULL;
    SDL_Texture *backgroundTexture = NULL;

    // Charger la police pour le texte
    if (TTF_Init() == -1) {
        SDL_Log("Erreur lors de l'initialisation de SDL_ttf : %s\n", TTF_GetError());
        return EXIT_FAILURE;
    }

    TTF_Font *font = TTF_OpenFont("./src/fonts/SPACEBAR.ttf", 32);
    if (!font) {
        SDL_Log("Erreur lors du chargement de la police : %s\n", TTF_GetError());
        return EXIT_FAILURE;
    }

    TTF_Font *fontUI = TTF_OpenFont("./src/fonts/ui.otf", 32);
    if (!font) {
        SDL_Log("Erreur lors du chargement de la police : %s\n", TTF_GetError());
        return EXIT_FAILURE;
    }

    // Charger l'image de fond
    background = SDL_LoadBMP("./src/image/spacet.bmp");
    if (background == NULL) {
        SDL_Log("Erreur : Chargement de l'image échoué > %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    backgroundTexture = SDL_CreateTextureFromSurface(renderer, background);
    SDL_FreeSurface(background);

    if (backgroundTexture == NULL) {
        SDL_Log("Erreur : Création de la texture échouée > %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    // Charger la musique du menu
    menuMusic = Mix_LoadMUS("./src/musique/menu.mp3");
    if (!menuMusic) {
        SDL_Log("Erreur lors du chargement de la musique du menu : %s\n", Mix_GetError());
        return EXIT_FAILURE;
    }

    // Charger la musique du vaisseau accéléré
    musique_acceleree = Mix_LoadMUS("./src/musique/tension.mp3");
    if (!musique_acceleree) {
        SDL_Log("Erreur lors du chargement de la musique du menu : %s\n", Mix_GetError());
        return EXIT_FAILURE;
    }

    // Jouer la musique du menu
    Mix_PlayMusic(menuMusic, -1);

    // Définir les boutons du menu principal
    int button_width = 300;
    int button_height = 50;
    int spacing = 30;

    SDL_Rect button_new_game = {(WINDOW_WIDTH - button_width) / 2, WINDOW_HEIGHT / 2 - (2 * (button_height + spacing)), button_width, button_height};
    SDL_Rect button_load_game = {(WINDOW_WIDTH - button_width) / 2, button_new_game.y + button_height + spacing, button_width, button_height};
    SDL_Rect button_soundtrack = {(WINDOW_WIDTH - button_width) / 2, button_load_game.y + button_height + spacing, button_width, button_height};
    SDL_Rect button_quit = {(WINDOW_WIDTH - button_width) / 2, button_soundtrack.y + button_height + spacing, button_width, button_height};

    // Couleurs pour les boutons
    SDL_Color buttonColor = {100, 100, 100, 255};
    SDL_Color buttonHoverColor = {150, 150, 150, 255};
    SDL_Color textColor = {255, 255, 255, 255};

    SDL_Surface *textNewGameSurface = TTF_RenderText_Solid(font, "Nouvelle Partie", textColor);
    SDL_Surface *textLoadGameSurface = TTF_RenderText_Solid(font, "Charger Partie", textColor);
    SDL_Surface *textSoundtrackSurface = TTF_RenderText_Solid(font, "Soundtrack", textColor);
    SDL_Surface *textQuitSurface = TTF_RenderText_Solid(font, "Quitter", textColor);

    SDL_Texture *textNewGameTexture = SDL_CreateTextureFromSurface(renderer, textNewGameSurface);
    SDL_Texture *textLoadGameTexture = SDL_CreateTextureFromSurface(renderer, textLoadGameSurface);
    SDL_Texture *textSoundtrackTexture = SDL_CreateTextureFromSurface(renderer, textSoundtrackSurface);
    SDL_Texture *textQuitTexture = SDL_CreateTextureFromSurface(renderer, textQuitSurface);

    SDL_FreeSurface(textNewGameSurface);
    SDL_FreeSurface(textLoadGameSurface);
    SDL_FreeSurface(textSoundtrackSurface);
    SDL_FreeSurface(textQuitSurface);

    SDL_bool hover_new_game = SDL_FALSE;
    SDL_bool hover_load_game = SDL_FALSE;
    SDL_bool hover_soundtrack = SDL_FALSE;
    SDL_bool hover_quit = SDL_FALSE;

    SDL_bool running = SDL_TRUE;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    return EXIT_FAILURE;

                case SDL_MOUSEMOTION:
                    // Vérifier si la souris survole un bouton
                    hover_new_game = SDL_FALSE;
                    hover_load_game = SDL_FALSE;
                    hover_soundtrack = SDL_FALSE;
                    hover_quit = SDL_FALSE;

                    if (event.motion.x >= button_new_game.x && event.motion.x <= button_new_game.x + button_width &&
                        event.motion.y >= button_new_game.y && event.motion.y <= button_new_game.y + button_height) {
                        hover_new_game = SDL_TRUE;
                    }

                    if (event.motion.x >= button_load_game.x && event.motion.x <= button_load_game.x + button_width &&
                        event.motion.y >= button_load_game.y && event.motion.y <= button_load_game.y + button_height) {
                        hover_load_game = SDL_TRUE;
                    }

                    if (event.motion.x >= button_soundtrack.x && event.motion.x <= button_soundtrack.x + button_width &&
                        event.motion.y >= button_soundtrack.y && event.motion.y <= button_soundtrack.y + button_height) {
                        hover_soundtrack = SDL_TRUE;
                    }

                    if (event.motion.x >= button_quit.x && event.motion.x <= button_quit.x + button_width &&
                        event.motion.y >= button_quit.y && event.motion.y <= button_quit.y + button_height) {
                        hover_quit = SDL_TRUE;
                    }
                    break;

                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        // Bouton Nouvelle Partie
                        if (hover_new_game) {
                            Mix_PlayChannel(CHANNEL_HOVER, sonHover, 0);
                            Mix_HaltMusic();  // Arrêter la musique du menu
                            Mix_FreeMusic(menuMusic);  // Libérer la musique du menu

                            // Charger et jouer la musique du jeu
                            gameMusic = Mix_LoadMUS("./src/musique/vaisseau.mp3");
                            if (!gameMusic) {
                                SDL_Log("Erreur lors du chargement de la musique du jeu : %s\n", Mix_GetError());
                            }
                            Mix_PlayMusic(gameMusic, -1);

                            return EXIT_SUCCESS; // Commence le jeu
                        }
                        // Bouton Quitter
                        if (hover_quit) {
                            Mix_PlayChannel(CHANNEL_HOVER, sonHover, 0);
                            SDL_Log("Fermeture du programme...");
                            Mix_HaltMusic();
                            Mix_FreeMusic(menuMusic);
                            Mix_CloseAudio();
                            SDL_DestroyTexture(backgroundTexture);  // Libération des ressources
                            SDL_DestroyRenderer(renderer);
                            SDL_DestroyWindow(window);
                            SDL_Quit(); // Quitte SDL et le programme
                            exit(EXIT_SUCCESS); // Assure la fermeture propre
                        }
                    }
                    break;
            }
        }

        // Dessiner l'arrière-plan
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);

        // Dessiner les boutons
        SDL_SetRenderDrawColor(renderer, buttonColor.r, buttonColor.g, buttonColor.b, buttonColor.a);

        // Bouton Nouvelle Partie
        if (hover_new_game) {
            SDL_SetRenderDrawColor(renderer, buttonHoverColor.r, buttonHoverColor.g, buttonHoverColor.b, buttonHoverColor.a);
        }
        SDL_RenderFillRect(renderer, &button_new_game);
        SDL_RenderCopy(renderer, textNewGameTexture, NULL, &button_new_game);

        // Bouton Charger Partie
        SDL_SetRenderDrawColor(renderer, buttonColor.r, buttonColor.g, buttonColor.b, buttonColor.a);
        if (hover_load_game) {
            SDL_SetRenderDrawColor(renderer, buttonHoverColor.r, buttonHoverColor.g, buttonHoverColor.b, buttonHoverColor.a);
        }
        SDL_RenderFillRect(renderer, &button_load_game);
        SDL_RenderCopy(renderer, textLoadGameTexture, NULL, &button_load_game);

        // Bouton Soundtrack
        SDL_SetRenderDrawColor(renderer, buttonColor.r, buttonColor.g, buttonColor.b, buttonColor.a);
        if (hover_soundtrack) {
            SDL_SetRenderDrawColor(renderer, buttonHoverColor.r, buttonHoverColor.g, buttonHoverColor.b, buttonHoverColor.a);
        }
        SDL_RenderFillRect(renderer, &button_soundtrack);
        SDL_RenderCopy(renderer, textSoundtrackTexture, NULL, &button_soundtrack);

        // Bouton Quitter
        SDL_SetRenderDrawColor(renderer, buttonColor.r, buttonColor.g, buttonColor.b, buttonColor.a);
        if (hover_quit) {
            SDL_SetRenderDrawColor(renderer, buttonHoverColor.r, buttonHoverColor.g, buttonHoverColor.b, buttonHoverColor.a);
        }
        SDL_RenderFillRect(renderer, &button_quit);
        SDL_RenderCopy(renderer, textQuitTexture, NULL, &button_quit);

        // Présenter le rendu
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(textNewGameTexture);
    SDL_DestroyTexture(textLoadGameTexture);
    SDL_DestroyTexture(textSoundtrackTexture);
    SDL_DestroyTexture(textQuitTexture);
    SDL_DestroyTexture(backgroundTexture);

    TTF_CloseFont(font);
    TTF_Quit();

    return EXIT_SUCCESS;
}

int main(int argc, char **argv) {
    // Initialiser SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        SDL_Log("Erreur : Initialisation SDL > %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    // Initialiser SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        SDL_Log("Erreur lors de l'initialisation du Mix_OpenAudio: %s", Mix_GetError());
        exit(EXIT_FAILURE);
    }

    initialiserSons(); // Charger les sons ici

    // Créer la fenêtre
    window = SDL_CreateWindow("Cosmic Yonder", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1500, 900, 0);
    if (window == NULL) {
        SDL_Log("Erreur : Création fenêtre échouée > %s\n", SDL_GetError());
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

    // Créer le renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        SDL_Log("Erreur : Création renderer échouée > %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

    // Afficher le menu principal
    if (afficherMenuPrincipal(renderer) != EXIT_SUCCESS) {
        SDL_Log("Erreur : Affichage du menu principal échoué");
        clean_ressources(window, renderer, NULL);
        exit(EXIT_FAILURE);
    }

    // Appel à la fonction jeu
    if (jeu(argc, argv) != EXIT_SUCCESS) {
        SDL_Log("Erreur lors de l'exécution du jeu");
    }

    // Libérer les ressources
    clean_ressources(window, renderer, NULL);
    Mix_CloseAudio();
    SDL_Quit();

    return EXIT_SUCCESS;
}
