#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>
#include <math.h>

#include "CosmicYonder.h"

void lancerBoucleDeJeu(int argc, char **argv);
int sauvegarderPartie(const char *nomFichier);

// Déclarations externes pour SDL et rendu
extern SDL_Window *window;
extern SDL_Renderer *renderer;

// Déclarations externes pour les sons
extern Mix_Chunk *sonMenu;
extern Mix_Chunk *sonMove;
extern Mix_Chunk *sonDamage;
extern Mix_Chunk *sonRoom;
extern Mix_Chunk *sonHover;
extern Mix_Chunk *swordSound;
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

extern Mix_Music *gameMusic;
extern Mix_Music *menuMusic;

extern int quotaPistolet;
extern int quotaBoss;
extern int Xcamera;
extern int Ycamera;
extern unsigned int start_time;
extern int num_salle;
extern int graine;

extern tile **map;                 // Structure pour représenter la carte
extern personnage perso;           // Structure pour représenter le joueur
extern int machineFuite;           // Nombre de grandes machines réparées

extern char inventaireIcons[7][64]; // Icônes des objets d'inventaire

extern int mouvementHaut(void);
extern int mouvementBas(void);
extern int mouvementGauche(void);
extern int mouvementDroite(void);
extern int camera(int argc, char **argv);

extern int creeMap(void);
extern int genererSalleValide(int longueurInitiale, int largeurInitiale, int num_salle, int cote);
extern int genererSalleBoss(int cote);
extern int actualiserMap(void);

extern int sauvegarderPartie(const char *nomFichier);
extern int chargerPartie(const char *nomFichier);

extern void afficherInterface(SDL_Renderer *renderer, TTF_Font *font, int selectedSlot);
extern void dessinerRectangleSelection(SDL_Renderer *renderer, int selectedSlot);

extern void interagirEnvironnement(int argc, char **argv);
extern void utiliserObjet(int slot);
extern void deplacerMonstresVersJoueur(void);
extern void detruireObjet(void);
extern void captureEntree(SDL_Renderer *renderer, TTF_Font *font, char *input, size_t tailleMax);

void bossInvoqueMonstres(void);

extern void SDL_LimitFPS(unsigned int limit);

extern unsigned int aleatoire(int min, int max);

extern unsigned int timer_start;
extern unsigned int start_time;

extern int pause(void);

extern int caseSize;
extern int startX;
extern SDL_bool bossPresent;

void intro();

int graine = -1;  // Graine non initialisée

