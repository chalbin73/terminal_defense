# TLDR;
## Tower L** Defense R****;

Un tower defense en terminal, codé en C, sans dépandences externes.

## Build instruction
### Linux/unix
 _(normalement, mac est supporté, mais non tésté)_
 - Installez `make` et un compilateur C (par ex `gcc`)
    Ceux ci sont généralement disponible dans le paquet `base-devel`
 - Téléchargez touts les fichier du projet
 - si vous le souhaitez, modifiez les raccourcis claviers dans le fichier `src/config.h`
 - Compilez en lancant `make` dans le dossier du projet, cela crée le fichier executable `tldr`
### Windows
 Voici les différente possibilité, du plus facile au plus compliqué
 - vous avez déja wsl/une vm linux: suivez les instruction de build linux en l'utilisant
 - testez le projet sur caséine
 - installez make et gcc sur windows a l'aide de cygiwin ou autre équivalent, puis suivez les instruction de build linux (peut ne pas fonctionner)

## Tester le programme

1) Le terminal dans lequel est lancé le jeu ne doit pas être resize une fois le jeu lancé, mettez donc votre terminal
    a la taille souhaité pour jouer avant de lancer le jeu (pour une experience optimale, mettez vous en plein écran)
2) Apprennez vos raccourcis claviers:
    - Les touches de déplacements (par défault `z q s d`), Maj + déplacement pour 5 cases
    - le menu construction (par défault `c`)
      - naviguez ensuite avec les touches de déplacement pour séléctioner la défense a construire avec haut/bas
      - touche droite/menu construction pour valider
      - touche gauche pour annuler
    - Maj + construction pour une construction rapide (dernière défense posée)
    - la touche pause (par défault p)
    - quiter le jeu (par défault `Ctrl+c`)
