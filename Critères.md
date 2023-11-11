# Critères d'évaluation
Ce fichier liste ou sont remplis les critères d'évaluation
si vous vous perdez dans le code

1) Les fichiers sources sont dans le dossier src
   La documentation dans le README.md

2) Lisez le README.md, partie Build instruction

3) Lisez le README.md, partie Tester le programme (un rappel des
   raccourcis claviers est présent en jeu)

4) Du moment que le terminal est suffisamment grand (voir README.md), tout est censé fonctionner comme prevu

5) Je vous laisse tester

6) Les spécifications de fonctions sont disponibles dans les .h (commentaire au format doxygen + déclaration)

7) La fonction treat_input (fichier terminaldefense.c) permet d'obtenir et de traiter les entrées clavier

8) Toutes les `picture_t.data` sont des tableaux 2D, ainsi que `monster_position`, `defense_array`, `pathfinder_array` ...
    `compositor_pixels` est un tableau 3D
    Tous les tableaux sont gérés avec un pointeur simple, et on se décale de `stride` (le nombre de colonne) pour changer de ligne

9) Tout l'affichage et géré dans graphical.h, la fonction finalisant les update de l'écran et l'appel
  a `pict_direct_display`, qui utilise bien printf pour afficher a l'écran (sur le terminal).

10) Tous les appels a malloc sont effectués à l'aide de la fonction `safe_malloc`, dans le fichier `common_and_ressources.c`, où
     la valeur de retour et bien vérifié.