int jeu(int argc, char **argv) {
    extern SDL_Window *window;
    extern SDL_Renderer *renderer;

    // Appeler l'introduction
    intro();

    // Charger et jouer la musique du jeu
    gameMusic = Mix_LoadMUS("./src/musique/vaisseau.mp3");
    if (!gameMusic) {
        SDL_Log("Erreur lors du chargement de la musique du jeu : %s\n", Mix_GetError());
    }
    Mix_PlayMusic(gameMusic, -1);

    quotaPistolet = aleatoire(3,10);
    quotaBoss = aleatoire(15,30);

    // Initialiser SDL_ttf
    if (TTF_Init() == -1) {
        SDL_Log("Erreur lors de l'initialisation de SDL_ttf : %s\n", TTF_GetError());
        return EXIT_FAILURE;
    }

    // Charger une police avec une taille adaptée à la fenêtre 1500x900
    TTF_Font *font = TTF_OpenFont("./src/fonts/texte.ttf", 48);  // Augmentation de la taille de la police
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
    SDL_Surface *validationSurface = TTF_RenderUTF8_Solid(font, "Graine validée, génération de la carte...", textColor);
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

    //SAuvegarde automatique
    char nomFichier[256];
    sprintf(nomFichier, "saves/save_%d.sav", graine);
    if (sauvegarderPartie(nomFichier) != EXIT_SUCCESS) {
        SDL_Log("Erreur lors de la sauvegarde.");
    }

    // Après la génération de la map, lancer la boucle de jeu
    lancerBoucleDeJeu(argc,argv);  // Assurez-vous que cette fonction est bien définie

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

void credits() {
    extern SDL_Renderer *renderer;

    // Afficher l'écran noir
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Fond noir
    SDL_RenderClear(renderer);

    SDL_RenderPresent(renderer);

    // Charger la musique des crédits
    Mix_Music *creditMusic = Mix_LoadMUS("src/musique/credits.mp3");
    if (!creditMusic) {
        SDL_Log("Erreur de chargement de la musique des crédits : %s", Mix_GetError());
        return;
    }
    Mix_PlayMusic(creditMusic, -1);

    // Variables pour déterminer la fin obtenue
    int etoiles = 0;
    const char *texteFin = NULL;
    const char *imageFin = NULL;
    if (timer_start > 300000 && machineFuite >= 3) { // Très bonne fin
        etoiles = 3;
        texteFin = "Les voyants verts s’alignent sur le tableau de bord du vaisseau alors que tu te tiens fièrement au sommet de la console. Les grandes machines ronronnent, le chef alien n’est plus qu’un tas d’os, et tu as assez d’oxygène pour voir ton plan jusqu’au bout. Le vaisseau décolle en trombe, percant le silence de l’espace, et en quelques heures, la Terre se dessine devant toi.\n À l’atterrissage, la panique gagne les rues. Les humains accourent pour voir ce qui s’est écrasé. Les regards s’élèvent… et là, tu le vois. Une cible parfaite, un symbole de leur arrogance : Donald Trump, dans un de ses éternels discours en extérieur. Avec une précision divine, tu descends en piqué, laissant ta marque indélébile sur sa chevelure dorée. Les caméras tournent, les humains s’indignent, mais toi, tu es enfin chez toi. Et cette fois, rien ni personne ne t’enverra ailleurs.\n Le pigeon est de retour. Longue vie au chaos !";
        imageFin = "src/image/fin_tres_bonne.bmp";
    } else if (timer_start <= 300000 && machineFuite >= 3) { // Bonne fin
        etoiles = 2;
        texteFin = "Avec un dernier coup de clé à molette, la troisième machine rugit à la vie, éclairant la station d’une lueur salvatrice. Le chef alien n’est plus qu’un souvenir, et tu actives les systèmes pour réactiver le vaisseau. Les alarmes d’oxygène te rappellent que le temps est compté, mais tu refuses de laisser l’espace être ta prison.\n Le vaisseau décolle dans un grondement assourdissant, et tu te diriges vers la Terre. Cependant, l’air se fait de plus en plus rare, et alors que l’atmosphère terrestre apparaît à l’horizon, tu fermes les yeux une dernière fois. Tu n’auras peut-être pas survécu au voyage, mais ton retour sur Terre restera gravé dans l’histoire. Un pigeon qui a défié l’univers.";
        imageFin = "src/image/fin_bonne.bmp";
    } else if (timer_start > 300000 && machineFuite < 3) { // Mauvaise fin
        etoiles = 1;
        texteFin = "Les cris du chef alien s’éteignent alors que tu te tiens, victorieux, sur une pile de débris extraterrestres. Mais en explorant la station, tu réalises que tu as laissé trop de choses inachevées. Les grandes machines restent hors d’usage, et bien que tu trouves une capsule d’évacuation, elle n’est pas assez puissante pour te ramener sur Terre.\n Tu dérives dans l’espace, condamné à une vie solitaire parmi les étoiles. Peut-être un jour croiseras-tu une autre espèce capable de comprendre ton histoire héroïque… ou du moins de t’offrir des miettes. Mais pour l’instant, tu n’es qu’un pigeon perdu dans l’infini.";
        imageFin = "src/image/fin_mauvaise.bmp";
    } else { // Très mauvaise fin
        etoiles = 0;
        texteFin = "Le chef alien gît à tes pattes emplumées. Tu as triomphé… mais à quel prix ? Alors que tu cherches désespérément une sortie, le sifflement de ton réservoir d’oxygène vide résonne dans le silence oppressant de la station. Les voyants rouges clignotent, et l’air devient rare. Ton plumage s’alourdit, ta vision se brouille. Malgré tous tes efforts, tu n’étais pas assez rapide, pas assez préparé.\n L’humanité a gagné. Le pigeon qui défiait les étoiles disparaît dans l’immensité du cosmos. Mais quelque part, un humain, la tête levée vers le ciel, sent comme un poids disparaître de ses épaules…";
        imageFin = "src/image/fin_tres_mauvaise.bmp";
    }

    // Sauvegarder les étoiles obtenues
    FILE *highScoreFile = fopen("highScore.txt", "a+");
    if (highScoreFile) {
        int totalEtoiles = 0;
        int maxEtoiles = 0;

        // Lire les scores précédents
        fscanf(highScoreFile, "Total etoiles: %d\n", &totalEtoiles);
        fscanf(highScoreFile, "Derniere graine: %d\n", &graine);
        fscanf(highScoreFile, "Etoiles max: %d\n", &maxEtoiles);

        // Ajouter les nouvelles étoiles
        totalEtoiles += etoiles;
        if (etoiles > maxEtoiles) {
            maxEtoiles = etoiles;
        }

        // Écrire les nouvelles valeurs
        freopen("highScore.txt", "w", highScoreFile);
        fprintf(highScoreFile, "Total etoiles: %d\n", totalEtoiles);
        fprintf(highScoreFile, "Derniere graine: %d\n", graine);
        fprintf(highScoreFile, "Etoiles max: %d\n", maxEtoiles);

        fclose(highScoreFile);
    } else {
        SDL_Log("Erreur : impossible d'ouvrir le fichier highScore.txt.");
    }

    // Afficher les étoiles
    SDL_Surface *etoileSurface = SDL_LoadBMP("src/image/etoile.bmp");
    if (etoileSurface) {
        SDL_Texture *etoileTexture = SDL_CreateTextureFromSurface(renderer, etoileSurface);
        SDL_FreeSurface(etoileSurface);

        for (int i = 0; i < etoiles; i++) {
            SDL_Rect etoileRect = {50 + i * 64, 50, 64, 64}; // Position des étoiles
            SDL_RenderCopy(renderer, etoileTexture, NULL, &etoileRect);
        }
        SDL_DestroyTexture(etoileTexture);
    }

    // Ajouter une pause avant de commencer les crédits
    SDL_Delay(2000);

    // Afficher l'image de la fin
    SDL_Surface *imageSurface = SDL_LoadBMP(imageFin);
    if (imageSurface) {
        SDL_Texture *imageTexture = SDL_CreateTextureFromSurface(renderer, imageSurface);
        SDL_FreeSurface(imageSurface);

        SDL_Rect imageRect = {WINDOW_WIDTH / 4, 50, (WINDOW_WIDTH) / 2, (WINDOW_HEIGHT) / 2};
        SDL_RenderCopy(renderer, imageTexture, NULL, &imageRect);
        SDL_DestroyTexture(imageTexture);
    }

    SDL_RenderPresent(renderer);

    // Ajouter une pause avant d'afficher le texte
    SDL_Delay(2000);

    // Afficher le texte de la fin avec un fondu
    SDL_Color textColor = {255, 255, 255, 255};
    TTF_Font *font = TTF_OpenFont("./src/fonts/texte.ttf", 48);
    if (font) {
        SDL_Surface *textSurface = TTF_RenderUTF8_Blended_Wrapped(font, texteFin, textColor, WINDOW_WIDTH - 100);
        if (textSurface) {
            SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            SDL_FreeSurface(textSurface);

            SDL_Rect textRect = {50, WINDOW_HEIGHT - 400, WINDOW_WIDTH - 100, 400};
            SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
            SDL_DestroyTexture(textTexture);
        }
        TTF_CloseFont(font);
    }

    SDL_RenderPresent(renderer);

    SDL_bool continuer = SDL_TRUE;
    while (continuer) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    SDL_Quit();
                    exit(EXIT_SUCCESS);
                    break;

                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_SPACE) {
                        continuer = SDL_FALSE;
                    }
                    break;

                default:
                    break;
            }
        }
    }

    SDL_Quit();
    exit(EXIT_SUCCESS);
}

