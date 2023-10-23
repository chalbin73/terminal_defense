#include "tldr.h"

/* ************************
 ***VARIABLES GLOBALES***
 ************************/
char* EXIT_MSG;

uint joueur_vie,joueur_score;
tab_size_t arena_size;
uint cursor_x,cursor_y; //position du curseur
bool cursor_is_shown;
pixel_t cursor_pixel;


monster_t *monster_pool_head;
void **monster_memories           = NULL;
uint32_t monster_memories_count = 0;
uint32_t max_monsters           = 0;
uint32_t alloced_monsters       = 0;

//associe a chaque case de l'arenne un monstre/une construction.
//chaque monstre pointe sur les autres monstres dans la même case (liste chainée)
//une seule construction est autorisée par cases
monster_t **monster_positions;

/***********************************
 ***FONCTIONS UTILITAIRES DE BASES***
 ************************************/

void cleanup() {
	//fonction appellé a la sortie du programme

	//free les variables qui trainent
	clear_input();
	graphical_cleanup();
	monster_pool_destroy();
	free(monster_positions);
	//exit reason
	printf("%s\n",EXIT_MSG);
}

void    print_monster(monster_t   *monster, int posx, int posy){
	//affiche un monstre
	compose_disp_pix(monster->type->sprite,COMPOSE_ARENA, posx, posy);
}

void    move_monster(monster_t *monster,monster_t** previous_ptr, uint new_x, uint new_y){
	//on retire le montre de son ancienne case
	//en faisant pointer le précédant sur le suivant
	*previous_ptr=monster->next_monster_in_room;
	//puis on ajoute le monstre au début de la liste de la case suivante
	//et on fait pointer le monstre sur les autres de la case
	monster->next_monster_in_room=monster_positions[new_x+new_y*arena_size.stride];
	monster_positions[new_x+new_y*arena_size.stride]=monster;
}

//enlève tout les inputs claviers non traitées
void clear_input(){
	char poubelle[20];
	while (read(STDIN_FILENO, poubelle, 20)) {
		//le but étant de vider stdin, on ne fais rien avec les charactères ...
	}
}
void show_cursor(){
	compose_disp_pix(cursor_pixel, COMPOSE_UI, cursor_x, cursor_y);
	cursor_is_shown=true;
}
void hide_cursor(){
	compose_del_pix(COMPOSE_UI,cursor_x,cursor_y);
	cursor_is_shown=false;
}
void blink_cursor(){
	if(cursor_is_shown){
		hide_cursor();
	} else {
		show_cursor();
	}
}

void move_cursor(DIRECTION dir){
	hide_cursor();
	switch (dir) {
	case DIR_UP:
		if (cursor_y>1) {
			cursor_y-=1;
		} break;
	case DIR_DOWN:
		if (cursor_y<arena_size.row-2) {
			cursor_y+=1;
		} break;
	case DIR_LEFT:
		if (cursor_x>1) {
			cursor_x-=1;
		} break;
	case DIR_RIGHT:
		if (cursor_x<arena_size.col-2) {
			cursor_x+=1;
		}
	}
}


/******************
 ***MOTEUR DE JEU***
 *******************/


int     main(){
	//si jamais ...
	EXIT_MSG="Crash while initializing ...";
	//***setup initial***

	init_graphical();

	//initialize randomness using system time
	srand( (unsigned int)time(NULL) );
	//renseigne la fonction a éxécuter a la sortie du programme
	atexit(cleanup);

	//*initialise les variables globales*

	//creation du background
	int reserved=20;
	//taille de l'arène
	arena_size.col=termsize.col-reserved;
	arena_size.stride=arena_size.col;
	arena_size.row=termsize.row;

	//initialisation du background avec son patterne
	pixel_t pixel=(pixel_t){
		.c1=' ',                 //le caractères étant un simple ascii (un espace),
		.c2='\0',                //il ne prend que c1, les autres sont donc nulls
		.color=COL_DEFAULT,
	};
	for (int i=0; i<arena_size.col; i++){
		for (int j=0; j<arena_size.row; j++) {
			if (((i%5)==2) || ((j%5)==2)){ //selectionne des lignes verticales et horizontales éspacé de 5 cases
				pixel.background_color=COL_BOARD_BACKGROUND_1;
			} else {
				pixel.background_color=COL_BOARD_BACKGROUND_2;
			}
			compose_disp_pix(pixel, COMPOSE_BACK, i, j);
		}
	}
	pixel.background_color=COL_DEFAULT;
	for (int i=arena_size.col; i<termsize.col; i++){
		for (int j=0; j<termsize.row; j++) {
			compose_disp_pix(pixel, COMPOSE_BACK, i,j);
		}
	}
	//initialisation du curseur


	monster_pool_create(200);
	//creation (et initialisation a zero) de monster_position
	monster_positions=safe_malloc(sizeof(monster_t*)*arena_size.row*arena_size.col);
	memset(monster_positions, (long int)NULL, sizeof(monster_t*)*arena_size.row*arena_size.col);
	//autre variables globales
	joueur_vie   = 1000;
	joueur_score = 0;
	cursor_pixel = (pixel_t){
		.c1=' ',
		.c2='\0',
		.color=COL_DEFAULT,
		.background_color=COL_CURSOR,
	};

	EXIT_MSG="Crashing whithout more precision while game was running";

	//on lance le jeu
	main_loop(10);
	EXIT_MSG="";
	return EXIT_SUCCESS;
}


