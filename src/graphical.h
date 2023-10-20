#ifndef GRAPHICALH
#define GRAPHICALH

#include <termios.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "common.h"
typedef unsigned int uint;

extern struct winsize termsize;

extern const char* CHARS_BLOCK_UP;
extern const char* CHARS_BLOCK_DOWN;
extern const char* CHARS_BLOCK_FULL;
extern const char* CHARS_BLOCK_LEFT;
extern const char* CHARS_BLOCK_RIGHT;
extern const char* CHARS_BLOCK_LIGHT;
extern const char* CHARS_BLOCK_MEDIUM;
extern const char* CHARS_BLOCK_DARK;
extern const char* CHARS_TRIANGLE_LEFT;
extern const char* CHARS_TRIANGLE_RIGHT;
extern const char* CHARS_TRIANGLE_DOWN;
extern const char* CHARS_TRIANGLE_UP;


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
void set_color(int color);
//set la couleur d'affichage background
void set_color_background(int color);
//affiche sur le terminal une image a une position donnée
void pict_display(picture_t pict, uint x, uint y);
#endif //def GRAPHICALH