void intro() {
    extern SDL_Renderer *renderer;

    // Charger l'image d'introduction
    SDL_Surface *imageIntroSurface = SDL_LoadBMP("src/image/intro.bmp");
    if (!imageIntroSurface) {
        SDL_Log("Erreur lors du chargement de l'image d'introduction : %s", SDL_GetError());
        return;
    }
    SDL_Texture *imageIntroTexture = SDL_CreateTextureFromSurface(renderer, imageIntroSurface);
    SDL_FreeSurface(imageIntroSurface);

    if (!imageIntroTexture) {
        SDL_Log("Erreur lors de la création de la texture d'introduction : %s", SDL_GetError());
        return;
    }

    // Charger la police pour le texte
    TTF_Font *font = TTF_OpenFont("./src/fonts/texte.ttf", 32);
    if (!font) {
        SDL_Log("Erreur lors du chargement de la police : %s", TTF_GetError());
        return;
    }

    SDL_Color textColor = {255, 255, 255, 255}; // Blanc

    const char *texteIntro =
        "L’humanité. Une espèce orgueilleuse, incapable de tolérer ce qui sort un peu… du cadre. "
        "Des siècles de conflits, d’innovations technologiques, et pourtant, un seul ennemi les a unis : les pigeons.\n\n"
        "Et toi, pauvre oiseau, tu n’étais qu’un parmi tant d’autres, appréciant les plaisirs simples de la vie : voler, picorer des miettes, et, "
        "bien sûr, marquer ton territoire… sur la tête des passants.\n\n"
        "Mais cette fois, ils en ont eu assez. Les humains ont construit une fusée pour te bannir, toi, “Pigeon 001”, dans les confins de l’espace. "
        "Loin de leurs statues, de leurs voitures, et de leurs coiffures impeccables.\n\n"
        "Ton exil aurait pu être la fin de l’histoire, mais non…\n\n"
        "Te voilà échoué sur une station spatiale abandonnée. Enfin… pas si abandonnée que ça. "
        "Des créatures extraterrestres grouillent partout, menaçant de te réduire en poussière. L’oxygène diminue, et le temps presse. "
        "Mais toi, courageux volatile, tu as un objectif clair : survivre, réparer ce vaisseau, et surtout… rentrer sur Terre "
        "pour leur montrer qu’on ne bannit pas un pigeon impunément !";

    // Créer une surface pour le texte
    SDL_Surface *textSurface = TTF_RenderUTF8_Blended_Wrapped(font, texteIntro, textColor, WINDOW_WIDTH - 100);
    if (!textSurface) {
        SDL_Log("Erreur lors de la création de la surface pour le texte d'introduction : %s", TTF_GetError());
        return;
    }
    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_FreeSurface(textSurface);

    if (!textTexture) {
        SDL_Log("Erreur lors de la création de la texture pour le texte d'introduction : %s", SDL_GetError());
        return;
    }

    SDL_Rect imageRect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    SDL_Rect textRect = {50, 50, WINDOW_WIDTH - 100, WINDOW_HEIGHT - 100};

    SDL_bool continuer = SDL_TRUE;

    while (continuer) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    SDL_Quit();
                    exit(EXIT_SUCCESS);
                    break;

                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_SPACE) {
                        // Passer à l'écran de la graine
                        continuer = SDL_FALSE;
                    }
                    break;

                default:
                    break;
            }
        }

        // Afficher l'image
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, imageIntroTexture, NULL, &imageRect);

        // Afficher le texte
        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
        SDL_RenderPresent(renderer);

        SDL_Delay(100); // Limiter la fréquence de rafraîchissement
    }

    // Nettoyage des ressources
    Mix_HaltMusic();
    Mix_FreeMusic(menuMusic);
    SDL_DestroyTexture(imageIntroTexture);
    SDL_DestroyTexture(textTexture);
    TTF_CloseFont(font);
}

