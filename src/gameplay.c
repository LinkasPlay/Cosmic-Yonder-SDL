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
extern Mix_Chunk * swordSound;
extern Mix_Chunk *gunSound;
extern Mix_Chunk *mobHurtSound1;
extern Mix_Chunk *mobDeadSound1;
extern Mix_Chunk *sonLevelUp;
extern Mix_Chunk *sonPotionLife;
extern Mix_Chunk *sonPotionXP;
extern Mix_Chunk *sonWrench;
extern Mix_Chunk *sonKey;
extern Mix_Chunk *sonBigKey;
extern Mix_Chunk *sonFail;
extern Mix_Chunk *sonSwordPickup;

extern TTF_Font *fontUI;

extern tile **map;

extern int graine;

extern int Xcamera;
extern int Ycamera;

char inventaireIcons[7][64] = {
    "src/image/inventaireVide.bmp", // Par défaut, emplacement 1 est vide
    "src/image/inventaireVide.bmp", // Emplacement 2
    "src/image/inventairePotion.bmp", // Emplacement 3
    "src/image/inventaireXp.bmp", // Emplacement 4
    "src/image/inventaireWrench.bmp", // Emplacement 5
    "src/image/inventaireKey.bmp", // Emplacement 6
    "src/image/inventaireBossKey.bmp" // Emplacement 7
};

personnage perso;
personnage persoPast;

extern int texture(int argc, char **argv);
extern int creeMap(void);
extern int actualiserMap(void);
extern int nouvelleSalle(int longueur, int largeur, int num_salle, int cote);
extern int genererSalleValide(int longueurInitiale, int largeurInitiale, int num_salle, int cote);
extern unsigned int aleatoire(int min, int max);
extern void save_game(const char *filename);
extern int camera(int argc, char **argv);
void afficherInterface(SDL_Renderer *renderer, TTF_Font *font, int selectedSlot);

extern void SDL_LimitFPS(unsigned int limit);

extern int num_salle;

unsigned int timer_start = 180000; // 3 minutes en millisecondes
unsigned int start_time = 0;

int caseSize = 64;  // Taille d'une case
int startX = (WINDOW_WIDTH - (7 * 64)) / 2;  // Centrer l'inventaire
int machineFuite = 0;

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

void lvlMaj(monstre mstr) {
    // Ajouter l'XP gagnée par le joueur
    if (perso.lvl < 20) { // Niveau maximum est 20
        perso.xp += mstr.xp;

        while (perso.xp >= perso.xpMax) {  // Vérifie si le joueur gagne plusieurs niveaux d'un coup
            perso.xp -= perso.xpMax;  // Soustrait l'XP nécessaire au niveau actuel
            perso.lvl++;  // Augmente le niveau
            Mix_PlayChannel(CHANNEL_LEVEL_UP, sonLevelUp, 0); // Son de montée de niveau

            // Buffs à chaque niveau
            timer_start += 30000; // Ajoute 30 secondes au chrono

            // Buffs progressifs
            if (perso.lvl % 5 == 0) { // Tous les 5 niveaux
                perso.sword_damage += 10; // Augmente les dégâts de l'épée
                perso.gun_damage += 5;    // Augmente les dégâts du pistolet
            }
            if (perso.lvl % 10 == 0) { // Tous les 10 niveaux
                perso.hpMax += 1;      // Ajoute 1 PV max au joueur
                perso.hp = perso.hpMax; // Remplit les PV actuels
            }
            if (perso.lvl % 2 == 0) { // Tous les 2 niveaux
                perso.inv[2]++; // Ajoute une potion d'XP comme bonus
                perso.inv[3]++; // Ajoute une potion d'XP comme bonus
                perso.inv[4]++; // Ajoute une potion d'XP comme bonus
                perso.inv[5]++; // Ajoute une potion d'XP comme bonus
                perso.inv[6]++; // Ajoute une potion d'XP comme bonus
            }

            // Stopper l'XP au niveau 20
            if (perso.lvl == 20) {
                perso.xp = 0;
                SDL_Log("Niveau maximum atteint !");
                break;
            }

            // Mise à jour de l'XP maximum pour le niveau suivant
            perso.xpMax = 100 * (perso.lvl * 0.9);
            SDL_Log("Niveau augmenté : %d, XP max : %d, Dégâts épée : %d, Dégâts pistolet : %d, PV max : %d",
                    perso.lvl, perso.xpMax, perso.sword_damage, perso.gun_damage, perso.hpMax);
        }
    } else {
        SDL_Log("Niveau maximum atteint, XP non ajoutée.");
    }
}

