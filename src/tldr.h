#include <stdint.h>
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>

#include "config.h"
#include "graphical.h"
#include "common.h"
typedef unsigned int uint ;

typedef enum {
	DIR_UP    = 0,
	DIR_DOWN  = 1,
	DIR_RIGHT = 2,
	DIR_LEFT  = 3,
} DIRECTION;
typedef struct {
	picture_t sprite;
	uint vie_max;
	uint vitesse;
	uint degats;
	
} monster_type;


typedef struct {
	monster_type* type;
	//direction dans laquelle regarde le mob
	DIRECTION direction;
	int32_t posx;
	int32_t posy;
	uint vie;
} monster_t;



//declaration de fonctions

//affiche un monstre
void print_monster(monster_t* monster);
//deplace un monstre
void move_monster(monster_t* monster,uint new_x,uint new_y);
//mets l'input clavier dans input_string
void get_input();
//boucle principale d'execution
void main_loop(uint difficulty);
