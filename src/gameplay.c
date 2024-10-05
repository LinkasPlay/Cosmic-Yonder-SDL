#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>

#include "CosmicYonder.h"

/*
	Windows : gcc src\*.c -o bin\progMain.exe -I include -L lib -lmingw32 -lSDL2main -lSDL2 -mwindows
	Linux : gcc renduCase.c $(sdl2-config __cflags --libs) -o progRenduCase

	Flags render
	SDL_RENDERER_SOFTWARE
	SDL_RENDERER_ACCELERATED
	SDL_RENDERER_PRESENTVSYNC
	SDL_RENDERER_TARGETTEXTURE
*/

/*
Expliquation gameplay du jeu :

    on commence avec l'invetaire vide
    1) l'épée/tuyau perme de fraper autour de soi, fait 50 de dégats
    2 ) le pistolet tire sur une ligne droite devant soi, fait 20 dégats
    3) les potions de vies redonnes 2 coeurs au joueur
    4) les potions d'xp donne 1/4 de l'xp nécessaire pour ganer un niveau
    5) les clé a molette servent à réparé les machines
    6) les clés servent à ouvrir les coffres
    7) la grande clé sert à ouvrir la dernière salle

    une épée/tuyau est généré dans l'une des 5 premières salles (1/3 chance par sale, 1/1 si salle 5)
    une salle de boss, fermé avec une porte spéciale est générée dans les alles 10 à 25 ==> vérifié qu'il y a des portes libres autrepart pour ne pas bloquer joueur

    on trouve forcement un pistole dans l'un des 10 premiers coffres ouverts
    on trouve la clé du boss dans l'un des coffres entre le 10eme et 20eme
    les coffres on 40% de chance de donner une clé a molette, 30% pour une potion de vie, 30% pour une potion d'xp

    utiliser une clé à molette sur une machine permet de gagner 3minutes en plus au chrono total
    il faut utiliser 2 clé a molette sur les grandes machines pour les réparés
    il faut réparé 3 grandes machines pour avoir la vrai fin

    il y a quatre type de monstres :
        niv 1 : 80 pv, 10 xp
        niv 2 : 180 pv, 20 xp | drop : clé * 0.05
        niv 3 : 260 pv, 30 xp | drop : clé * 0.1, clé a molette * 0.1
        niv 5 (boss) : 1000 pv, 0 xp => fin du jeu

*/

extern Mix_Chunk *sonMenu;
extern Mix_Chunk *sonMove;
extern Mix_Chunk *sonDamage;
extern Mix_Chunk *sonRoom;
extern Mix_Chunk *sonHover;


extern tile **map;

extern int graine;

extern int Xcamera;
extern int Ycamera;

personnage perso;
personnage persoPast;

extern int texture(int argc, char **argv);
extern int creeMap(void);
extern int actualiserMap(void);
extern int nouvelleSalle(int longueur, int largeur, int num_salle, int cote);
extern unsigned int aleatoire(int salle, int graine, int min, int max);
extern void save_game(const char *filename);
extern int camera(int argc, char **argv);

extern void SDL_LimitFPS(unsigned int limit);

extern int num_salle;

