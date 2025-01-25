#ifndef COSMICYONDER_H
#define COSMICYONDER_H

#define DIMENSION_MAP 300
#define WINDOW_WIDTH 1500
#define WINDOW_HEIGHT 900
#define FPS_LIMIT 16

#define BOSSSHP 5000

// Définition des canaux audio spécifiques
#define CHANNEL_MENU 0
#define CHANNEL_MOVE 1
#define CHANNEL_DAMAGE 2
#define CHANNEL_ROOM 3
#define CHANNEL_SWORD 4
#define CHANNEL_GUN 5
#define CHANNEL_MONSTER_HURT 6
#define CHANNEL_MONSTER_DEAD 7
#define CHANNEL_HOVER 8
#define CHANNEL_LEVEL_UP 9
#define CHANNEL_POTION_LIFE 10
#define CHANNEL_POTION_XP 11
#define CHANNEL_WRENCH 12
#define CHANNEL_KEY 13
#define CHANNEL_BIG_KEY 14
#define CHANNEL_FAIL 15
#define CHANNEL_SWORD_PICKUP 16
#define CHANNEL_ITEM 17
#define CHANNEL_BRAK 18
#define CHANNEL_BOSS_HURT 19

extern void SDL_ExitWithError(const char *message);
extern int texture( int argc, char **argv);
/*
int contenuCase = 0;
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Rect Case;
*/

typedef struct boss {
    int hp;                // Points de vie du boss
    int max_hp;            // Points de vie maximum
    int x;                 // Coordonnée X du morceau principal (milieu haut)
    int y;                 // Coordonnée Y du morceau principal (milieu haut)
    SDL_bool invulnerable; // Indique si le boss est invulnérable
    unsigned int invuln_timer; // Timer pour la période d'invulnérabilité
} Boss;

typedef struct sauvegarde {
    int posX, posY;
    int direction;
    int hp, hpMax;
    int xp, lvl;
    int inv[7];  // Inventaire
    int machineFuite;
    unsigned int timer;
    int map[DIMENSION_MAP][DIMENSION_MAP]; // État de la carte
    int cameraX;
    int cameraY; // Position de la caméra
    int graine; // Graine de la partie
    int nbCoffres; // Nombre de coffres ouverts
    int nbSalles; // Nombre de salles générées
} Sauvegarde;


typedef struct personnage {
    int direction; //haut = 1, gauche = 2, bas = 3, droite = 4
    int posX;
    int posY;
    int frameAnimation; // 4 frames , -5 = dégat, -10 = dcd
    int hp;
    int hpMax;
    int xp;
    int xpMax;
    int lvl;
    int sword_damage;
    int gun_damage;
    int nbCoffres;
    int * inv;
    SDL_bool invulnerable;            // Indique si le joueur est invulnérable
    unsigned int invulnerable_timer;  // Timer d'invulnérabilité
} personnage;

typedef struct monstre {
    int hp;
    int xp;
    int morceau;
    int type; // 1 = chuchu , 2 = creeper , 3 = sans, 4 = boss
} monstre;

typedef struct special {
    int type; // 1 = coffre; 2 = machine, 3 = grande machine, 4 = grande machine ( 1 clé a molette utilisée ), 5 = coffre ouvert, 6 = machine réparée, 7 = grande machine réparée, 8 = épée
    int inv; // la valeur de l'objet contenu dans le coffre ( 0 si machine )
} special;

typedef struct tile {
    int contenu; // -5 = vide, -2 = mur, -1 = porte, 0 = sol, 1 = personnage, 2 = monstre, 3 = coffre / machine, 4 = porte de boss
    monstre mstr;
    special spe; // .type signifie le cote si porte (0 = haut, 1 = gauche, 2 = bas, 3 = droite)
} tile;

typedef struct salle {
    int num;
    int largeur;
    int longueur;
    int posX;
    int posY;
    tile ** cases; 
} salle ;

extern int creeMap(void);
extern int jeu(int argc, char **argv);
extern tile contenuCase;
extern SDL_Window *window;
extern SDL_Renderer *renderer;
extern int Xcamera;
extern int Ycamera;
extern tile **map;

extern salle room;
void debug(const char *msg);


#endif