int sauvegarderPartie(const char *nomFichier) {
    Sauvegarde sauvegarde;

    // Copier les données du joueur
    sauvegarde.posX = perso.posX;
    sauvegarde.posY = perso.posY;
    sauvegarde.direction = perso.direction;
    sauvegarde.hp = perso.hp;
    sauvegarde.hpMax = perso.hpMax;
    sauvegarde.xp = perso.xp;
    sauvegarde.lvl = perso.lvl;
    memcpy(sauvegarde.inv, perso.inv, sizeof(perso.inv));
    sauvegarde.machineFuite = machineFuite;
    sauvegarde.timer = SDL_GetTicks() - start_time;

    // Copier les informations supplémentaires
    sauvegarde.cameraX = Xcamera;
    sauvegarde.cameraY = Ycamera;
    sauvegarde.graine = graine;
    sauvegarde.nbCoffres = perso.nbCoffres;
    sauvegarde.nbSalles = num_salle;

    // Copier l'état de la carte
    for (int i = 0; i < DIMENSION_MAP; i++) {
        for (int j = 0; j < DIMENSION_MAP; j++) {
            sauvegarde.map[i][j] = map[i][j].contenu;
        }
    }

    // Vérification des données avant la sauvegarde
    if (sauvegarde.posX < 0 || sauvegarde.posX >= DIMENSION_MAP ||
        sauvegarde.posY < 0 || sauvegarde.posY >= DIMENSION_MAP) {
        SDL_Log("Erreur : Coordonnées du joueur invalides (%d, %d).", sauvegarde.posX, sauvegarde.posY);
        return EXIT_FAILURE;
    }

    if (sauvegarde.hp <= 0 || sauvegarde.hp > sauvegarde.hpMax) {
        SDL_Log("Erreur : Points de vie invalides (HP: %d, Max HP: %d).", sauvegarde.hp, sauvegarde.hpMax);
        return EXIT_FAILURE;
    }

    // Ouvrir le fichier en écriture binaire
    FILE *fichier = fopen(nomFichier, "wb");
    if (!fichier) {
        SDL_Log("Erreur : Impossible d'ouvrir le fichier %s pour la sauvegarde.", nomFichier);
        return EXIT_FAILURE;
    }

    // Écrire la sauvegarde
    size_t result = fwrite(&sauvegarde, sizeof(Sauvegarde), 1, fichier);
    fclose(fichier);

    if (result != 1) {
        SDL_Log("Erreur : Échec de l'écriture des données dans le fichier %s.", nomFichier);
        return EXIT_FAILURE;
    }

    SDL_Log("Partie sauvegardée avec succès dans le fichier %s.", nomFichier);
    return EXIT_SUCCESS;
}