void gameOver() {
    extern SDL_Renderer *renderer;
    extern Mix_Chunk *gameOverSound;

    // Arrêter tous les sons actuellement en cours pour libérer les canaux
    Mix_HaltChannel(-1);  // Arrêter tous les canaux audio

    // Jouer l'effet sonore du game over
    if (Mix_PlayChannel(-1, gameOverSound, 0) == -1) {
        SDL_Log("Erreur lors de la lecture du son Game Over : %s\n", Mix_GetError());
    }

    // Initialiser SDL_ttf si ce n'est pas déjà fait
    if (TTF_Init() == -1) {
        SDL_Log("Erreur lors de l'initialisation de SDL_ttf : %s\n", TTF_GetError());
        return;
    }

    // Charger une police
    TTF_Font *font = TTF_OpenFont("./src/fonts/SPACEBAR.ttf", 64);  // Taille de la police adaptée pour "Game Over"
    if (!font) {
        SDL_Log("Erreur lors du chargement de la police : %s\n", TTF_GetError());
        return;
    }

    // Couleur pour le texte : noir
    SDL_Color textColor = {0, 0, 0, 255};  // Noir

    // Surface et texture pour le texte "Game Over"
    SDL_Surface *textSurface = TTF_RenderText_Solid(font, "Game Over", textColor);
    if (!textSurface) {
        SDL_Log("Erreur lors de la création du texte Game Over : %s\n", TTF_GetError());
        return;
    }
    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_FreeSurface(textSurface);

    // Définir un rectangle pour positionner "Game Over"
    SDL_Rect textRect;
    textRect.x = (WINDOW_WIDTH - 400) / 2;  // Centrer horizontalement
    textRect.y = (WINDOW_HEIGHT - 100) / 2;  // Centrer verticalement
    textRect.w = 400;
    textRect.h = 100;

    // Afficher un écran rouge
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);  // Rouge
    SDL_RenderClear(renderer);

    // Afficher "Game Over"
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_RenderPresent(renderer);

    // Libérer la texture et fermer la police
    SDL_DestroyTexture(textTexture);
    TTF_CloseFont(font);

    // Attendre que le son du game over finisse avant de quitter
    while (Mix_Playing(-1) != 0) {
        SDL_Delay(100);  // Vérifie toutes les 100ms si le son est terminé
    }

    // Quitter SDL après le Game Over
    Mix_HaltChannel(-1);  // Arrêter tout son en cours juste avant de quitter
    SDL_Quit();
    exit(EXIT_SUCCESS);  // Fermer le jeu après l'affichage du "Game Over"
}

int attaqueEpee(){
    printf("aya");
    return EXIT_SUCCESS;
}


void lvlMaj(monstre mstr) {
    perso.xp += mstr.xp;
    if (perso.xp > (100 * (perso.lvl * 0.9))) {
        perso.xp -= (100 * (perso.lvl * 0.9));
        perso.lvl++;
    }
}

int degatMonstre(int dmg, monstre mstr) {
    mstr.hp -= dmg;
    if (mstr.hp > 0) {
        mstr.frameAnimation = -5;
    } else {
        mstr.frameAnimation = -10;
        lvlMaj(mstr);
        if (mstr.loot > 1) {
            perso.inv[mstr.loot]++;
        }
    }
    return 0;
}

void deplacerMonstresVersJoueur() {
    for (int i = 0; i < DIMENSION_MAP; i++) {
        for (int j = 0; j < DIMENSION_MAP; j++) {
            if (map[i][j].contenu == 2) {  // Si la case contient un monstre
                int dx = 0, dy = 0;

                // Calculer la différence de position entre le monstre et le joueur
                int diffX = perso.posX - i;
                int diffY = perso.posY - j;

                // Priorité : se déplacer horizontalement si la différence est plus grande en X, sinon verticalement
                if (abs(diffX) > abs(diffY)) {
                    if (diffX > 0) {
                        dx = 1;  // Déplacement vers la droite
                    } else {
                        dx = -1;  // Déplacement vers la gauche
                    }
                } else {
                    if (diffY > 0) {
                        dy = 1;  // Déplacement vers le bas
                    } else {
                        dy = -1;  // Déplacement vers le haut
                    }
                }

                // Nouvelle position du monstre
                int new_x = i + dx;
                int new_y = j + dy;

                // Vérifier que la nouvelle case est libre (ni mur, ni monstre)
                if (map[new_x][new_y].contenu == 0) {
                    // Déplacer le monstre vers la nouvelle case
                    map[new_x][new_y].contenu = 2;
                    map[new_x][new_y].mstr = map[i][j].mstr;

                    // Vider l'ancienne case
                    map[i][j].contenu = 0;
                    map[i][j].mstr.hp = 0;
                }
                // Si le monstre atteint le joueur
                else if (new_x == perso.posX && new_y == perso.posY) {
                    if (!perso.invulnerable) {
                        perso.hp--;
                        Mix_PlayChannel(-1, sonDamage, 0);
                        SDL_Log("Le joueur a été attaqué par un monstre ! HP restants : %d", perso.hp);
                        if (perso.hp <= 0) {
                            gameOver();
                            return;
                        }
                        // Activer l'invulnérabilité du joueur
                        perso.invulnerable = SDL_TRUE;
                        perso.invulnerable_timer = SDL_GetTicks();
                    }

                    // Le monstre disparaît après avoir attaqué le joueur
                    map[i][j].contenu = 0;
                    map[i][j].mstr.hp = 0;
                }
            }
        }
    }
}




