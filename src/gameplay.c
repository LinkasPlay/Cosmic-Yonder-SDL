#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <time.h>

#include "CosmicYonder.h"

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
        niv 2 : 180 pv, 20 xp | drop : clé * 0.1
        niv 3 : 260 pv, 30 xp | drop : clé * 0.2, clé a molette * 0.1
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
extern Mix_Chunk *sonItem;
extern Mix_Chunk *mobBossSound;

extern TTF_Font *fontUI;

extern tile **map;

extern int graine;

extern int Xcamera;
extern int Ycamera;

char inventaireIcons[7][64] = {
    "src/image/inventaireVide.bmp", // Par défaut, emplacement 1 est vide
    "src/image/inventaireVide.bmp", // Emplacement 2
    "src/image/inventaireVide.bmp", // Emplacement 3
    "src/image/inventaireVide.bmp", // Emplacement 4
    "src/image/inventaireVide.bmp", // Emplacement 5
    "src/image/inventaireVide.bmp", // Emplacement 6
    "src/image/inventaireVide.bmp" // Emplacement 7
};

personnage perso;
personnage persoPast;
Boss boss;

extern int texture(int argc, char **argv);
extern int creeMap(void);
extern int actualiserMap(void);
extern int nouvelleSalle(int longueur, int largeur, int num_salle, int cote);
extern int genererSalleValide(int longueurInitiale, int largeurInitiale, int num_salle, int cote);
extern unsigned int aleatoire(int min, int max);
extern void save_game(const char *filename);
extern int camera(int argc, char **argv);
extern int genererSalleBoss(int cote);
void afficherInterface(SDL_Renderer *renderer, TTF_Font *font, int selectedSlot);
extern void credits();

extern void SDL_LimitFPS(unsigned int limit);

extern int num_salle;

unsigned int timer_start = 180000; // 3 minutes en millisecondes
unsigned int start_time = 0;

int caseSize = 64;  // Taille d'une case
int startX = (WINDOW_WIDTH - (7 * 64)) / 2;  // Centrer l'inventaire
int machineFuite = 0;
int quotaPistolet = 10;
int quotaBoss = 30;

SDL_bool bossPresent = SDL_FALSE;

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
                perso.sword_damage += 20; // Augmente les dégâts de l'épée
                perso.gun_damage += 10;    // Augmente les dégâts du pistolet
            }
            if (perso.lvl % 10 == 0) { // Tous les 10 niveaux
                perso.hpMax += 1;      // Ajoute 1 PV max au joueur
                perso.hp = perso.hpMax; // Remplit les PV actuels
            }
            if (perso.lvl % 2 == 0) { // Tous les 2 niveaux
                perso.inv[5]++; // Ajoute une clé comme bonus
                strcpy(inventaireIcons[5], "src/image/inventaireKey.bmp");
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

void verifierInvulnerabiliteBoss() {
    if (boss.invulnerable && (SDL_GetTicks() - boss.invuln_timer >= 500)) {
        boss.invulnerable = SDL_FALSE;
    }
}

int degatBoss(int dmg) {
    verifierInvulnerabiliteBoss();
    if (boss.invulnerable) {
        return 0;
    }

    boss.hp -= dmg;

    // Jouer le son de dégâts du boss
    Mix_PlayChannel(CHANNEL_BOSS_HURT, mobBossSound, 0);

    // Activer l'invulnérabilité temporaire
    boss.invulnerable = SDL_TRUE;
    boss.invuln_timer = SDL_GetTicks();

    // Si les points de vie du boss tombent à 0 ou en dessous
    if (boss.hp <= 0) {
        boss.hp = 0;
        bossPresent = SDL_FALSE;

        // Lancer les crédits
        Mix_HaltMusic(); // Arrêter la musique actuelle
        credits(); // Implémentez cette fonction ultérieurement
    }

    return 0;
}

int degatMonstre(int dmg, monstre *mstr) {
    // Gestion spéciale pour le boss (type 4)
    if (mstr->type == 4) {
        return degatBoss(dmg); // Déléguer à la fonction `degatBoss`
    } else {

        // Gestion classique pour les autres monstres
        mstr->hp -= dmg; // Réduire les points de vie du monstre
        if (mstr->hp > 0) {
            // Le monstre est encore vivant
            Mix_PlayChannel(CHANNEL_MONSTER_HURT, mobHurtSound1, 0); // Jouer le son des dégâts subis par le monstre
        } else {
            // Monstre tué
            if (! bossPresent) {
                lvlMaj(*mstr); // Gérer le gain d'XP et autres récompenses liées au niveau

                // Probabilité de loot
                int proba = aleatoire(1, 100);
                if (mstr->type == 2 && proba <= 20) {
                    // Type 2 : 20% de chance de dropper une clé
                    perso.inv[5]++;
                    strcpy(inventaireIcons[5], "src/image/inventaireKey.bmp");
                    Mix_PlayChannel(CHANNEL_ITEM, sonItem, 0); // Son pour recevoir un loot
                } else if (mstr->type == 3) {
                    if (proba <= 30) {
                        // Type 3 : 30% de chance de dropper une clé
                        perso.inv[5]++;
                        strcpy(inventaireIcons[5], "src/image/inventaireKey.bmp");
                        Mix_PlayChannel(CHANNEL_ITEM, sonItem, 0); // Son pour recevoir un loot
                    } else if (proba <= 40) {
                        // Type 3 : 10% de chance de dropper une clé à molette
                        perso.inv[4]++;
                        strcpy(inventaireIcons[4], "src/image/inventaireWrench.bmp");
                        Mix_PlayChannel(CHANNEL_ITEM, sonItem, 0); // Son pour recevoir un loot
                    }
                }

                
            }
            // Jouer le son de la mort du monstre
            Mix_PlayChannel(-1, mobDeadSound1, 0);
            return 0;
        }
    } 
    return 0;
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
            degatMonstre(perso.gun_damage, &(map[x][y].mstr)); // Call degatMonstre
            if (map[x][y].mstr.hp <= 0) {
                map[x][y].contenu = 0;  // Vider la case si le monstre est mort
            }
            break; // Arrêter le tir après avoir touché un monstre
        }

        x += dx;
        y += dy;
    }

    return EXIT_SUCCESS;
}