int chargerPartie(const char *nomFichier) {
    Sauvegarde sauvegarde;

    // Ouvrir le fichier en lecture binaire
    FILE *fichier = fopen(nomFichier, "rb");
    if (!fichier) {
        Mix_PlayChannel(CHANNEL_FAIL, sonFail, 0); // Jouer un son d'échec si le fichier n'existe pas
        SDL_Log("Erreur lors de l'ouverture du fichier de chargement.");
        return EXIT_FAILURE;
    }

    // Lire la sauvegarde
    fread(&sauvegarde, sizeof(Sauvegarde), 1, fichier);
    fclose(fichier);

    // Restaurer les données du joueur
    perso.posX = sauvegarde.posX;
    perso.posY = sauvegarde.posY;
    perso.direction = sauvegarde.direction;
    perso.hp = sauvegarde.hp;
    perso.hpMax = sauvegarde.hpMax;
    perso.xp = sauvegarde.xp;
    perso.lvl = sauvegarde.lvl;
    memcpy(perso.inv, sauvegarde.inv, sizeof(perso.inv));
    machineFuite = sauvegarde.machineFuite;
    start_time = SDL_GetTicks() - sauvegarde.timer;

    // Restaurer les informations supplémentaires
    Xcamera = sauvegarde.cameraX;
    Ycamera = sauvegarde.cameraY;
    graine = sauvegarde.graine;
    perso.nbCoffres = sauvegarde.nbCoffres;
    num_salle = sauvegarde.nbSalles;

    // Restaurer l'état de la carte
    for (int i = 0; i < DIMENSION_MAP; i++) {
        for (int j = 0; j < DIMENSION_MAP; j++) {
            map[i][j].contenu = sauvegarde.map[i][j];
        }
    }

    // Actualiser la carte et d'autres éléments si nécessaire
    actualiserMap();
    SDL_Log("Partie chargée avec succès !");
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
    SDL_Rect button_load = {(WINDOW_WIDTH - button_width) / 2, button_save.y + button_height + spacing, button_width, button_height};
    SDL_Rect button_quit = {(WINDOW_WIDTH - button_width) / 2, button_load.y + button_height + spacing, button_width, button_height};

    // Créer les surfaces et textures pour le texte des boutons
    SDL_Surface *textPauseSurface = TTF_RenderText_Solid(font, "Menu Pause", textColor);
    SDL_Surface *textResumeSurface = TTF_RenderText_Solid(font, "Reprendre", textColor);
    SDL_Surface *textSaveSurface = TTF_RenderText_Solid(font, "Sauvegarder", textColor);
    SDL_Surface *textLoadSurface = TTF_RenderText_Solid(font, "Charger", textColor);
    SDL_Surface *textQuitSurface = TTF_RenderText_Solid(font, "Quitter", textColor);

    SDL_Texture *textPauseTexture = SDL_CreateTextureFromSurface(renderer, textPauseSurface);
    SDL_Texture *textResumeTexture = SDL_CreateTextureFromSurface(renderer, textResumeSurface);
    SDL_Texture *textSaveTexture = SDL_CreateTextureFromSurface(renderer, textSaveSurface);
    SDL_Texture *textLoadTexture = SDL_CreateTextureFromSurface(renderer, textLoadSurface);
    SDL_Texture *textQuitTexture = SDL_CreateTextureFromSurface(renderer, textQuitSurface);

    SDL_FreeSurface(textPauseSurface);
    SDL_FreeSurface(textResumeSurface);
    SDL_FreeSurface(textSaveSurface);
    SDL_FreeSurface(textLoadSurface);
    SDL_FreeSurface(textQuitSurface);

    SDL_Rect textPauseRect = {(WINDOW_WIDTH - 200) / 2, (WINDOW_HEIGHT - menu_height) / 2, 200, 50};

    // Variables pour savoir si un bouton est survolé
    SDL_bool hover_resume = SDL_FALSE;
    SDL_bool hover_save = SDL_FALSE;
    SDL_bool hover_load = SDL_FALSE;
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

                    if (event.motion.x >= button_load.x && event.motion.x <= button_load.x + button_width &&
                        event.motion.y >= button_load.y && event.motion.y <= button_load.y + button_height) {
                        hover_load = SDL_TRUE;
                    } else {
                        hover_load = SDL_FALSE;
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
                            char nomFichier[256];
                            sprintf(nomFichier, "saves/save_%d.sav", graine);
                            if (sauvegarderPartie(nomFichier) != EXIT_SUCCESS) {
                                SDL_Log("Erreur lors de la sauvegarde.");
                            }
                        } else if (hover_load) {
                            Mix_PlayChannel(CHANNEL_HOVER, sonHover, 0);
                            char input[100] = {0};

                            // Capture l'entrée utilisateur via SDL
                            captureEntree(renderer, font, input, sizeof(input));
                            
                            char nomFichier[256];

                            FILE *file = fopen(nomFichier, "r");
                            if (file) {
                                fclose(file);
                                SDL_Log("Le fichier existe.");
                            } else {
                                SDL_Log("Le fichier n'existe pas.");
                            }
                            sprintf(nomFichier, "saves/save_%s.sav", input);
                            if (chargerPartie(nomFichier) == EXIT_FAILURE) {
                                SDL_Log("Échec du chargement de la partie.");
                            }
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
                        case SDLK_DELETE: // Supr (cheat)
                            perso.inv[6]++; // Clé du boss
                            strcpy(inventaireIcons[6], "src/image/monstreType3.bmp");
                            num_salle = 50;
                            perso.inv[5] = 99; // Clé
                            strcpy(inventaireIcons[5], "src/image/inventaireKey.bmp");
                            perso.inv[4] = 99; // Clé à molette
                            strcpy(inventaireIcons[4], "src/image/inventaireWrench.bmp");
                            perso.inv[2] = 99; // Potion de vie
                            strcpy(inventaireIcons[2], "src/image/inventairePotion.bmp");
                            perso.inv[3] = 99 ; // Potion d'XP
                            strcpy(inventaireIcons[3], "src/image/inventaireXp.bmp");
                            break;
                        case SDLK_r: // Touche "R" detruire obnjet
                            detruireObjet();
                            break;
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
                            interagirEnvironnement(argc, argv);
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
                        interagirEnvironnement(argc, argv); // Interaction avec clic droit
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

        if (bossPresent) {
            bossInvoqueMonstres();
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
