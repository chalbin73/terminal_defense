#include "tldr.h"

//variable globale
//(plus simple que de passer des pointeurs dans touts les sens)
picture_t background_pict;
uint joueur_vie,joueur_score;
char* EXIT_MSG="Crash while initializing ...";

void cleanup() {
	//fonction appellé a la sortie du programme
	//free les variables qui trainent
	graphical_cleanup();
	clear_input();
	free(background_pict.data);
	printf("%s",EXIT_MSG);
}


int main(int argc,char **argv,char **env){
	//***setup initial***

	init_graphical();
	//initialize randomness using system time
	srand((unsigned int)time(NULL));
	//renseigne la fonction a éxécuter a la sortie du programme
	atexit(cleanup);

	//initialise les variables globales
	//creation du background
	int reserved=15;
	background_pict.col=termsize.ws_col-reserved;
	background_pict.row=termsize.ws_row;
	background_pict.stride=background_pict.col;
	background_pict.data=(pixel_t*)safe_malloc(sizeof(pixel_t)*background_pict.col*background_pict.row);
	for (int i=0; i<background_pict.col; i++){
		for (int j=0; j<background_pict.row; j++) {
			pixel_t pixel=(pixel_t){
				.c1=' ',         //le caractères étant un simple ascii (un espace),
				.c2='\0',        //il ne prend que c1, les autres sont donc nulls
				.c3='\0',
				.c4='\0',
				.color=COL_DEFAULT,
				.background_color=0,
			};
			if (((i%5)==2) && ((j%5)==2)){ //selectionne les carreaux du damier
				pixel.background_color=COL_BOARD_BACKGROUND_1;
			} else {
				pixel.background_color=COL_BOARD_BACKGROUND_2;
			}
			background_pict.data[i+j*background_pict.stride] = pixel;
		}
	}
	pict_display(background_pict, 0, 0);
	//autre variables globales
	joueur_vie=1000;
	joueur_score=0;

	//on lance le jeu
	EXIT_MSG="Crashing whitout more precision while game was running";
	main_loop(10);
	return EXIT_SUCCESS;
}


void print_monster(monster_t* monster){
	//affiche un monstre
	pict_display(monster->type->sprite, monster->posx, monster->posy);
}
void move_monster(monster_t* monster,uint new_x,uint new_y){
	picture_t monster_sprite=monster->type->sprite;
	//on efface l'ancien monstre en affichant le background a son ancienne position.
	picture_t to_print=pict_crop_size(background_pict, monster->posx, monster_sprite.col, monster->posy, monster_sprite.row);
	pict_display(to_print, monster->posx, monster->posy);
	//puis on réaffiche le nouveau monstre, et on change sa position
	pict_display(monster_sprite, new_x, new_y);
	monster->posx=new_x;
	monster->posy=new_y;
}


//enlève tout les inputs claviers non traitées
void clear_input(){
	while (read(STDIN_FILENO, NULL, 20)) {

	}
}

//obtient et traite les inputs claviers
void treat_input(){
	char input;
	while (read(STDIN_FILENO,&input,1)) {
		switch (input) {
		case '\e':
			//Le caractère d'échapement est présent devant plein de trucs spéciaux (Eg F1)
			//trop compliquer a parser, on détruit l'input
			clear_input();
			break;
		case '\3':
		case '\4':
		case KEY_QUIT:
			EXIT_MSG="Interupted by user, quiting";
			exit(130);

		}
	}
}

//main game loop
//run until quit or die
void main_loop(uint difficulty){
	while (joueur_vie>0) {
		wait(100);
		treat_input();
		joueur_vie-=difficulty;

	}
	return;
}