int attaqueEpee() {
    Mix_PlayChannel(CHANNEL_SWORD, swordSound, 0);

    // Définir les directions d'attaque en fonction de l'orientation du joueur
    int directions[5][2];
    switch (perso.direction) {
        case 1: // Haut
            directions[0][0] = -1; directions[0][1] = 0;
            directions[1][0] = -1; directions[1][1] = -1;
            directions[2][0] = 0;  directions[2][1] = -1;
            directions[3][0] = 1;  directions[3][1] = -1;
            directions[4][0] = 1;  directions[4][1] = 0;
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

    // Appliquer les dégâts aux cases cibles
for (int i = 0; i < 5; i++) {
    int targetX = perso.posX + directions[i][0];
    int targetY = perso.posY + directions[i][1];

    // Vérifier si les coordonnées sont valides
    if (targetX >= 0 && targetX < DIMENSION_MAP && targetY >= 0 && targetY < DIMENSION_MAP) {
        if (map[targetX][targetY].contenu == 2) { // Monstre détecté
            // Passer un pointeur sur le monstre pour mettre à jour ses valeurs directement
            monstre *targetMonstre = &map[targetX][targetY].mstr;

            degatMonstre(perso.sword_damage, targetMonstre); // Appeler avec pointeur

            if (targetMonstre->hp <= 0) {
                map[targetX][targetY].contenu = 0; // Vider la case si le monstre est mort
            } else if ( targetMonstre->type != 4) {
                // Calculer la direction opposée au joueur pour le recul
                int reculX = targetX + directions[i][0];
                int reculY = targetY + directions[i][1];

                // Vérifier si la case de recul est valide et vide
                if (reculX >= 0 && reculX < DIMENSION_MAP && reculY >= 0 && reculY < DIMENSION_MAP &&
                    map[reculX][reculY].contenu == 0) {
                    // Déplacer le monstre vers la case de recul
                    map[reculX][reculY] = map[targetX][targetY];
                    map[targetX][targetY].contenu = 0; // Vider la case initiale
                }
            }
        }
    }
}

return EXIT_SUCCESS;

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
            if ( ( map[i][j].contenu == 2 ) && ! ( map[i][j].mstr.type == 4 ) ) {  // Si la case contient un monstre
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
                if (perso.inv[slot] == 0) {
                    strcpy(inventaireIcons[slot], "src/image/inventaireVide.bmp");
                }
                
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
            if (perso.inv[slot] == 0) {
                strcpy(inventaireIcons[slot], "src/image/inventaireVide.bmp");
            }
            break;

        case 4: // Clé à molette
        case 5: // Clés
        case 6: // Grande clé
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

void detruireObjet() {
    // Calculer la position de la case devant le joueur
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

    // Vérifier que les coordonnées sont valides
    if (x < 0 || x >= DIMENSION_MAP || y < 0 || y >= DIMENSION_MAP) {
        SDL_Log("Case invalide.");
        return;
    }

    // Vérifier si l'objet peut être détruit (coffre ou machine, mais pas grande machine)
    if ((map[x][y].contenu == 3 && 
        (map[x][y].spe.type == 1 || map[x][y].spe.type == 2 || map[x][y].spe.type == 6)) ||
        (map[x][y].contenu == 3 && map[x][y].spe.type == 5)) { // Coffre ouvert ou machine

        // Jouer le son de destruction
        Mix_Chunk *breakSound = Mix_LoadWAV("src/musique/break.mp3");
        if (breakSound) {
            Mix_PlayChannel(CHANNEL_BRAK, breakSound, 0);
        } else {
            SDL_Log("Erreur de chargement du son de destruction : %s", Mix_GetError());
        }

        // Détruire l'objet et remplacer par un sol
        map[x][y].contenu = 0; // Sol vide
        map[x][y].spe.type = 0;

    } else {
        SDL_Log("Aucun objet destructible devant le joueur.");
    }
}

void ouvrirCoffre(int x, int y) {
    if (perso.inv[5] > 0) { // Vérifie si le joueur a des clés
        perso.inv[5]--; // Consomme une clé
        if (perso.inv[5] == 0) {
            strcpy(inventaireIcons[5], "src/image/inventaireVide.bmp");
        }
        Mix_PlayChannel(CHANNEL_KEY, sonKey, 0);

        // Ajouter des récompenses
        int proba = aleatoire(1, 100);
        if (proba <= 40) {
            perso.inv[4]++; // Clé à molette
            strcpy(inventaireIcons[4], "src/image/inventaireWrench.bmp");
        } else if (proba <= 70) {
            perso.inv[2]++; // Potion de vie
            strcpy(inventaireIcons[2], "src/image/inventairePotion.bmp");
        } else {
            perso.inv[3]++; // Potion d'XP
            strcpy(inventaireIcons[3], "src/image/inventaireXp.bmp");
        }

        if (++perso.nbCoffres == quotaPistolet) {
            perso.inv[1]++; // Pistolet
            strcpy(inventaireIcons[1], "src/image/inventairePistolet.bmp");
        } else if (perso.nbCoffres == quotaBoss) {
            perso.inv[6]++; // Clé du boss
            strcpy(inventaireIcons[6], "src/image/inventaireBossKey.bmp");
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
        if (perso.inv[4] == 0) {
        strcpy(inventaireIcons[4], "src/image/inventaireVide.bmp");
        }
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
        if (perso.inv[4] == 0) {
        strcpy(inventaireIcons[4], "src/image/inventaireVide.bmp");
        }
        if (map[x][y].spe.type == 3) {
            map[x][y].spe.type = 4; // Grande machine en réparation
            Mix_PlayChannel(CHANNEL_WRENCH, sonWrench, 0);
        } else if (map[x][y].spe.type == 4) {
            map[x][y].spe.type = 7; // Grande machine réparée
            machineFuite++;

            // Effacer l'ancien affichage du texte des grandes machines réparées
            SDL_Rect eraseRect = {
                .x = WINDOW_WIDTH - 200,
                .y = WINDOW_HEIGHT - 50,
                .w = 190,
                .h = 30
            };
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Couleur noire
            SDL_RenderFillRect(renderer, &eraseRect);
            Mix_PlayChannel(CHANNEL_WRENCH, sonWrench, 0);
        }
    } else {
        SDL_Log("Vous n'avez pas de clé à molette !");
        Mix_PlayChannel(CHANNEL_FAIL, sonFail, 0); // Son si pas de clé à molette
    }
}

void bossInvoqueMonstres() {
    static unsigned int dernierInvoque = 0; // Temps de la dernière invocation
    unsigned int tempsActuel = SDL_GetTicks();

    // Calculer le ratio de vie du boss (entre 0.0 et 1.0)
    float ratioPv = (float)boss.hp / (float)boss.max_hp;

    // Calculer le délai basé sur des paliers
    unsigned int delaiInvocation = (unsigned int)(1000 + (500 * ceil(ratioPv * 10))); // 1s à 5s par palier de 10 %

    // Vérifier si le délai est écoulé
    if (tempsActuel - dernierInvoque >= delaiInvocation) {
        dernierInvoque = tempsActuel;

        // Définir les positions des monstres
        int positions[4][2] = {
            {boss.x + 3, boss.y + 3}, // Bas à droite
            {boss.x - 3, boss.y + 3}, // Bas à gauche
            {boss.x + 3, boss.y},     // Droite (ajouté après 50 %)
            {boss.x - 3, boss.y}      // Gauche (ajouté après 50 %)
        };

        // Déterminer combien de monstres invoquer (4 si <= 50 % HP, sinon 2)
        int nombreMonstres = (ratioPv <= 0.5) ? 4 : 2;

        // Générer un monstre pour chaque position spécifiée
        for (int i = 0; i < nombreMonstres; i++) {
            int x = positions[i][0];
            int y = positions[i][1];

            // Vérifier que la case est valide et vide
            if (x >= 0 && x < DIMENSION_MAP && y >= 0 && y < DIMENSION_MAP && map[x][y].contenu == 0) {
                // Générer un type de monstre aléatoire
                int typeMonstre = aleatoire(1, 3);

                // Placer un monstre sur la carte
                map[x][y].contenu = 2;
                map[x][y].mstr.type = typeMonstre;
                map[x][y].mstr.hp = (typeMonstre == 1) ? 80 : (typeMonstre == 2) ? 180 : 260;
                map[x][y].mstr.xp = (typeMonstre == 1) ? 10 : (typeMonstre == 2) ? 20 : 30;
            }
        }
    }
}

int mouvementHaut(void);

void spawnBoss(int argc, char **argv) {
    // Arrêter la musique actuelle
    Mix_HaltMusic();

    // Variable pour suivre le délai entre les actions
    unsigned int delay = 1000; // Augmenter le délai à 500 millisecondes
    unsigned int current_time = SDL_GetTicks();

    int murX = perso.posX;
    int murY = perso.posY - 1;
    if (murX >= 0 && murX < DIMENSION_MAP && murY >= 0 && murY < DIMENSION_MAP) {
        map[murX][murY].contenu = 0; // Placer un sol
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Impossible de placer un sol au dessus du joueur : coordonnées invalides.");
    }

    // Mouvement du joueur de deux cases vers le haut, une par une
    for (int i = 0; i < 2; i++) {
        while (SDL_GetTicks() - current_time < delay) {
            SDL_Delay(delay); // Attendre que le délai soit écoulé
        }
        current_time = SDL_GetTicks(); // Mettre à jour le temps actuel
        mouvementHaut(); // Appeler la fonction de mouvement haut
        //Xcamera = perso.posX - (WINDOW_WIDTH / caseSize) / 2;
        //Ycamera = perso.posY - (WINDOW_HEIGHT / caseSize) / 2;

        // Empêche la caméra de sortir des limites
        if (Xcamera < 0) Xcamera = 0;
        if (Ycamera < 0) Ycamera = 0;
        if (Xcamera > DIMENSION_MAP - (WINDOW_WIDTH / caseSize)) {
            Xcamera = DIMENSION_MAP - (WINDOW_WIDTH / caseSize);
        }
        if (Ycamera > DIMENSION_MAP - (WINDOW_HEIGHT / caseSize)) {
            Ycamera = DIMENSION_MAP - (WINDOW_HEIGHT / caseSize);
        }

        if (camera(argc, argv) != EXIT_SUCCESS) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Erreur lors de la mise à jour de la caméra pendant la cinématique.");
        }
    }

    // Placer un mur en dessous du joueur
    murX = perso.posX;
    murY = perso.posY + 1;
    if (murX >= 0 && murX < DIMENSION_MAP && murY >= 0 && murY < DIMENSION_MAP) {
        map[murX][murY].contenu = -2; // Placer un mur
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Impossible de placer un mur sous le joueur : coordonnées invalides.");
    }

    // Jouer le son de la grande clé
    if (Mix_PlayChannel(CHANNEL_BIG_KEY, sonBigKey, 0) == -1) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Erreur lors de la lecture du son de la grande clé : %s", Mix_GetError());
    }

    // Déplacer la caméra de deux cases vers le haut, une par une, pendant que le son joue
    for (int i = 0; i < 2; i++) {
        while (SDL_GetTicks() - current_time < delay) {
            SDL_Delay(delay); // Attendre que le délai soit écoulé
        }
        current_time = SDL_GetTicks(); // Mettre à jour le temps actuel
        if (Ycamera > 0) {
            Ycamera--;
        }
        if (camera(argc, argv) != EXIT_SUCCESS) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Erreur lors de la mise à jour de la caméra pendant la cinématique.");
        }
    }

    SDL_Delay(3000);

    // Activer la variable indiquant que le boss est présent
    bossPresent = SDL_TRUE;

    // Charger et jouer la musique du boss en boucle
    Mix_Music *bossMusic = Mix_LoadMUS("src/musique/boss.mp3");
    if (!bossMusic) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Erreur lors du chargement de la musique du boss : %s", Mix_GetError());
    } else {
        if (Mix_PlayMusic(bossMusic, -1) == -1) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Erreur lors de la lecture de la musique du boss : %s", Mix_GetError());
        }
    }
}

