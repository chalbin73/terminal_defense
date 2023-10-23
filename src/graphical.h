#ifndef GRAPHICALH
#define GRAPHICALH

#include <termios.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "config.h"
#include "common.h"
/************************
 *** GLOBALS VARIABLES ***
 *************************/

extern tab_size_t termsize;

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


/*********************************
 *** TYPEDEFS AND GENERAL STUFF ***
 **********************************/


//un pixel: une couleur,une couleur de fond, et 1 caractère UTF_8 (donc 4 char)
typedef struct {
	int color;
	int background_color;
	char c1;
	char c2;
	char c3;
	char c4;
} pixel_t;

// Les images sont des tableau de pixels
// (on met les lignes les une après les autres)
// stride renseigne de combien on doit se décaler pour accéder a la ligne suivante
typedef struct {
	tab_size_t size;
	pixel_t* data;
} picture_t;

//declaration de fonctions

//mets en place les graphismes
void init_graphical();
//appellé a la sortie, résponsable de clean les graphisme
void graphical_cleanup();


/*****************************************
 *** DIRECT PICTURES&TERMINAL UTILITIES ***
 ******************************************/

//@brief compare 2 pixels
bool pix_equal(pixel_t pix1, pixel_t pix2);

//séléctionne une sous image (renvoie une image "rognée" / "masqué"). La première image reste valide
//cette copie est fantôme et touts changement a l'image renvoyée se repercute sur l'image en paramettre
picture_t pict_crop_bound(picture_t pict, uint xmin, uint xmax, uint ymin, uint ymax);
picture_t pict_crop_size(picture_t pict, uint xmin, uint col, uint ymin, uint row);
//@brief mets le curseur a la position x,y sur le terminal
void go_to(int x,int y);
//@brief avance le curseur de nb char (vers la droite)
void advance_cursor(uint nb);
//set la couleur d'affichage
void set_color(int color);
//set la couleur d'affichage background
void set_color_background(int color);
//affiche sur le terminal une image a une position donnée
void pict_direct_display(picture_t pict, uint x, uint y);


/***************************
 *** COMPOSITOR UTILITIES ***
 ****************************/

// @brief Rangs compris par le compositeur
typedef enum COMPOSE_RANK {
	//premier plan
	COMPOSE_UI      = 0,
	//second plan
	COMPOSE_ARENA   = 1,
	//arière plan
	COMPOSE_BACK    = 2,
	//privé, composition des images
	COMPOSE_RESULT  = 3,
	//privé, changement depuis la dèrnière frames
	COMPOSE_CHANGES = 4,
} COMPOSE_RANK;

// @brief initialise le compositeur
void compose_init();
// @brief free les variables du compositeur
void compose_free();
/* @brief affiche une image plan demandé
 *
 * @param pict image a affciher
 * @param rank plan d'affichage
 * @param posx abscisse de la colonne gauche de l'image
 * @parma posy ordonée de la ligne supérieur de l'iamge
 */
void compose_disp_pict(picture_t pict,COMPOSE_RANK rank,uint posx,uint posy);
// @brief affiche un pixel au plan demandé
void compose_disp_pix(pixel_t pixel,COMPOSE_RANK rank,uint posx,uint posy);
// @brief calcule les changements a la position x,y
void compose_have_changed(int posx,int posy);
// @brief Affiche a l'écran les changements
void compose_refresh();
#endif //def GRAPHICALH
