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

//taille du terminal
extern tab_size_t termsize;

/*********************************
 *** TYPEDEFS AND GENERAL STUFF ***
 **********************************/


//un pixel: une couleur,une couleur de fond, et 1 caractère UTF_8 (donc max 4 char)
typedef struct
{
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
typedef struct
{
	tab_size_t size;
	pixel_t      *data;
} picture_t;

//declaration de fonctions

//mets en place les graphismes
void         init_graphical(void);
//appellé a la sortie, résponsable de clean les graphisme et de free les variables
void         graphical_cleanup(void);


/*****************************************
 *** DIRECT PICTURES&TERMINAL UTILITIES ***
 ******************************************/

//@brief compare 2 pixels
//@return Renvoie true si les 2 pixels sont égaux
bool         pix_equal(pixel_t pix1, pixel_t pix2);

//séléctionne une sous image (renvoie une image "rognée" / "masqué"). La première image reste valide
//cette copie est fantôme et touts changement a l'image renvoyée se repercute sur l'image en paramettre
//version par min max et version par taille
picture_t    pict_crop_bound(picture_t pict, coordonee_t min, coordonee_t max);
picture_t    pict_crop_size(picture_t pict, coordonee_t min, coordonee_t size);

//@brief mets le curseur a la position x,y sur le terminal
void         go_to(coordonee_t pos);
//@brief avance le curseur de nb char (vers la droite, sur la même ligne)
void         advance_cursor(uint nb);
//set la couleur d'affichage
void         set_color(int color);
//set la couleur d'affichage background
void         set_color_background(int color);
//affiche sur le terminal une image a une position donnée
void         pict_direct_display(picture_t pict, coordonee_t pos);
//mets un string (ansi uniquement) dans une image
void         txt_to_img(picture_t result, const char *text_to_display, COLOR text_color, COLOR background_color);

/***************************
 *** COMPOSITOR UTILITIES ***
 ****************************/

// @brief Rangs compris par le compositeur
typedef enum COMPOSE_RANK
{
	//premier plan
	COMPOSE_UI = 0,
	//second plan
	COMPOSE_ARENA = 1,
	//arière plan
	COMPOSE_BACK = 2,
	//privé, composition des images
	COMPOSE_RESULT = 3,
	//privé, changement depuis la dèrnière frames
	COMPOSE_CHANGES = 4,
} COMPOSE_RANK;

// @brief initialise le compositeur
void    compose_init(void);
// @brief free les variables du compositeur
void    compose_free(void);


/* @brief Affiche un rectangle de couleur unie
 * 
 * @param color couleur du rectangle
 * @param rank rang d'affichage du rectangle
 * @param pos coin supérieur gauche du rectangle
 * @param size taille du rectangle
 */
void    compose_disp_rect(COLOR color, COMPOSE_RANK rank, coordonee_t pos, coordonee_t size);

/* @brief affiche une image plan demandé
 *
 * @param pict image a affciher
 * @param rank plan d'affichage
 * @param pos position du coin superieur gauche de l'image
 */
void    compose_disp_pict(picture_t pict, COMPOSE_RANK rank, coordonee_t pos);

// @brief affiche du texte a un emplacment donné, dans une boite de taille size_of_text_box positionée en pos (si le texte dépasse, il est coupé)
void    compose_disp_text(const char *text_to_display, COLOR text_color, COLOR background_color,
                          COMPOSE_RANK rank, coordonee_t pos, coordonee_t size_of_text_box);

// @brief affiche un pixel au plan demandé
void    compose_disp_pix(pixel_t pixel, COMPOSE_RANK rank, coordonee_t pos);

// @brief calcule les changements a la position x,y
void    compose_have_changed(coordonee_t pos);

// @brief Affiche a l'écran les changements
void    compose_refresh(void);

// @brief efface un pixel
void    compose_del_pix(COMPOSE_RANK rank, coordonee_t pos);

/* @brief efface une zone
 *
 * @param rank rang dans lequel éffacer
 * @param min position du coin superieur gauche de la zone a éffacer
 * @param max position du coin superieur droit de la zone a éffacer
 */
void    compose_del_area(COMPOSE_RANK rank, coordonee_t min, coordonee_t max);

#endif //def GRAPHICALH
