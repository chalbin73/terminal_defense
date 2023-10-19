#include "tldr.h"

char input_string[NB_INPUT_CHAR+1];

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

	return EXIT_SUCCESS;
}

void print_monster(monster_t monster){
	//affiche un monstre
	pict_display(monster.type->sprite, monster.posx, monster.posy);
}

//obtient des inputs pendant 0.1 secondes, et les mets input_string
void get_input(){
	scanf("%s",input_string);
}
