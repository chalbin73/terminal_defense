# TerminalDefense
## un Tower Defense en terminal

Un tower defense en terminal, codé en C, sans dépandences externes (a part quelques librairies standard).

## Build instruction
### Tous systèmes (incluant Windows/linux)
il est possbile de tester le jeu sans l'installer a l'aide de ssh (demande une connexion internet):
 - en commande `ssh -4Cp 389 insecure@mwaaa.fr`
    Il vous serra demandé, la première fois, d'accepter une clé ssh (répondez oui ou yes, selon la langue)
 - avec un client ssh, renseignez les informations suivantes
   - Adresse: mwaaa.fr
   - Nom d'utilisateur: insecure
   - Port: 389

NB: Il n'est pas possible de cette manière de modifier les raccourcis claviers \
NB: Cette solution est mise a disposition car le programme fonctionne très mal sur caséine (lag + timeout + mauvais rendu)

### Linux/unix/mac
 _(normalement, mac est supporté, mais non tésté)_
 - Installez `make` et un compilateur C (par ex `gcc`)
    Ceux ci sont généralement disponible dans le paquet `base-devel`
 - Téléchargez touts les fichier du projet
 - dézipez l'archive des sources (en terminal: `unzip src.zip`)
 - si vous le souhaitez, modifiez les raccourcis claviers dans le fichier `src/config.h`
 - Compilez en lancant `make` dans le dossier du projet, cela crée le fichier executable `terminal_defense` (lancable avec `./terminal_defense`)

### Windows
 Voici les différente possibilité, du plus facile au plus compliqué
 - utilisez la version ssh (pour tester la compilation, vous pouvez aller sur caséine, mais lancer le programme dessus est fortement déconséillé)
 - vous avez déja wsl/une vm linux: suivez les instruction de build linux en l'utilisant
 - installez make et gcc sur windows a l'aide de cygiwin ou autre équivalent, puis suivez les instruction de build linux (non tésté, peut ne pas fonctionner)
 /!\ Le fonctionnement n'a jamais été testé sur Windows, il n'est pas garanti

## Tester le programme

1) Le terminal dans lequel est lancé le jeu ne doit pas être changé de taille une fois le jeu lancé, mettez donc votre terminal
    a la taille souhaité pour jouer avant de lancer le jeu (pour une experience optimale, mettez vous en plein écran)
    (un fonctionement minimal requiert une taille de plus de 30*20 charactères, des bugs graphiques peuvent apparaitre en dessous)

2) Apprennez les raccourcis claviers
    - Les raccourcis claviers par défault (et de la version en ligne) sont détaillé ci-dessous (section Controles)
    - Vous pouvez les modifier dans config.h (il faut allors recompiler avec la commande make, et les raccourcis Maj+touche sont
    également modifié)
   
3) Lancez le jeu
 - si vous avez compilé, l'executable se nomme terminal_defense et peut se lancer avec la commande `./terminal_defense` dans le dossier du jeu
 - si vous utilisez la version ssh, relancer simplement la commande ssh ( ` ssh -4Cp 389 insecure@mwaaa.fr ` )

4) Survivez!
 - Les monstres (les flèches a gauche de l'écran) tentent d'atteindre et de détruire vôtre base (le drapeau a droite de l'écran)
 - Pour les en empécher, construisez des défenses (murs et tourelles)
 - le jeu se termine lorsque vous mourez (vie de la base a 0),
 que le jeu crash (ca n'arrive pas, normalement) ou que vous quittez le jeu avec le raccourcis clavier

# Controles
* zqsd : Deplacement du curseur
* Maj maintenu : Deplacement du curseur 5x plus rapide
* c : Ouverture du menu construction
  - séléction avec les touche de déplacement
  - annulation avec déplacement a gauche
  - validation avec déplacment a droite/touche de construction
* Maj aintenu: construit la dernière défence construite (pas de séléction)
* Ctrl+c: Quitter le programme

# Ce qui fonctionne/ne fonctionne pas
## Ce qui fonctionne
### Le jeu:
 - Les monstres se déplacent vers la base, détruisant ou contournant les obstacles sur leur passage
 - Les tourrelles tirent sur les monstres, permétant de les tuer
 - La mort des monstre permet de gagner des ressources de construction ainsi que du score
 - Les constructions illégales (sur une autre/manque de ressources) sont impossible
### Le menu:
 - On peut choisir sa difficulté
 - On peut relancer une partie lorsque l'on meurt

## Ce qui ne fonctionne pas
### Le build sur windows
 - Pas de diponibilité de make facilement
 - Code non tésté

### Le jeu sur caséine
Le code compile et se lance, cepandant
 - L'affichage est moche
  - Fines bandes noires entre chaque charactères
  - Décalage des lignes (police non monospace)
 - Timeout au bout de 240s (une partie complète durant généralement plus longtemps)


## Structure du projet

* `Readme.md` (Ce fichier) Documentation et présentation
* `Critères.md` Indique ou trouver les points de conformance aux critères d'évaluation, si vous vous perdez dans le code
* `Makefile` contient les commandes de compilation.
* `build` dossier créé à la compilation, contient les fichiers crée par le compilateur.
* `src` Contient le code source.

## Structure du code

* `terminaldefense.c` Contient le code principale du jeu en lui même
* `terminaldefense.h` Contient les definitions de structures et fonctions du jeu
* `graphical.h` `graphical.c` Contient le code relatif au rendu "graphique" dans le terminal
* `common.h` `common_and_ressources.c` Contient des utilitaires communs à tout le projet, plus les ressources du projet
* `config.h` Contient la configuration du jeu, ce fichier est fait pour être modifié par les utilisateurs
