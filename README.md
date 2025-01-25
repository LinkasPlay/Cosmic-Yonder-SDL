# Cosmic Yonder SDL
# README - Cosmic Yonder

## Description
**Cosmic Yonder** est un jeu d'exploration et de survie dans l'espace. Vous incarnez un pigeon qui doit explorer une station spatiale abandonnée, combattre des monstres, réparer des machines, et finalement affronter le boss final pour s'échapper. Chaque partie est unique grâce à une génération procédurale des niveaux.

---

## Objectif du jeu
L'objectif principal est de **vaincre le boss final** situé dans une salle spéciale verrouillée. Pour y parvenir, vous devez :

1. **Explorer les salles** : Trouvez des équipements, des objets et des coffres.
2. **Réparer des machines** : Cela augmente votre temps disponible pour atteindre le boss.
3. **Gagner de l'expérience** : Combattez des monstres pour monter de niveau et devenir plus puissant.
4. **Trouver la clé du boss** : Nécessaire pour accéder à la salle du boss.

---

## Instructions de compilation

### Windows
1. Assurez-vous d'avoir installé **MinGW** et les bibliothèques suivantes :
   - SDL2
   - SDL2_mixer
   - SDL2_ttf
2. Utilisez la commande suivante pour compiler le projet :

   gcc src/*.c -o bin/CosmicYonder.exe -I include -L lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_mixer -lSDL2_ttf -mwindows


### Linux
1. Installez les bibliothèques nécessaires :

   sudo apt-get install libsdl2-dev libsdl2-mixer-dev libsdl2-ttf-dev

2. Compilez avec la commande :

   gcc src/*.c $(sdl2-config --cflags --libs) -o CosmicYonder


---

## Contrôles

| **Touche**                         | **Action**                                    |
|------------------------------------|-----------------------------------------------|
| **Z / Flèche haut**                | Déplacement vers le haut                      |
| **Q / Flèche gauche**              | Déplacement vers la gauche                    |
| **S / Flèche bas**                 | Déplacement vers le bas                       |
| **D / Flèche droite**              | Déplacement vers la droite                    |
| **Espace / A / clic droit**        | Interagir avec l'environnement                |
| **E / Clic gauche**                | Utiliser l'objet sélectionné / attaque        |
| **R**                              | Détruire l'environement                       |
| **1 - 7**                          | Changer l'objet sélectionné dans l'inventaire |
| **Molette souris**                 | Changer d'objet dans l'inventaire             |
| **Échap**                          | Ouvrir le menu pause                          |

---

## Gameplay

### Inventaire
- Vous commencez avec un inventaire vide.
- Les objets suivants peuvent être collectés :
  1. **Épée/Tuyau** :
     - Permet de frapper autour de vous (5 cases).
     - Inflige 50 points de dégâts.
  2. **Pistolet** :
     - Tire en ligne droite.
     - Inflige 20 points de dégâts.
  3. **Potion de vie** :
     - Restaure 2 cœurs de santé.
  4. **Potion d'XP** :
     - Donne 25% de l'expérience nécessaire pour monter de niveau.
  5. **Clé à molette** :
     - Sert à réparer des machines et des grandes machines.
  6. **Clé** :
     - Ouvre des coffres.
  7. **Grande clé** :
     - Ouvre la salle du boss.

### Règles importantes
- Une **épée** est générée dans l'une des 5 premières salles.
- Un **pistolet** se trouve obligatoirement dans l'un des 10 premiers coffres ouverts.
- La **clé du boss** est située dans un coffre entre le 15ème et le 30ème coffre.
- Les coffres ont une probabilité de :
  - 40% : donner une clé à molette.
  - 30% : donner une potion de vie.
  - 30% : donner une potion d'XP.

### Machines
- **Machines normales** : Utilisez une clé à molette pour ajouter 3 minutes au chronomètre.
- **Grandes machines** : Nécessitent 2 clés à molette pour être réparées.
- Réparer **3 grandes machines** est requis pour une meilleure fin.

### Monstres
- Quatre types de monstres existent :
  1. **Niveau 1** : 80 PV, donne 10 XP.
  2. **Niveau 2** : 180 PV, donne 20 XP, chance de laisser une clé (20%).
  3. **Niveau 3** : 260 PV, donne 30 XP, chance de laisser :
     - Une clé (30%).
     - Une clé à molette (10%).
  4. **Boss** : 10 000 PV, pas d'XP. La victoire met fin au jeu.

---

## Informations supplémentaires

### Génération procédurale
- Le niveau est généré dynamiquement à partir d'une graine que vous pouvez entrer au début de la partie.
- La salle du boss est générée à partir de la 30ème salle si vous empruntez une porte au nord.

### Fin du jeu
- Vous devez trouver la salle du boss, vaincre le boss, et réparer les grandes machines pour obtenir la meilleure fin.
- Il existe plusieurs fins en fonction de vos performances.

---

## Remerciements
Projet d'informatique de CY Tech de 2024.
Fait par Lukas BORSARINI (Linkas)
Utilisation de open-ai pour l'aide à la programation, principalement le débogage, ainsi que pour certaines images (je les remplace petit à petit par mes magnifiques dessins).
Vous trouverez les informations des musiques dans la section soundtrack !

Merci d'avoir joué à **Cosmic Yonder** ! Bonne chance dans votre quête pour échapper à la station spatiale.