//obtient et traite les inputs claviers
void treat_input(){
	char input;
	while (read(STDIN_FILENO,&input,1)) {  // read se comporte comme scanf("%c",&input), a l'éxeption de ne pas etre bugée
		switch (input) {
		case '\e':
			//Le caractère d'échapement est présent devant plein de trucs spéciaux (Eg F1)
			//trop compliquer a parser, on détruit l'input
			clear_input();
			break;
		case KEY_QUIT:
			EXIT_MSG="Interupted by user, quiting";
			exit(130);
		case KEY_UP:
			move_cursor(DIR_UP); break;
		case KEY_DOWN:
			move_cursor(DIR_DOWN); break;
		case KEY_LEFT:
			move_cursor(DIR_LEFT); break;
		case KEY_RIGHT:
			move_cursor(DIR_RIGHT); break;

		}
	}
}

//main game loop
//run until quit or die
void main_loop(uint difficulty){
	while (joueur_vie>0) {
		treat_input();
		blink_cursor();
		compose_refresh();

		wait(100);
	}
	return;
}
// Augmente la taille de la mémoire de monstre
void monster_pool_expand(uint32_t expand_size){
	//allocation d'un nouveau morceau de mémoire pour les monstre
	monster_memories_count+=1;
	monster_memories=safe_realloc(monster_memories, monster_memories_count*sizeof(monster_t*));

	monster_t* new_pool=(monster_t*)safe_malloc(expand_size*sizeof(monster_t));
	monster_memories[monster_memories_count-1]=(void*)new_pool;

	max_monsters+=expand_size;

	//remplissage de la nouvelle mémoire avec des header (monstres ne possédant qu'un pointeur)
	for (uint32_t i=0;i<expand_size-1;i++){
		new_pool[i]=(monster_t){
			.next_monster_in_room=&new_pool[i+1],
		};
	}
	//on connecte la nouvelle pool au reste
	new_pool[expand_size-1]=(monster_t){
		.next_monster_in_room=monster_pool_head,
	};
	monster_pool_head=new_pool;
}


// Creates and initialized the pool of all monsters
void    monster_pool_create(uint32_t pool_size)
{
	// Allocate the memory to store the monsters
	monster_memories         = NULL;
	monster_pool_head      = NULL;
	max_monsters           = 0;
	alloced_monsters       = 0;
	monster_memories_count = 0;
	
	monster_pool_expand(pool_size);
}

// Cleans up and frees the memory of the monster pool
void    monster_pool_destroy(void)
{
	if(monster_memories == NULL)
		return;
	for(int i=0;i<monster_memories_count;i++){
		free(monster_memories[i]);
	}
	free(monster_memories);
	monster_pool_head      = NULL;
	monster_memories       = NULL;
	alloced_monsters       = 0;
	max_monsters           = 0;
	monster_memories_count = 0;
}

/*
 * @brief Allocates a monster in the monster pool
 *
 * @returns A pointer to a empty monster_t structure which can be used freely
 */
monster_t   *monster_pool_alloc(void)
{
	if (
		max_monsters == alloced_monsters
		|| monster_memories == NULL
		|| monster_pool_head == NULL
		)
		monster_pool_expand(max_monsters);
	void *res = monster_pool_head;
	monster_pool_head = monster_pool_head->next_monster_in_room;
	alloced_monsters++;
	return res;

}

// Deallocates a monster slot in the monster pool
void    monster_pool_dealloc(monster_t   *monster)
{
	if(monster == NULL ){
		return;
	}

	alloced_monsters--;
	monster->next_monster_in_room=monster_pool_head;
	monster_pool_head = monster;
}

// Returns the count of alloced monsters in the pool
uint32_t    monster_pool_count(void)
{
	return alloced_monsters;
}

