#include <stdint.h>
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <signal.h>

#include "config.h"
#include "graphical.h"
#include "common.h"

/************************
 ****TYPEDEF ET STRUCTS***
 *************************/

typedef enum
{
	DIR_UP    = 0,
	DIR_DOWN  = 1,
	DIR_RIGHT = 2,
	DIR_LEFT  = 3,
	DIR_NOWHERE=4,//valeur spéciale, indiquant un manque d'information
} DIRECTION;
typedef enum {
	GAME_STOPED, //pas de partie en cours
	GAME_PAUSED, //partie en cours mais en pause
	GAME_PLAYING,//jeu lancé normalement
	GAME_SELECT_DEF, //jeu lancé, en train de faire un choix de défense
} GAME_STATE;


typedef struct
{
	pixel_t sprite;
	int32_t max_life;
	//en frame/cases
	//+ élevé = plus lent
	uint speed;
	//damage/frame
	uint damage;

} monster_type_t;
typedef struct monster_t
{
	const monster_type_t *type;
	int32_t vie;
	uint64_t last_action_turn;
	//pointeur vers les autres monstres dans la même case
	//permet de "simplifier" le stockage des monstre a une certaine position
	//(on en connait un, donc tout les autres en passant d'un mob a l'autre)
	struct monster_t   *next_monster_in_room;
} monster_t;

typedef struct
{
	pixel_t sprite;
	uint cost;
	uint max_life;
	uint damage;
	uint range;
	const char *ui_txt;

} defense_type_t;

typedef struct
{
	const defense_type_t *type;
	int64_t life;
} defense_t;

// Represente l'arbre de choix de defense dans le menu de selecion
typedef struct defence_choice_tree_t
{
	const char                          *ui_txt;
	pixel_t icon;

	uint16_t sub_category_count;
	const struct defence_choice_tree_t  **sub_categories;

	uint16_t defense_count;
	const defense_type_t                     **defenses;
} defence_choice_tree_t;
typedef struct {
	uint64_t distance;
	DIRECTION next;
} pathfinder_data;
/***********************************
 ***FONCTIONS UTILITAIRES DE BASES***
 ************************************/

// @brief free tout notre bordel a la fin du programme
void         cleanup(void);
/* @brief permet de restaurer l'état du terminal en cas de chrash
 *
 * @param _ Unused. Required to be registered as signal handler
 */
void sig_handler(int _);

/* @brief renvoie les coordonée du voisin (positions dans l'arène)
 *
 * @param coo coordonée pour laquelle on demande un voisin
 * @param neighbor direction du voisin demandé
 * @return les coordonée du voisin (si il existe)
 *
 * @note renvoie NO_COORDINATE si le voisin n'éxsite pas
 */
coordonee_t neighbor_of(coordonee_t coo, DIRECTION neighbor);
//renvoie la direction oposée a celle passée en argument
DIRECTION oposite_direction(DIRECTION dir);

/* @brief affiche un monstre
 *
 * @param monster monstre a afficher
 * @param pos position d'affichage
 */
void         print_monster(const monster_t *monster, coordonee_t pos);
// @brief affiche le premier monstre a la position demandée
void print_monster_at(coordonee_t pos);
/* @brief fais apparaitre un monstre
 *
 * @param type type de monstre a faire apparaitre
 * @param position endroit ou il faut faire apparaitre le monstre
 */
void spawn_monster(const monster_type_t *type, coordonee_t position);
/* @brief finalise la mort d'un monstre (le free et le sort de monster_position)
 *
 * @param monster_ptr pointeur sur le pointeur sur un monstre afin de le retirer de la liste chainée de monster_position
 */
void kill_monster(monster_t **monster_ptr);
/* @brief deplace un monstre
 *
 * @param monster_ptr pointeur sur le pointeur sur un monstre dans la liste chainée de monster_position (permet de déplacer le monstre dans cette dernière)
 * @param objective position à laquelle déplacer le monstre
 */
void    move_monster(monster_t **monster_ptr, coordonee_t objective);

//fait clignoter le curseur
void    blink_cursor(void);
//cache le curseur
void    hide_cursor(void);
//montre le curseur
void    show_cursor(void);
// @brief vide l'input clavier
void         clear_input(void);
// @brief bouge le curseur dans la direction demandée
void         move_cursor(DIRECTION dir);

/*** PATHFINDER ***/

// @brief (Ré)initialse le moteur de pathfinding
void path_reinit(void);
/* @brief Update le pathfinder a partir de la coordonée en paramètre
 *
 * @param position Position a partir de laquelle update le pathfinder
 */
void update_pathfinder_from(coordonee_t position);

/******************
 ***MOTEUR DE JEU***
 *******************/

// @brief setup initial
int          main(void);
// Pour chaque input clavier depuis la dernière frame, execute l'action associé
void    treat_input(void);
/* @brief boucle principale d'execution
 *
 * @param difficulty dificultée de la partie
 */
void         main_loop(uint difficulty);

//spawn eventuels de mobs
void randomly_spawn_mobs(int difficulty);

//routine pour l'ensemble des mobs
void monsters_routine(void);
//routine pour l'ensemble des defense
void defenses_routine(void);
//routine pour un mob
void single_monster_routine(monster_t** monster_ptr,coordonee_t position);
//routine pour une defense
void single_defense_routine(coordonee_t defense_position);
//inflige des dégats a une défense
void damage_defense(coordonee_t target_position,uint32_t damage);
//inflige des dégats a un monstre
void damage_monster(monster_t **monster_ptr,int32_t damage);


/***DEFENSE SELECTION***/

// @brief selectionne la défense
void select_defense(void);
// @brief construit une défense au niveau du curseur
void build_defense(const defense_type_t *defense_type);
// Affiche le menu de selection de defense
void    display_selection(void);
// affiche un item de la selection de défense
void    display_defense_selection_item(pixel_t icon, uint32_t indice);
// Cache le menu de selection de defense
void hide_selection(void);
// Augmente la selection de 1 (et update le selecteur graphique)
void augment_selection(void);
// Diminue la selection de 1 (et update le selecteur graphique)
void diminish_selection(void);
/*****************
 ***MONSTER POOL***
 ******************/

/* @brief Creates and initialized the pool of all monsters
 *
 * @param pool_size The size of the initial monster memory allocation
 */
void         monster_pool_create(uint32_t pool_size);
/* @brief Augmente la taille de la mémoire pour les monstres
 *
 * @param expand_size nombre de monstre a rendre disponible en plus
 */
void    monster_pool_expand(uint32_t expand_size);
// @brief Cleans up and frees the memory of the monster pool
void         monster_pool_destroy(void);

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
void         monster_pool_dealloc(monster_t   *monster);

/*
 * @brief Returns the count of alloced monsters in the pool
 *
 * @return The number of monsters alloced in monster pool, 0 if the pool is empty
 *
 */
uint32_t     monster_pool_count(void);


/*****************
 *** RESSOURCES ***
 ******************/
//definie dans common_and_ressources.c

extern const monster_type_t runner;
extern const monster_type_t armored;

extern const defense_type_t wall;
extern const defense_type_t basic_turret;
extern const defense_type_t la_base;

extern const picture_t frame;
extern const defence_choice_tree_t main_selection_tree;

