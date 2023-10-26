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
	pixel_t sprite;
	uint max_life;
	//en frame/cases
	//+ élevé = plus lent
	uint speed;
	//damage/frame
	uint damage;

} monster_type;
typedef struct monster_t {
	monster_type* type;
	uint vie;
	//pointeur vers les autres monstres dans la même case
	//permet de "simplifier" le stockage des monstre a une certaine position
	//(on en connait un, donc tout les autres en passant d'un mob a l'autre)
	struct monster_t* next_monster_in_room;
} monster_t;

typedef struct {
	pixel_t sprite;
	uint cost;
	uint max_life;
	uint damage;
	uint range;
	char* ui_txt;

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
// @brief bouge le curseur dans la direction demandée
void move_cursor(DIRECTION dir);

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

/*
 * @brief Allocates a monster in the monster pool
 *
 * @returns A pointer to a empty monster_t structure which can be used freely
 */
monster_t   *monster_pool_alloc(void);

/*
 * @brief Deallocates a monster slot in the monster pool
 *
 * @param ptr A valid pointer to a slot in the monster pool which is to be freed
 * @note Nothing happens if pool is unitialized or empty or ptr is null.
 *       if ptr is not a valid pointer given by monster_pool_alloc, behavior is unspecified.
 */
void    monster_pool_dealloc(monster_t   *monster);

/*
 * @brief Returns the count of alloced monsters in the pool
 *
 * @return The number of monsters alloced in monster pool, 0 if the pool is empty
 *
 */
uint32_t    monster_pool_count(void);


/*****************
 *** RESSOURCES ***
 ******************/
//definie dans common_and_ressources.c

extern const monster_type runner;
extern const monster_type armored;

extern const defense_type_t wall;
extern const defense_type_t basic_turret;
