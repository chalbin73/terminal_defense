#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>


#include "config.h"
#include "graphical.h"
#include "common.h"
typedef unsigned int uint ;

enum DIRECTION {
	DIR_UP    = 0,
	DIR_DOWN  = 1,
	DIR_RIGHT = 2,
	DIR_LEFT  = 3,
};

typedef struct {
	picture_t sprite;
	enum DIRECTION direction;
	int32_t posx;
	int32_t posy;
	uint32_t id;
} monster_t;



//declaration de fonctions

//affiche un monstre
void print_monster(monster_t monster);
