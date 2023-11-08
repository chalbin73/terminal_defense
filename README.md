# TLDR;
## Tower L** Defense R****;

Un tower defense en terminal, codé en C, sans dépandences externes.

## Build instruction
### Tous systèmes (incluant Windows/linux)
il est possbile de tester le jeu sans l'installer a l'aide de ssh (demande une connexion internet):
 - en commande `ssh -Cp 389 insecure@mwaaa.fr`
 - avec un client ssh, renseignez les informations suivantes
   - Adresse: mwaaa.fr
   - Nom d'utilisateur: insecure
   - Port: 389
sh va vous demander d'accépter une clé publique, répondez oui (ou yes)
NB: Il n'est pas possible de cette manière de modifier les raccourcis claviers
NB: Cette solution est mise a disposition car le programme fonctionne très mal sur caséine (lag + timeout + mauvais rendu)
### Linux/unix
 _(normalement, mac est supporté, mais non tésté)_
 - Installez `make` et un compilateur C (par ex `gcc`)
    Ceux ci sont généralement disponible dans le paquet `base-devel`
 - Téléchargez touts les fichier du projet
 - si vous le souhaitez, modifiez les raccourcis claviers dans le fichier `src/config.h`
 - Compilez en lancant `make` dans le dossier du projet, cela crée le fichier executable `tldr`
### Windows
 Voici les différente possibilité, du plus facile au plus compliqué
 - utilisez la version ssh
 - vous avez déja wsl/une vm linux: suivez les instruction de build linux en l'utilisant
 - installez make et gcc sur windows a l'aide de cygiwin ou autre équivalent, puis suivez les instruction de build linux (peut ne pas fonctionner)

## Tester le programme

1) Le terminal dans lequel est lancé le jeu ne doit pas être changé de taille une fois le jeu lancé, mettez donc votre terminal
    a la taille souhaité pour jouer avant de lancer le jeu (pour une experience optimale, mettez vous en plein écran)
2) Apprennez les raccourcis claviers (pour les modifier, changez `src/config.h` et recompilez. c'est impossible sur la version internet):
    - Les touches de déplacements (par défault `z q s d`), Maj + déplacement pour 5 cases
    - le menu construction (par défault `c`)
      - naviguez ensuite avec les touches de déplacement pour séléctioner la défense a construire avec haut/bas
      - touche droite/menu construction pour valider
      - touche gauche pour annuler
    - Maj + construction pour une construction rapide (dernière défense posée)
    - la touche pause (par défault p)
    - quiter le jeu (par défault `Ctrl+c`)
3) Lancez le jeu
 - si vous avez compilé, l'executable se nomme tldr et peut se lancer avec la commande `./tldr` dans le dossier du jeu
 - si vous utilisez la version ssh, relancer simplement la commande ssh ( ` ssh -4Cp 389 insecure@mwaaa.fr ` )
4) Survivez!
 - Les monstres (les flèches a gauche de l'écran) tentent d'atteindre et de détruire vôtre base (a droite de l'écran)
 - Pour les en empécher, construisez des défenses (mur/tourelle)
 - le jeu se termine lorsque vous mourez (vie de la base a 0),
 que le jeu crash (ca n'arrive pas, normalement) ou que vous quitez le jeu avec le raccourcis clavier
