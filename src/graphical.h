#ifndef GRAPHICALH
#define GRAPHICALH

#include <termios.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <locale.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "config.h"
#include "common.h"
typedef unsigned int uint;

enum CHAR {
	CHARS_BLOCK_UP       = L'▀',
	CHARS_BLOCK_DOWN     = L'▄',
	CHARS_BLOCK_FULL     = L'█',
	CHARS_BLOCK_LEFT     = L'▌',
	CHARS_BLOCK_RIGHT    = L'▐',
	CHARS_BLOCK_LIGHT    = L'░',
	CHARS_BLOCK_MEDIUM   = L'▒',
	CHARS_BLOCK_DARK     = L'▓',
	CHARS_TRIANGLE_LEFT  = L'◀',
	CHARS_TRIANGLE_RIGHT = L'▶',
	CHARS_TRIANGLE_DOWN  = L'▼',
	CHARS_TRIANGL_UP     = L'▼',
};

//Couleur pour les charactères d'échapement.
//Il faut ajouter 10 pour set le background.
typedef enum COLOR {
	COL_BLACK  =30,
	COL_RED    =31,
	COL_GREEN  =32,
	COL_YELLOW =33,
	COL_BLUE   =34,
	COL_MAGENTA=35,
	COL_CYAN   =36,
	COL_WHITE  =37,
	COL_DEFAULT=39,
} COLOR;

//un pixel: une couleur,une couleur de fond, et 1 caractère UTF_8 (donc 4 char)
typedef struct {
	COLOR color;
	COLOR background_color;
	char c1;
	char c2;
	char c3;
	char c4;
} pixel_t;

// Les images sont des tableau de pixels
// (on met les lignes les une après les autres)
// stride renseigne de combien on doit se décaler pour accéder a la ligne suivante
typedef struct {
	int col;
	int row;
	int stride;
	pixel_t* data;
} picture_t;

//var globale
char input_string[NB_INPUT_CHAR+1];
//declaration de fonctions

//mets en place les graphismes
void init_graphical();
//appellé a la sortie, résponsable de clean les graphisme
void graphical_cleanup();
//séléctionne une sous image (renvoie une image "rognée")
picture_t pict_crop_bound(picture_t pict, uint xmin, uint xmax, uint ymin, uint ymax);
picture_t pict_crop_size(picture_t pict, uint xmin, uint col, uint ymin, uint row);
//mets le curseur a la position x,y sur le terminal
void go_to(int x,int y);
//set la couleur d'affichage
void set_color(COLOR color);
//set la couleur d'affichage background
void set_color_background(COLOR color);
//affiche sur le terminal une image a une position donnée
void pict_display(picture_t pict, uint x, uint y);
//mets l'input clavier dans input_string
void get_input();
#endif //def GRAPHICALH
