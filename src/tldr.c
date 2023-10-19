#include "tldr.h"

char input[NB_INPUT_CHAR];

void cleanup() {
	//fonction appellé a la sortie du programme
	graphical_cleanup();
}


int main (int argc,char **argv,char **env){
	//***setup initial***

	init_graphical();
	//initialize randomness using system time
	srand((unsigned int)time(NULL));
	//renseigne la fonction a éxécuter a la sortie du programme
	atexit(cleanup);
	//initialise les variables globales

	return rand();
}

void print_monster(monster_t monster){
	//affiche un monstre
	pict_display(monster.type->sprite, monster.posx, monster.posy);
}