int testSol(int x, int y, int cote) {
    switch (map[x][y].contenu) {
        case -2:  // Mur
        case 3:   // Objet spécial
            return 0;
            break;

        case -1:  // Porte
            num_salle++;  // Incrémenter le nombre de salles
            int largeur = aleatoire(15 * num_salle, graine * 7, 5, 11);
            int longueur = aleatoire(9 * num_salle, graine * 2, 5, 11);
            
            if (nouvelleSalle(longueur, largeur, num_salle, cote) != EXIT_SUCCESS) {
                printf("Erreur lors de la génération de la salle\n");
                return -1;
            }
            switch (cote) {
                case 0: // Haut
                    perso.posY = y + 1;
                    break;
                case 1: // Gauche
                    perso.posX = x + 1;
                    break;
                case 2: // Bas
                    perso.posY = y - 1;
                    break;
                case 3: // Droite
                    perso.posX = x - 1;
                    break;
                default:
                    break;
            }
            Mix_PlayChannel(-1, sonRoom, 0);
            return 1;
            break;

        case 2:  // Monstre
            if (!perso.invulnerable) {
                // Joueur touché par le monstre, perdre un PV
                perso.hp--;
                Mix_PlayChannel(-1, sonDamage, 0);
                SDL_Log("Le joueur a été attaqué par un monstre ! HP restants : %d", perso.hp);

                // Le monstre disparaît
                map[x][y].contenu = 0;  // Suppression du monstre
                map[x][y].mstr.hp = 0;
                map[x][y].mstr.xp = 0;
                map[x][y].mstr.loot = 0;

                // Déclencher l'invulnérabilité du joueur pendant 5 secondes
                perso.invulnerable = SDL_TRUE;
                perso.invulnerable_timer = SDL_GetTicks();  // Début du timer d'invulnérabilité
            }

            if (perso.hp <= 0) {
                gameOver();
            }

            return 1;
            break;

        case 0: // rien
            Mix_PlayChannel(-1, sonMove, 0);
            return 1;
            break;
        default:
            return 1;
            break;
    }
    return 1;
}