int attaquePistolet() {
    Mix_PlayChannel(CHANNEL_GUN, gunSound, 0);

    int dx = 0, dy = 0;
    switch (perso.direction) {
        case 1: dy = -1; break; // Haut
        case 3: dy = 1;  break; // Bas
        case 2: dx = -1; break; // Gauche
        case 4: dx = 1;  break; // Droite
        default:
            SDL_Log("Orientation invalide");
            return EXIT_FAILURE;
    }

    int x = perso.posX + dx;
    int y = perso.posY + dy;

    while (x >= 0 && x < DIMENSION_MAP && y >= 0 && y < DIMENSION_MAP) {
        if (map[x][y].contenu == -2) break; // Mur

        if (map[x][y].contenu == 2) { // Monstre
            map[x][y].mstr.hp -= perso.gun_damage;

            if (map[x][y].mstr.hp <= 0) {
                // Monstre tué
                lvlMaj(map[x][y].mstr);
                map[x][y].contenu = 0;  // Vider la case
                Mix_PlayChannel(CHANNEL_MONSTER_DEAD, mobDeadSound1, 0);
            } else {
                Mix_PlayChannel(CHANNEL_MONSTER_HURT, mobHurtSound1, 0);
            }
            break; // Arrêter le tir après avoir touché un monstre
        }

        x += dx;
        y += dy;
    }

    return EXIT_SUCCESS;
}

int attaqueEpee() {
    // Jouer le son de l'épée sur le canal dédié
    Mix_PlayChannel(CHANNEL_SWORD, swordSound, 0);

    // Définir les directions d'attaque en fonction de l'orientation du joueur
    int directions[5][2];
    switch (perso.direction) {
        case 1: // Haut
            directions[0][0] = -1; directions[0][1] = 0;   // Gauche
            directions[1][0] = -1; directions[1][1] = -1;  // Devant à gauche
            directions[2][0] = 0;  directions[2][1] = -1;  // Devant
            directions[3][0] = 1;  directions[3][1] = -1;  // Devant à droite
            directions[4][0] = 1;  directions[4][1] = 0;   // Droite
            break;
        case 3: // Bas
            directions[0][0] = -1; directions[0][1] = 0;   
            directions[1][0] = -1; directions[1][1] = 1;   
            directions[2][0] = 0;  directions[2][1] = 1;   
            directions[3][0] = 1;  directions[3][1] = 1;   
            directions[4][0] = 1;  directions[4][1] = 0;   
            break;
        case 2: // Gauche
            directions[0][0] = 0;  directions[0][1] = -1;  
            directions[1][0] = -1; directions[1][1] = -1;  
            directions[2][0] = -1; directions[2][1] = 0;   
            directions[3][0] = -1; directions[3][1] = 1;   
            directions[4][0] = 0;  directions[4][1] = 1;   
            break;
        case 4: // Droite
            directions[0][0] = 0;  directions[0][1] = -1;  
            directions[1][0] = 1;  directions[1][1] = -1;  
            directions[2][0] = 1;  directions[2][1] = 0;   
            directions[3][0] = 1;  directions[3][1] = 1;   
            directions[4][0] = 0;  directions[4][1] = 1;   
            break;
        default:
            SDL_Log("Orientation invalide");
            return EXIT_FAILURE;
    }

    // Appliquer les dégâts et gérer le recul
    for (int i = 0; i < 5; i++) {
        int targetX = perso.posX + directions[i][0];
        int targetY = perso.posY + directions[i][1];

        // Vérifier si les coordonnées sont valides
        if (targetX >= 0 && targetX < DIMENSION_MAP && targetY >= 0 && targetY < DIMENSION_MAP) {
            if (map[targetX][targetY].contenu == 2) { // Monstre détecté
                map[targetX][targetY].mstr.hp -= perso.sword_damage;

                if (map[targetX][targetY].mstr.hp <= 0) {
                    // Le monstre meurt
                    lvlMaj(map[targetX][targetY].mstr); // Ajout d'expérience
                    map[targetX][targetY].contenu = 0;  // Vider la case
                    Mix_PlayChannel(CHANNEL_MONSTER_DEAD, mobDeadSound1, 0);  
                } else {
                    // Le monstre recule
                    int reculeX = targetX + directions[i][0];
                    int reculeY = targetY + directions[i][1];

                    if (reculeX >= 0 && reculeX < DIMENSION_MAP && reculeY >= 0 && reculeY < DIMENSION_MAP && map[reculeX][reculeY].contenu == 0) {
                        map[reculeX][reculeY] = map[targetX][targetY]; // Déplacer le monstre
                        map[targetX][targetY].contenu = 0;            // Vider l'ancienne case
                    }
                    Mix_PlayChannel(CHANNEL_MONSTER_HURT, mobHurtSound1, 0);
                }
            }
        }
    }

    return EXIT_SUCCESS;
}

