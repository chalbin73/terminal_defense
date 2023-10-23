#include <stdint.h>
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

#include "config.h"
#include "graphical.h"
#include "common.h"

/************************
 ****TYPEDEF ET STRUCTS***
 *************************/

typedef unsigned int uint;

typedef enum {
	DIR_UP    = 0,
	DIR_DOWN  = 1,
	DIR_RIGHT = 2,
	DIR_LEFT  = 3,
} DIRECTION;

typedef struct {
	picture_t sprite;
	uint max_life;
	uint speed;
	uint damage;

} monster_type;
typedef struct monter_t {
	monster_type* type;
	//direction dans laquelle regarde le mob
	DIRECTION direction;
	uint vie;
	//pointeur vers les autres monstres dans la même case
	//permet de "simplifier" le stockage des monstre a une certaine position
	//(on en connait un, donc tout les autres en parcourant la liste)
	struct monter_t* next_monster_in_room;
} monster_t;

typedef struct monster_pool_empty_header
{
	struct monster_pool_empty_header   *next;
} monster_pool_empty_header;

typedef struct {
	pixel_t sprite;
	uint cost;
	uint max_life;
	uint damage;
	uint range;
} defense_type_t;
typedef struct {
	defense_type_t* type;
	uint life;
} defense_t;

/***********************************
 ***FONCTIONS UTILITAIRES DE BASES***
 ************************************/

// @brief free tout notre bordel a la fin du programme
void cleanup();
/* @brief affiche un monstre
 *
 * @param monster monstre a afficher
 * @param posx colonne d'affichage
 * @param posy ligne d'affichage
 */
void print_monster(monster_t* monster, int posx, int posy);
/* @brief deplace un monstre
 *
 * @param monster monstre a déplacer
 * @param previous_ptr pointeur sur le (pointeur de) monstre précédant
 * @param new_x colonne dans laquelle déplacer le monstre
 * @param new_y ligne dans laquelle déplacer le monstre
 */
void    move_monster(monster_t *monster,monster_t** previous_ptr, uint new_x, uint new_y);
// @brief vide l'input clavier
void clear_input();


/******************
 ***MOTEUR DE JEU***
 *******************/

// @brief setup initial
int main();
/* @brief boucle principale d'execution
 *
 * @param difficulty dificultée de la partie
 */
void main_loop(uint difficulty);

/*****************
 ***MONSTER POOL***
 ******************/

/* @brief Creates and initialized the pool of all monsters
 *
 * @param pool_size The maximum amount of monster that can be allocated inside the pool
 */
void    monster_pool_create(uint32_t pool_size);
// @brief Cleans up and frees the memory of the monster pool
void    monster_pool_destroy(void);
