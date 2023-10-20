#include "tldr.h"

//variable globale
//(plus simple que de passer des pointeurs dans touts les sens)
char input_string[NB_INPUT_CHAR+1];
picture_t background_pict;
uint joueur_vie,joueur_score;


void cleanup() {
	//fonction appellé a la sortie du programme
	//free les variables qui trainent
	graphical_cleanup();
	free(background_pict.data);
}


int main(int argc,char **argv,char **env){
	//***setup initial***

	init_graphical();
	//initialize randomness using system time
	srand((unsigned int)time(NULL));
	//renseigne la fonction a éxécuter a la sortie du programme
	atexit(cleanup);

	//initialise les variables globales
	//mise a zero de l'input
	memset(input_string,0, sizeof(char)*(NB_INPUT_CHAR+1));
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
			if (((i/2)%2) ^ ((j/2)%2)){ //selectionne les carreaux du damier
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


//obtient les inputs et les mets input_string
void get_input(){
	scanf("%50s",input_string);	
}

//main game loop
//run until quit or die
void main_loop(uint difficulty){
	while (joueur_vie>0) {
		wait(100);
		get_input();
		joueur_vie-=1;
		printf("%s",input_string);
		if (input_string[0]=='q'){
			exit(0);
		}
	}
	return;
}