int degatMonstre(int dmg, monstre mstr) {
    mstr.hp -= dmg;
    Mix_PlayChannel(-1, mobHurtSound1, 0);  // Jouer le son des dégâts subis par le monstre

    if (mstr.hp > 0) {
        //mstr.frameAnimation = -5;
    } else {
        //mstr.frameAnimation = -10;
        lvlMaj(mstr);
        if (mstr.loot > 1) {
            perso.inv[mstr.loot]++;
        }
        Mix_PlayChannel(-1, mobDeadSound1, 0);  // Jouer le son de la mort du monstre
    }
    return 0;
}

void deplacerMonstresVersJoueur() {
    // Créer une copie de la map pour suivre les changements sans perturber les autres ennemis
    tile **new_map = malloc(sizeof(tile *) * DIMENSION_MAP);
    for (int i = 0; i < DIMENSION_MAP; i++) {
        new_map[i] = malloc(sizeof(tile) * DIMENSION_MAP);
        for (int j = 0; j < DIMENSION_MAP; j++) {
            new_map[i][j] = map[i][j];
        }
    }

    for (int i = 0; i < DIMENSION_MAP; i++) {
        for (int j = 0; j < DIMENSION_MAP; j++) {
            if (map[i][j].contenu == 2) {  // Si la case contient un monstre
                int dx = 0, dy = 0;

                // Calculer la différence de position entre le monstre et le joueur
                int diffX = perso.posX - i;
                int diffY = perso.posY - j;

                // Priorité : se déplacer horizontalement si la différence est plus grande en X, sinon verticalement
                if (abs(diffX) > abs(diffY)) {
                    dx = (diffX > 0) ? 1 : -1;  // Vers la droite ou la gauche
                } else {
                    dy = (diffY > 0) ? 1 : -1;  // Vers le bas ou le haut
                }

                // Nouvelle position du monstre
                int new_x = i + dx;
                int new_y = j + dy;

                // Vérifier que la nouvelle case est libre (ni mur, ni monstre)
                if (new_map[new_x][new_y].contenu == 0) {
                    // Déplacer le monstre vers la nouvelle case
                    new_map[new_x][new_y].contenu = 2;
                    new_map[new_x][new_y].mstr = map[i][j].mstr;

                    // Vider l'ancienne case
                    new_map[i][j].contenu = 0;
                    new_map[i][j].mstr.hp = 0;
                } 
                // Si le monstre atteint le joueur
                else if (new_x == perso.posX && new_y == perso.posY) {
                    if (!perso.invulnerable) {
                        perso.hp--;
                        Mix_PlayChannel(CHANNEL_DAMAGE, sonDamage, 0);
                        if (perso.hp <= 0) {
                            gameOver();
                            return;
                        }
                        perso.invulnerable = SDL_TRUE;
                        perso.invulnerable_timer = SDL_GetTicks();
                    }
                }
            }
        }
    }

    // Mettre à jour la map avec les nouvelles positions
    for (int i = 0; i < DIMENSION_MAP; i++) {
        for (int j = 0; j < DIMENSION_MAP; j++) {
            map[i][j] = new_map[i][j];
        }
        free(new_map[i]);
    }
    free(new_map);
}

