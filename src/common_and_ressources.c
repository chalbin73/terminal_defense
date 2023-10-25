//fonctions utilitaires
#include "common.h"

//ressources
#include "tldr.h"

/****************************
 *** FONCTIONS UTILITAIRES ***
 *****************************/


void* safe_malloc(size_t size){
	//obtient de la place mémoire et vérifie qu'elle a bien été alouée
	void* ptr;
	ptr=malloc(size);
	if (ptr!=NULL){
		return ptr;
	}
	EXIT_MSG="malloc a fail! sortie de programme!";
	//la fonction cleanup *devrait* clean notre bordel
	exit(254);
}

void* safe_realloc(void* ptr,size_t new_size){
	//obtient de la place mémoire et vérifie qu'elle a bien été alouée
	void* new_ptr;
	new_ptr=realloc(ptr, new_size);
	if (new_ptr!=NULL){
		return new_ptr;
	}
	EXIT_MSG="realloc a fail! sortie du programme!";
	//la fonction cleanup *devrait* clean notre bordel
	exit(254);
}

//attend ms milliseconde
int wait(unsigned long ms){
	#if SYSTEM_POSIX
	//nanosleep accept un struct en seconds et nanoseconds
	//on convertit donc l'entrée
	struct timespec ts;
	ts.tv_sec = ms / 1000;
	ts.tv_nsec = (ms % 1000) * 1000000;
	return nanosleep(&ts, &ts);
	#else
	return Sleep(ms);
	#endif
}

/*****************
 *** RESSOURCES ***
 ******************/

const monster_type runner = {
	.speed=4,
	.damage=10,
	.max_life=200,
	.sprite = {
		.color=COL_RED,
		.background_color=COL_DEFAULT,
		.c1=0xe2, //≫
		.c2=0x89,
		.c3=0xab,
		.c4='\0',
	},
};
const monster_type armored = {
	.speed=10,
	.damage=30,
	.max_life=1000,
	.sprite = {
		.color=COL_CYAN,
		.background_color=COL_DEFAULT,
		.c1=0xe2, //⇛
		.c2=0x87,
		.c3=0x9b,
		.c4='\0',
	}
};

const defense_type_t wall = {
	.max_life=1000,
	.damage=0,
	.cost=100,
	.range=0,
	.sprite= {
		.color=COL_MAGENTA,
		.background_color=COL_DEFAULT,
		.c1=CHARS_BLOCK_FULL[0], //█
		.c2=CHARS_BLOCK_FULL[1],
		.c3=CHARS_BLOCK_FULL[2],
		.c4=CHARS_BLOCK_FULL[3],
	}
};
const defense_type_t basic_turret = {
	.max_life=100,
	.damage=150,
	.range=10,
	.cost=200,
	.sprite= {
		.color=COL_BLUE,
		.background_color=COL_YELLOW,
		.c1=0xc2, //¶
		.c2=0xb6,
		.c3='\0',
		.c4='\0',
	}
};