void interagirEnvironnement(int argc, char **argv) {
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
    } else if (map[x][y].contenu == 4 && perso.inv[6] > 0) {
        spawnBoss(argc, argv);
    } else {
        SDL_Log("Rien à interagir ici.");
    }
}

int testSol(int x, int y, int cote) {
    static int bossSalleGeneree = 0; // Variable pour vérifier si la salle du boss a déjà été générée

    switch (map[x][y].contenu) {
        case -2:  // Mur
        case 3:   // Objet spécial
        case 4:   // porte boss
            return 0;
            break;

        case -1:  // Porte
            num_salle++;  // Incrémenter le nombre de salles
            int largeur = aleatoire(5, 11);
            int longueur = aleatoire(5, 11);

            // Vérifier si c'est la 20e salle, si le joueur vient du bas, et si la salle du boss n'a pas encore été générée
            if (num_salle >= 30 && cote == 0 && bossSalleGeneree == 0) {
                if (genererSalleBoss(cote)) {
                    bossSalleGeneree = 1; // Marquer que la salle du boss a été générée
                }
                map[perso.posX][perso.posY - 1].contenu = 4; // Porte de boss
                return 0; // Salle du boss généré
            } else {
                // Générer une salle classique si la salle du boss ne peut pas être générée
                if (genererSalleValide(longueur, largeur, num_salle, cote) != EXIT_SUCCESS) {
                    int murX = perso.posX;
                    int murY = perso.posY;

                    switch (cote) {
                        case 0: murY = perso.posY - 1; break;
                        case 1: murX = perso.posX - 1; break;
                        case 2: murY = perso.posY + 1; break;
                        case 3: murX = perso.posX + 1; break;
                        default: break;
                    }

                    if (murX >= 0 && murX < DIMENSION_MAP && murY >= 0 && murY < DIMENSION_MAP) {
                        map[murX][murY].contenu = -2; // Placer un mur
                    }

                    return 0; // Aucun mouvement ne doit être effectué
                }

                // Déplacement du joueur si une salle classique a été générée avec succès
                switch (cote) {
                    case 0: perso.posY = y + 1; break;
                    case 1: perso.posX = x + 1; break;
                    case 2: perso.posY = y - 1; break;
                    case 3: perso.posX = x - 1; break;
                    default: break;
                }
                Mix_PlayChannel(CHANNEL_ROOM, sonRoom, 0);
                return 1;
                }

        case 2:  // Monstre
            if (map[x][y].mstr.type != 4) {
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
            }
            return 0;
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
        if ( ( Ycamera > 0 ) && ! bossPresent) {
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
        if ( ( Ycamera < DIMENSION_MAP - (WINDOW_HEIGHT / 100) ) && ! bossPresent ) {
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
        if ( ( Xcamera > 0 ) && ! bossPresent) {
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
        if ( ( Xcamera < DIMENSION_MAP - (WINDOW_WIDTH / 100) ) && ! bossPresent ) {
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