void utiliserObjet(int slot) {
    if (perso.inv[slot] <= 0) {
        Mix_PlayChannel(CHANNEL_FAIL, sonFail, 0); // Son d'échec
        SDL_Log("Échec : Aucun objet disponible dans le slot %d", slot);
        return;
    }

    switch (slot) {
        case 0: // Épée
            if (attaqueEpee() == EXIT_SUCCESS) {
            }
            break;

        case 1: // Pistolet
            if (attaquePistolet() == EXIT_SUCCESS) {
            }
            break;

        case 2: // Potion de vie
            if (perso.hp < perso.hpMax) {
                perso.hp += 2;
                if (perso.hp > perso.hpMax) {
                    perso.hp = perso.hpMax;
                }
                Mix_PlayChannel(CHANNEL_POTION_LIFE, sonPotionLife, 0); // Son pour potion de vie
                SDL_Log("Potion de vie utilisée. HP : %d", perso.hp);
                perso.inv[slot]--;
            } else {
                Mix_PlayChannel(CHANNEL_FAIL, sonFail, 0); // Son d'échec
                SDL_Log("Potion de vie non utilisée : HP déjà au maximum.");
            }
            break;

        case 3: // Potion d'XP
            perso.xp += perso.xpMax / 4; // Ajout d'XP
            Mix_PlayChannel(CHANNEL_POTION_XP, sonPotionXP, 0); // Son pour potion d'XP
            SDL_Log("Potion d'XP utilisée. XP : %d", perso.xp);
            perso.inv[slot]--;
            break;

        case 4: // Clé à molette
            Mix_PlayChannel(CHANNEL_WRENCH, sonWrench, 0); // Placeholder pour clé à molette
            SDL_Log("Clé à molette utilisée. Vérifiez les interactions.");
            break;

        case 5: // Clés
            Mix_PlayChannel(CHANNEL_KEY, sonKey, 0); // Placeholder pour clé normale
            SDL_Log("Clé utilisée. Vérifiez les interactions.");
            break;

        case 6: // Grande clé
            Mix_PlayChannel(CHANNEL_BIG_KEY, sonBigKey, 0); // Placeholder pour grande clé
            SDL_Log("Grande clé utilisée. Vérifiez les interactions.");
            break;

        default:
            Mix_PlayChannel(CHANNEL_FAIL, sonFail, 0); // Son d'échec
            SDL_Log("Aucun objet sélectionné ou slot invalide.");
            break;
    }
}

void interagirEpee() {
    // Calculer les coordonnées de la case devant le joueur
    int x = perso.posX;
    int y = perso.posY;

    switch (perso.direction) {
        case 1: // Haut
            y--;
            break;
        case 2: // Gauche
            x--;
            break;
        case 3: // Bas
            y++;
            break;
        case 4: // Droite
            x++;
            break;
        default:
            SDL_Log("Direction invalide du joueur.");
            return;
    }

    // Vérifier si la case devant le joueur contient une épée
    if (map[x][y].contenu == 3 && map[x][y].spe.type == 8) {
            perso.inv[0] = 1; // Ajouter l'épée à l'inventaire (emplacement 1)
            map[x][y].contenu = 0; // Supprimer l'épée de la carte
            Mix_PlayChannel(CHANNEL_SWORD_PICKUP, sonSwordPickup, 0); // Jouer le son de prise d'épée
            // Mettre à jour l'icône de l'emplacement 1 de l'inventaire
            strcpy(inventaireIcons[0], "src/image/inventaireEpee.bmp");
    }
}