int mouvementHaut(void) {
    if (perso.posY > 0 && testSol(perso.posX, perso.posY - 1, 0)) {
        if (Ycamera > 0) {
            Ycamera--;
        }
        persoPast.posX = perso.posX;
        persoPast.posY = perso.posY;
        perso.posY--;
        perso.direction = 1;
    }

    if (actualiserMap() != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int mouvementBas(void) {
    if (perso.posY < DIMENSION_MAP - 1 && testSol(perso.posX, perso.posY + 1, 2)) {
        if (Ycamera < DIMENSION_MAP - (WINDOW_HEIGHT / 100)) {
            Ycamera++;
        }
        persoPast.posX = perso.posX;
        persoPast.posY = perso.posY;
        perso.posY++;
        perso.direction = 3;
    }

    if (actualiserMap() != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int mouvementGauche(void) {
    if (perso.posX > 0 && testSol(perso.posX - 1, perso.posY, 1)) {
        if (Xcamera > 0) {
            Xcamera--;
        }
        persoPast.posX = perso.posX;
        persoPast.posY = perso.posY;
        perso.posX--;
        perso.direction = 2;
    }

    if (actualiserMap() != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int mouvementDroite(void) {
    if (perso.posX < DIMENSION_MAP - 1 && testSol(perso.posX + 1, perso.posY, 3)) {
        if (Xcamera < DIMENSION_MAP - (WINDOW_WIDTH / 100)) {
            Xcamera++;
        }
        persoPast.posX = perso.posX;
        persoPast.posY = perso.posY;
        perso.posX++;
        perso.direction = 4;
    }

    if (actualiserMap() != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}



void save_game(const char *filename) {
    FILE *file = fopen(filename, "w");
    if (file) {
        fprintf(file, "Level: %d, XP: %d\n", perso.lvl, perso.xp);
        fclose(file);
        printf("Game saved!\n");
    } else {
        printf("Error saving game!\n");
    }
}

void afficher_menu(SDL_Renderer *renderer, TTF_Font *font, int highlight, char *choices[], int n_choices) {
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color red = {255, 0, 0, 255};
    SDL_Rect menuRect;

    // Boucle d'affichage des choix du menu
    for (int i = 0; i < n_choices; i++) {
        SDL_Surface *surface = TTF_RenderText_Solid(font, choices[i], (i == highlight) ? red : white);
        SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);

        menuRect.x = 200;  // Position horizontale
        menuRect.y = 150 + (i * 50);  // Position verticale
        menuRect.w = 400;
        menuRect.h = 50;
        
        SDL_RenderCopy(renderer, texture, NULL, &menuRect);
        SDL_DestroyTexture(texture);
    }
    SDL_RenderPresent(renderer);
}

int pause() {
    SDL_bool continuer = SDL_TRUE;
    SDL_Event event;

    if (TTF_Init() == -1) {
    SDL_Log("Erreur lors de l'initialisation de SDL_ttf : %s\n", TTF_GetError());
    return EXIT_FAILURE;
    }

    // Dimensions de la fenêtre du menu pause
    int menu_width = 400;
    int menu_height = 300;
    int button_width = 200;
    int button_height = 50;
    int spacing = 20;

    // Définir les couleurs
    SDL_Color backgroundColor = {50, 50, 50, 255};  // Gris foncé
    SDL_Color buttonColor = {100, 100, 100, 255};   // Gris clair
    SDL_Color buttonHoverColor = {150, 150, 150, 255};  // Couleur des boutons survolés
    SDL_Color textColor = {255, 255, 255, 255};  // Blanc pour le texte

    // Charger une police pour le texte
    TTF_Font *font = TTF_OpenFont("./src/fonts/SPACEBAR.ttf", 32);
    if (!font) {
        SDL_Log("Erreur lors du chargement de la police : %s\n", TTF_GetError());
        return EXIT_FAILURE;
    }

    // Positions des boutons
    SDL_Rect button_resume = {(WINDOW_WIDTH - button_width) / 2, (WINDOW_HEIGHT - menu_height) / 2 + 50, button_width, button_height};
    SDL_Rect button_save = {(WINDOW_WIDTH - button_width) / 2, button_resume.y + button_height + spacing, button_width, button_height};
    SDL_Rect button_quit = {(WINDOW_WIDTH - button_width) / 2, button_save.y + button_height + spacing, button_width, button_height};

    // Créer les surfaces et textures pour le texte des boutons
    SDL_Surface *textPauseSurface = TTF_RenderText_Solid(font, "Menu Pause", textColor);
    SDL_Surface *textResumeSurface = TTF_RenderText_Solid(font, "Reprendre", textColor);
    SDL_Surface *textSaveSurface = TTF_RenderText_Solid(font, "Sauvegarder", textColor);
    SDL_Surface *textQuitSurface = TTF_RenderText_Solid(font, "Quitter", textColor);

    SDL_Texture *textPauseTexture = SDL_CreateTextureFromSurface(renderer, textPauseSurface);
    SDL_Texture *textResumeTexture = SDL_CreateTextureFromSurface(renderer, textResumeSurface);
    SDL_Texture *textSaveTexture = SDL_CreateTextureFromSurface(renderer, textSaveSurface);
    SDL_Texture *textQuitTexture = SDL_CreateTextureFromSurface(renderer, textQuitSurface);

    SDL_FreeSurface(textPauseSurface);
    SDL_FreeSurface(textResumeSurface);
    SDL_FreeSurface(textSaveSurface);
    SDL_FreeSurface(textQuitSurface);

    SDL_Rect textPauseRect = {(WINDOW_WIDTH - 200) / 2, (WINDOW_HEIGHT - menu_height) / 2, 200, 50};

    // Variables pour savoir si un bouton est survolé
    SDL_bool hover_resume = SDL_FALSE;
    SDL_bool hover_save = SDL_FALSE;
    SDL_bool hover_quit = SDL_FALSE;

    while (continuer) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    return EXIT_FAILURE;

                case SDL_MOUSEMOTION:
                    // Vérifier si la souris survole les boutons
                    if (event.motion.x >= button_resume.x && event.motion.x <= button_resume.x + button_width &&
                        event.motion.y >= button_resume.y && event.motion.y <= button_resume.y + button_height) {
                        hover_resume = SDL_TRUE;
                    } else {
                        hover_resume = SDL_FALSE;
                    }

                    if (event.motion.x >= button_save.x && event.motion.x <= button_save.x + button_width &&
                        event.motion.y >= button_save.y && event.motion.y <= button_save.y + button_height) {
                        hover_save = SDL_TRUE;
                    } else {
                        hover_save = SDL_FALSE;
                    }

                    if (event.motion.x >= button_quit.x && event.motion.x <= button_quit.x + button_width &&
                        event.motion.y >= button_quit.y && event.motion.y <= button_quit.y + button_height) {
                        hover_quit = SDL_TRUE;
                    } else {
                        hover_quit = SDL_FALSE;
                    }
                    break;

                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        // Vérifier quel bouton a été cliqué
                        if (hover_resume) {
                            Mix_PlayChannel(-1, sonHover, 0);
                            continuer = SDL_FALSE;  // Reprendre la partie
                        } else if (hover_save) {
                            Mix_PlayChannel(-1, sonHover, 0);
                            // Fonction de sauvegarde (à implémenter)
                            SDL_Log("Sauvegarde en cours...");
                        } else if (hover_quit) {
                            Mix_PlayChannel(-1, sonHover, 0);
                            return EXIT_FAILURE;  // Quitter le jeu
                        }
                    }
                    break;

                default:
                    break;
            }
        }

        // Dessiner l'arrière-plan du menu pause
        SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
        SDL_RenderClear(renderer);

        // Dessiner les boutons
        SDL_SetRenderDrawColor(renderer, buttonColor.r, buttonColor.g, buttonColor.b, buttonColor.a);
        
        // Résumé bouton
        if (hover_resume) {
            SDL_SetRenderDrawColor(renderer, buttonHoverColor.r, buttonHoverColor.g, buttonHoverColor.b, buttonHoverColor.a);
        }
        SDL_RenderFillRect(renderer, &button_resume);
        SDL_RenderCopy(renderer, textResumeTexture, NULL, &button_resume);

        // Sauvegarder bouton
        SDL_SetRenderDrawColor(renderer, buttonColor.r, buttonColor.g, buttonColor.b, buttonColor.a);
        if (hover_save) {
            SDL_SetRenderDrawColor(renderer, buttonHoverColor.r, buttonHoverColor.g, buttonHoverColor.b, buttonHoverColor.a);
        }
        SDL_RenderFillRect(renderer, &button_save);
        SDL_RenderCopy(renderer, textSaveTexture, NULL, &button_save);

        // Quitter bouton
        SDL_SetRenderDrawColor(renderer, buttonColor.r, buttonColor.g, buttonColor.b, buttonColor.a);
        if (hover_quit) {
            SDL_SetRenderDrawColor(renderer, buttonHoverColor.r, buttonHoverColor.g, buttonHoverColor.b, buttonHoverColor.a);
        }
        SDL_RenderFillRect(renderer, &button_quit);
        SDL_RenderCopy(renderer, textQuitTexture, NULL, &button_quit);

        // Afficher le texte "Menu Pause"
        SDL_RenderCopy(renderer, textPauseTexture, NULL, &textPauseRect);

        // Présenter le rendu
        SDL_RenderPresent(renderer);
    }

    // Nettoyage des textures
    SDL_DestroyTexture(textPauseTexture);
    SDL_DestroyTexture(textResumeTexture);
    SDL_DestroyTexture(textSaveTexture);
    SDL_DestroyTexture(textQuitTexture);
    TTF_CloseFont(font);

    return EXIT_SUCCESS;
}

void lancerBoucleDeJeu(int argc, char **argv) {
    SDL_bool continuer = SDL_TRUE;
    SDL_Event event;
    unsigned int frame_limit = 0;
    unsigned int dernier_deplacement_monstres = 0;  // Pour gérer le délai entre chaque déplacement de monstres

    while (continuer) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
                        case SDLK_e:
                        case SDLK_SPACE:
                            if (attaqueEpee() != EXIT_SUCCESS) {
                                SDL_ExitWithError("Erreur attaque");
                            }
                            break;

                        case SDLK_z:
                        case SDLK_UP:
                            if (mouvementHaut() != EXIT_SUCCESS) {
                                SDL_ExitWithError("Erreur mouvement haut");
                            }
                            break;

                        case SDLK_q:
                        case SDLK_LEFT:
                            if (mouvementGauche() != EXIT_SUCCESS) {
                                SDL_ExitWithError("Erreur mouvement gauche");
                            }
                            break;

                        case SDLK_s:
                        case SDLK_DOWN:
                            if (mouvementBas() != EXIT_SUCCESS) {
                                SDL_ExitWithError("Erreur mouvement bas");
                            }
                            break;

                        case SDLK_d:
                        case SDLK_RIGHT:
                            if (mouvementDroite() != EXIT_SUCCESS) {
                                SDL_ExitWithError("Erreur mouvement droite");
                            }
                            break;

                        case SDLK_ESCAPE:
                            Mix_PlayChannel(-1, sonMenu, 0);
                            if (pause() != EXIT_SUCCESS) {
                                continuer = SDL_FALSE;  // Quitter si le joueur sélectionne "Quitter"
                            }
                            break;

                        default:
                            break;
                    }
                    break;

                case SDL_QUIT:
                    continuer = SDL_FALSE;
                    break;

                default:
                    break;
            }
        }

        // Gérer l'invulnérabilité du joueur (5 secondes)
        if (perso.invulnerable && SDL_GetTicks() - perso.invulnerable_timer > 5000) {
            perso.invulnerable = SDL_FALSE;  // Fin de l'invulnérabilité après 5 secondes
        }

        // Déplacer les monstres toutes les 1 seconde
        if (SDL_GetTicks() - dernier_deplacement_monstres > 1000) {
            deplacerMonstresVersJoueur();
            dernier_deplacement_monstres = SDL_GetTicks();  // Réinitialiser le timer des déplacements
        }

        // Mettre à jour la caméra et afficher le rendu
        if (camera(argc, argv) != EXIT_SUCCESS) {
            SDL_ExitWithError("Problème fonction camera");
        }

        // Limiter les FPS
        frame_limit = SDL_GetTicks() + FPS_LIMIT;
        SDL_LimitFPS(frame_limit);
    }
}