void ouvrirCoffre(int x, int y) {
    if (perso.inv[5] > 0) { // Vérifie si le joueur a des clés
        perso.inv[5]--; // Consomme une clé
        Mix_PlayChannel(CHANNEL_KEY, sonKey, 0);

        // Ajouter des récompenses
        int proba = aleatoire(1, 100);
        if (proba <= 40) {
            perso.inv[4]++; // Clé à molette
        } else if (proba <= 70) {
            perso.inv[2]++; // Potion de vie
        } else {
            perso.inv[3]++; // Potion d'XP
        }

        if (++perso.nbCoffres == 10) {
            perso.inv[1]++; // Pistolet
        } else if (perso.nbCoffres == 20) {
            perso.inv[6]++; // Clé du boss
        }

        // Vider le coffre
        map[x][y].contenu = 3;
        map[x][y].spe.type = 5; // Coffre ouvert
    } else {
        SDL_Log("Vous n'avez pas de clé !");
    }
}

void reparerMachine(int x, int y) {
    if (perso.inv[4] > 0) { // Vérifie si le joueur a une clé à molette
        perso.inv[4]--; // Consomme une clé à molette
        timer_start += 180000; // Ajoute 3 minutes
        Mix_PlayChannel(CHANNEL_WRENCH, sonWrench, 0);

        // Marque la machine comme réparée
        map[x][y].contenu = 3;
        map[x][y].spe.type = 6; // Machine réparée
    } else {
        SDL_Log("Vous n'avez pas de clé à molette !");
    }
}

void reparerGrandeMachine(int x, int y) {
    if (perso.inv[4] > 0) { // Vérifie si le joueur a une clé à molette
        perso.inv[4]--; // Consomme une clé à molette
        if (map[x][y].spe.type == 3) {
            map[x][y].spe.type = 4; // Grande machine en réparation
            Mix_PlayChannel(CHANNEL_WRENCH, sonWrench, 0);
        } else if (map[x][y].spe.type == 4) {
            map[x][y].spe.type = 7; // Grande machine réparée
            machineFuite++;
            Mix_PlayChannel(CHANNEL_WRENCH, sonWrench, 0);
        }
    } else {
        SDL_Log("Vous n'avez pas de clé à molette !");
    }
}


void interagirEnvironnement() {
    int x = perso.posX;
    int y = perso.posY;

    // Déterminer la case devant le joueur
    switch (perso.direction) {
        case 1: // Haut
            y--;
            break;
        case 2: // Gauche
            x--;
            break;
        case 3: // Bas
            y++;
            break;
        case 4: // Droite
            x++;
            break;
        default:
            SDL_Log("Direction invalide du joueur.");
            return;
    }

    // Interaction avec l'épée
    if (map[x][y].spe.type == 8) {
        interagirEpee(x, y);
    }
    // Interaction avec un coffre
    else if (map[x][y].contenu == 3 && map[x][y].spe.type == 1) {
        ouvrirCoffre(x, y);
    }
    // Interaction avec une machine
    else if (map[x][y].contenu == 3 && map[x][y].spe.type == 2) {
        reparerMachine(x, y);
    }
    // Interaction avec une grande machine
    else if (map[x][y].contenu == 3 && (map[x][y].spe.type == 3 || map[x][y].spe.type == 4)) {
        reparerGrandeMachine(x, y);
    } else {
        SDL_Log("Rien à interagir ici.");
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
            int largeur = aleatoire(5, 11);
            int longueur = aleatoire(5, 11);

            // Générer la nouvelle salle
            if (genererSalleValide(largeur, longueur, num_salle, cote) != EXIT_SUCCESS) {
                printf("Erreur lors de la génération de la salle\n");
                
                // Placer un mur à l'emplacement de la porte
                int murX = perso.posX;
                int murY = perso.posY;

                switch (cote) {
                    case 0: // Haut
                        murY = perso.posY - 1;
                        break;
                    case 1: // Gauche
                        murX = perso.posX - 1;
                        break;
                    case 2: // Bas
                        murY = perso.posY + 1;
                        break;
                    case 3: // Droite
                        murX = perso.posX + 1;
                        break;
                    default:
                        break;
                }

                // Vérifier si les coordonnées du mur sont valides
                if (murX >= 0 && murX < DIMENSION_MAP && murY >= 0 && murY < DIMENSION_MAP) {
                    map[murX][murY].contenu = -2; // Placer un mur
                    printf("Mur placé aux coordonnées (%d, %d)\n", murX, murY);
                }

                // Retourner 0 pour indiquer qu'aucun mouvement ne doit être effectué
                return 0;
            }

            // Déplacement du joueur si la salle a été générée avec succès
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
            Mix_PlayChannel(CHANNEL_ROOM, sonRoom, 0);
            return 1;
            break;

        case 2:  // Monstre
            if (!perso.invulnerable) {
                perso.hp--;
                Mix_PlayChannel(CHANNEL_DAMAGE, sonDamage, 0);
                // Suppression du monstre
                map[x][y].contenu = 0;
                map[x][y].mstr.hp = 0;

                perso.invulnerable = SDL_TRUE;
                perso.invulnerable_timer = SDL_GetTicks();
            }

            if (perso.hp <= 0) {
                gameOver();
            }

            return 1;
            break;

        case 0: // Case vide
            Mix_PlayChannel(CHANNEL_MOVE, sonMove, 0);
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
                            Mix_PlayChannel(CHANNEL_HOVER, sonHover, 0);
                            continuer = SDL_FALSE;  // Reprendre la partie
                        } else if (hover_save) {
                            Mix_PlayChannel(CHANNEL_HOVER, sonHover, 0);
                            // Fonction de sauvegarde (à implémenter)
                            SDL_Log("Sauvegarde en cours...");
                        } else if (hover_quit) {
                            Mix_PlayChannel(CHANNEL_HOVER, sonHover, 0);
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

void dessinerRectangleSelection(SDL_Renderer *renderer, int selectedSlot) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Couleur noire
    SDL_Rect rect = {
        .x = startX - 10,
        .y = WINDOW_HEIGHT - 100 + caseSize + 5,
        .w = caseSize * 8,
        .h = 30
    };
    SDL_RenderFillRect(renderer, &rect);
    SDL_RenderPresent(renderer); // Assurez-vous d'actualiser l'écran
}

void lancerBoucleDeJeu(int argc, char **argv) {
    start_time = SDL_GetTicks();

    SDL_bool continuer = SDL_TRUE;
    SDL_Event event;
    unsigned int frame_limit = 0;
    unsigned int dernier_deplacement_monstres = 0;  // Pour gérer le délai entre chaque déplacement de monstres

    // Variable pour l'emplacement d'inventaire sélectionné
    int selectedSlot = 0;  // Par défaut, le premier emplacement est sélectionné

    // Charger la police pour l'interface
    TTF_Font *font = TTF_OpenFont("./src/fonts/ui.otf", 24);
    if (!font) {
        SDL_Log("Erreur lors du chargement de la police pour l'interface : %s", TTF_GetError());
        return;
    }

    while (continuer) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    continuer = SDL_FALSE;
                    break;

                case SDL_KEYDOWN:
                    // Gestion des touches numériques pour sélectionner un emplacement d'inventaire
                    if (event.key.keysym.sym >= SDLK_1 && event.key.keysym.sym <= SDLK_7) {
                        selectedSlot = event.key.keysym.sym - SDLK_1;  // Sélection de 0 à 6
                    }

                    // Gestion des autres touches (mouvement, interaction, utilisation d'objet)
                    switch (event.key.keysym.sym) {
                        case SDLK_w: // Molette haut (équivalent)
                            selectedSlot = (selectedSlot + 1) % 7; // Passer au slot suivant
                            if (selectedSlot < 0) selectedSlot = 6; // Correction pour éviter un slot négatif
                            dessinerRectangleSelection(renderer, selectedSlot);
                            break;

                        case SDLK_x: // Molette bas (équivalent)
                            selectedSlot = (selectedSlot - 1 + 7) % 7; // Passer au slot précédent
                            dessinerRectangleSelection(renderer, selectedSlot);
                            break;

                        case SDLK_e: // Utilisation d'un objet
                            utiliserObjet(selectedSlot);
                            break;

                        case SDLK_SPACE:
                        case SDLK_a: // Interaction avec l'environnement
                            interagirEnvironnement();
                            break;

                        case SDLK_z:
                        case SDLK_UP: // Déplacement haut
                            if (mouvementHaut() != EXIT_SUCCESS) {
                                SDL_ExitWithError("Erreur mouvement haut");
                            }
                            break;

                        case SDLK_q:
                        case SDLK_LEFT: // Déplacement gauche
                            if (mouvementGauche() != EXIT_SUCCESS) {
                                SDL_ExitWithError("Erreur mouvement gauche");
                            }
                            break;

                        case SDLK_s:
                        case SDLK_DOWN: // Déplacement bas
                            if (mouvementBas() != EXIT_SUCCESS) {
                                SDL_ExitWithError("Erreur mouvement bas");
                            }
                            break;

                        case SDLK_d:
                        case SDLK_RIGHT: // Déplacement droite
                            if (mouvementDroite() != EXIT_SUCCESS) {
                                SDL_ExitWithError("Erreur mouvement droite");
                            }
                            break;

                        case SDLK_ESCAPE: // Pause
                            Mix_PlayChannel(CHANNEL_MENU, sonMenu, 0);
                            if (pause() != EXIT_SUCCESS) {
                                continuer = SDL_FALSE;  // Quitter si le joueur sélectionne "Quitter"
                            }
                            break;

                        default:
                            break;
                    }
                    break;

                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        utiliserObjet(selectedSlot); // Utiliser l'objet sélectionné
                    } else if (event.button.button == SDL_BUTTON_RIGHT) {
                        interagirEnvironnement(); // Interaction avec clic droit
                    }
                    break;

                case SDL_MOUSEWHEEL: // Molette pour changer de case
                    if (event.wheel.y > 0) {
                        selectedSlot = (selectedSlot + 1) % 7;
                        dessinerRectangleSelection(renderer, selectedSlot);
                    } else if (event.wheel.y < 0) {
                        selectedSlot = (selectedSlot - 1 + 7) % 7;
                        dessinerRectangleSelection(renderer, selectedSlot);
                    }
                    break;

                default:
                    break;
            }
        }

        // Déplacer les monstres toutes les secondes
        unsigned int temps_courant = SDL_GetTicks();
        if (temps_courant - dernier_deplacement_monstres >= 1000) {
            deplacerMonstresVersJoueur();
            dernier_deplacement_monstres = temps_courant;
        }

        // Vérifier si l'invulnérabilité a expiré
        if (perso.invulnerable && (SDL_GetTicks() - perso.invulnerable_timer >= 3000)) {
            perso.invulnerable = SDL_FALSE;
        }

        // Afficher le jeu (rendu de la carte, caméra, etc.)
        if (camera(argc, argv) != EXIT_SUCCESS) {
            SDL_ExitWithError("Problème fonction camera");
        }

        // Afficher l'interface utilisateur
        afficherInterface(renderer, font, selectedSlot);

        // Limiter les FPS
        frame_limit = SDL_GetTicks() + FPS_LIMIT;
        SDL_LimitFPS(frame_limit);
    }

    // Nettoyage de la police
    TTF_CloseFont(font);
}