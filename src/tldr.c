#include "tldr.h"

/* ************************
 ***VARIABLES GLOBALES***
 ************************/
const char *EXIT_MSG;

uint64_t joueur_vie, joueur_score, turn, joueur_ressources;
tab_size_t arena_size;
coordonee_t cursor_pos; //position du curseur
bool cursor_is_shown;
pixel_t cursor_pixel;

// Taille de la zone réservée à droite
int reserved = 20;

monster_t *monster_pool_head;
void **monster_memories         = NULL;
uint32_t monster_memories_count = 0;
uint32_t max_monsters           = 0;
uint32_t alloced_monsters       = 0;

//associe a chaque case de l'arenne un monstre/une construction.
//chaque monstre pointe sur les autres monstres dans la même case (liste chainée)
monster_t **monster_positions;
//associe a chaque case de l'arenne une défense
defense_t *defense_array;
//associe a chaque case de l'arenne une distance de la base et la direction dans laquelle aller
//pour s'en rapprocher (pathfinding)
pathfinder_data *pathfinder_array;
//utilisé par les fonction du pathfinder
//mais comme c'est un gros tableau, on ne l'alloue qu'une fois au debut
coordonee_t *position_list;
//la taille du tableau ci dessus
uint32_t pos_list_size;
//coordonées de la base
coordonee_t base_coordinate;
// État du jeu
GAME_STATE game_state             = GAME_STOPED;
//arbre de séléction des défense actuellement affiché
const defence_choice_tree_t *shown_tree = NULL;
// Index selectionné dans le menu
int32_t sel_index                       = 0;
static const pixel_t selection_indicator =
{
	.background_color = COL_DEFAULT,
	.color            = COL_GREEN,
	.c1               = '*',
	.c2               = 0,
};
static const coordonee_t NO_COORDINATE={
	.x=-1,
	.y=0,
};


/***********************************
 ***FONCTIONS UTILITAIRES DE BASES***
 ************************************/

//renvoi l'offset associé a un couple coo-stride
int32_t offset_of(coordonee_t coo,int32_t stride){
	return coo.x + coo.y*stride;
}

//renvoie les coordonée du voisin (positions dans l'arène)
coordonee_t neighbor_of(coordonee_t coo, DIRECTION neighbor){
	switch (neighbor) {
	case DIR_LEFT:
		//a gauche => posx-=1
		if (coo.x==0) return NO_COORDINATE;
		coo.x-=1;
		return coo;
	case DIR_RIGHT:
		//a droite => posx+=1
		if (coo.x==arena_size.col-1) return NO_COORDINATE;
		coo.x+=1;
		return coo;
	case DIR_UP:
		//en haut => posy-=1
		if (coo.y==0) return NO_COORDINATE;
		coo.y-=1;
		return coo;
	case DIR_DOWN:
		//en bas => posy+=1
		if (coo.y==arena_size.row-1) return NO_COORDINATE;
		coo.y+=1;
		return coo;
	default:
		return NO_COORDINATE;
	}
}

//renvoie la direction opposé
DIRECTION oposite_direction(DIRECTION dir){
	switch (dir) {
	case DIR_DOWN:
		return DIR_UP;
	case DIR_UP:
		return DIR_DOWN;
	case DIR_RIGHT:
		return DIR_LEFT;
	case DIR_LEFT:
		return DIR_RIGHT;
	default:
		return DIR_NOWHERE;
	}
}


void    cleanup(void)
{
	//fonction appellé a la sortie du programme

	//free les variables qui trainent
	clear_input();
	graphical_cleanup();
	monster_pool_destroy();
	free(monster_positions);
	free(defense_array);
	free(pathfinder_array);
	//affiche la raison d'éxit
	printf("%s\n", EXIT_MSG);
}
//affiche le premier monstre a la position demandée
void print_monster_at(coordonee_t pos){
	print_monster(monster_positions[offset_of(pos,arena_size.stride)], pos);
}

//affiche un monstre (et clear si monster et le pointeur null)
void    print_monster(monster_t *monster, coordonee_t pos)
{
	//affiche un monstre
	if (monster==NULL){
		compose_del_pix(COMPOSE_ARENA,pos);
	}else {
		compose_disp_pix(monster->type->sprite, COMPOSE_ARENA, pos);
	}
}

//fais spawn un monstre
void spawn_monster(monster_type *type, coordonee_t position){
	monster_t *monster=monster_pool_alloc();
	*monster=(monster_t){
		.type=type,
		.vie=type->max_life,
		.next_monster_in_room=monster_positions[offset_of(position, arena_size.stride)],
	};
	monster_positions[offset_of(position, arena_size.stride)]=monster;
	print_monster(monster,position);
}
//finalise la mort d'un monstre
void kill_monster(monster_t *monster, monster_t **previous_ptr){
	*previous_ptr=monster->next_monster_in_room;
	monster_pool_dealloc(monster);
}

void    move_monster(monster_t *monster, monster_t **previous_ptr, coordonee_t monster_pos, DIRECTION direction)
{
	//on retire le montre de son ancienne case
	//en faisant pointer le précédant sur le suivant
	*previous_ptr = monster->next_monster_in_room;
	//on update l'affichage
	print_monster_at(monster_pos);
	//on obtient la nouvelle case
	coordonee_t new_pos=neighbor_of(monster_pos, direction);
	if (new_pos.x==-1){
		//ceci est un bug
		EXIT_MSG="bugged monster moved out of screen! Report a pthfinding bug";
		exit(2);
	}
	//on ajoute le monstre au début de la liste de la case suivante
	//et on fait pointer le monstre sur les autres de la case
	monster->next_monster_in_room                        = monster_positions[offset_of(new_pos, arena_size.stride)];
	monster_positions[offset_of(new_pos, arena_size.stride)] = monster;
	//et on update l'affichage
	print_monster(monster, new_pos);
}

//enlève tout les inputs claviers non traitées
void    clear_input(void)
{
	char poubelle[20];
	while ( read(STDIN_FILENO, poubelle, 20) )
	{
		//le but étant de vider stdin, on ne fais rien avec les charactères ...
	}
}

/*** CURSOR ***/

void    show_cursor(void)
{
	compose_disp_pix(cursor_pixel, COMPOSE_UI, cursor_pos);
	cursor_is_shown = true;
}
void    hide_cursor(void)
{
	compose_del_pix(COMPOSE_UI, cursor_pos);
	cursor_is_shown = false;
}
void    blink_cursor(void)
{
	if(cursor_is_shown)
	{
		hide_cursor();
	}
	else
	{
		show_cursor();
	}
}

void    move_cursor(DIRECTION dir)
{
	hide_cursor();
	coordonee_t new_pos=neighbor_of(cursor_pos, dir);
	if (new_pos.x!=-1) {
		cursor_pos=new_pos;
	}
	show_cursor();
}
/*** PATHFINDER ***/
// (re) initilaise le pathfinder array
void path_reinit(void){
	for (int i=0; i<arena_size.col*arena_size.row; i++) {
		pathfinder_array[i]=(pathfinder_data){
			.next=DIR_NOWHERE,
			.distance=UINT64_MAX,
		};
	}
	update_pathfinder_from(base_coordinate);
}
//update le pathfinder a partir de la position demandée
void update_pathfinder_from(coordonee_t position){
	//indice déja traité
	uint borne_inf=0;
	//indice jusqu' auquel la liste est remplie
	uint borne_sup=1;

	//la ou nous a demander d'update
	position_list[0]=position;
	pathfinder_array[offset_of(position, arena_size.stride)].next=DIR_NOWHERE;

	//utilisé dans la boucle
	pathfinder_data here_before;
	pathfinder_data here_after;

	//tant qu'il reste des case a traiter
	while (borne_inf!=borne_sup) {
		//position de la case a traiter
		position=position_list[borne_inf];
		//indice dans les tableau
		here_before=pathfinder_array[offset_of(position, arena_size.stride)];

		if (here_before.next!=DIR_NOWHERE){
			//cette position a déja été update (présente plusieurs fois dans la liste), on skip
			borne_inf++;
			if (borne_inf==pos_list_size)borne_inf=0;
			continue;
		}//else ....

		//position de la base (c'est ici que les mobs doivent arriver: distance de 0)
		if (position.x==base_coordinate.x && position.y==base_coordinate.y){
			here_after=(pathfinder_data){
				.distance=0,
				.next=DIR_RIGHT, //on sort de l'écran, mais cela n'arrive que si la base est détruite (le jeu s'arrête de toute facon)
			};
		} else {
			here_after=(pathfinder_data){
				.distance=UINT64_MAX,
				.next=DIR_NOWHERE,
			};
			for (DIRECTION direction=DIR_UP; direction<DIR_NOWHERE; direction++) {
				//pour chaque direction cardinale
				coordonee_t neighbor=neighbor_of(position, direction);
				if (neighbor.x!=-1){//si le voisin existe
					int32_t neighbor_offset=offset_of(neighbor, arena_size.stride);
					//si le voisin ne pointe pas sur nous et n'est pas undef/en cours de recalcul (DIR_NOWHERE)
					if (pathfinder_array[neighbor_offset].next != oposite_direction(direction)
					    && pathfinder_array[neighbor_offset].next != DIR_NOWHERE){
						//si il est intéréssant
						if (pathfinder_array[neighbor_offset].distance < here_after.distance){
							//on passe par lui
							here_after=(pathfinder_data){
								.distance=pathfinder_array[neighbor_offset].distance,
								.next=direction
							};
						}
					}
				}
			}//fin du for

			here_after.distance+=1;
			if (defense_array[offset_of(position, arena_size.stride)].type != NULL){
				//on est sur une défense, on augmente la "taille" du chemin selon la vie
				here_after.distance+=defense_array[offset_of(position, arena_size.stride)].life/100;
			}
		}//fin du if(pos_base)

		pathfinder_array[offset_of(position, arena_size.stride)]=here_after;
		//si on est undef ou qu'on s'est amélioré
		if (here_after.next == DIR_NOWHERE || here_after.distance<here_before.distance){
			//on dit a tous les voisins de s'update
			for (DIRECTION direction=DIR_UP; direction<DIR_NOWHERE; direction++) {
				//pour chaue direction cardinale
				coordonee_t neighbor=neighbor_of(position, direction);
				if (neighbor.x!=-1){    //si le voisin existe
					borne_sup+=1;
					if (borne_sup==pos_list_size)borne_sup=0;
					position_list[borne_sup]=neighbor;
					//si se voisin pointait sur nous, on le fait pointer sur rien
					if (pathfinder_array[offset_of(neighbor,arena_size.stride)].next==oposite_direction(direction)){
						pathfinder_array[offset_of(neighbor,arena_size.stride)].next=DIR_NOWHERE;
					}
				}
			}//fin du for
		}

		borne_inf++;
		if (borne_inf==pos_list_size)borne_inf=0;
	}// fin du while
}

/******************
 ***MOTEUR DE JEU***
 *******************/


int    main()
{
	//si jamais ...
	EXIT_MSG = "Crash while initializing ...";
	//***setup initial***

	init_graphical();

	//initialize randomness using system time
	srand( (unsigned int)time(NULL) );
	//renseigne la fonction a éxécuter a la sortie du programme
	atexit(cleanup);

	//*initialise les variables globales*

	//creation du background
	//taille de l'arène
	arena_size.col    = termsize.col - reserved;
	arena_size.stride = arena_size.col;
	arena_size.row    = termsize.row;

	//initialisation du background avec son patterne
	pixel_t pixel = (pixel_t)
	{
		.c1    = ' ',            //le caractères étant un simple ascii (un espace),
		.c2    = '\0',           //il ne prend que c1, les autres sont donc nulls
		.color = COL_DEFAULT,
	};
	for (int i = 0; i<arena_size.col; i++)
	{
		for (int j = 0; j<arena_size.row; j++)
		{
			if ( ( (i % 5)==2 ) || ( (j % 5)==2 ) ) //selectionne des lignes verticales et horizontales éspacé de 5 cases
			{
				pixel.background_color = COL_BOARD_BACKGROUND_1;
			}
			else
			{
				pixel.background_color = COL_BOARD_BACKGROUND_2;
			}
			compose_disp_pix(pixel, COMPOSE_BACK, (coordonee_t){i,j});
		}
	}
	pixel.background_color = COL_DEFAULT;
	for (int i = arena_size.col; i<termsize.col; i++)
	{
		for (int j = 0; j<termsize.row; j++)
		{
			compose_disp_pix(pixel, COMPOSE_BACK, (coordonee_t){i,j});
		}
	}


	monster_pool_create(200);
	//creation (et initialisation a zero) de monster_position
	monster_positions = safe_malloc(sizeof(monster_t *) * arena_size.row * arena_size.col);
	memset(monster_positions, (long int)NULL, sizeof(monster_t *) * arena_size.row * arena_size.col);
	//autre variables globales
	defense_array=safe_malloc(arena_size.col*arena_size.row*sizeof(defense_t));
	pathfinder_array=safe_malloc(arena_size.col*arena_size.row*sizeof(pathfinder_data));
	position_list=safe_malloc(arena_size.col*arena_size.row*10*sizeof(coordonee_t));
	pos_list_size=arena_size.col*arena_size.row*10;
	base_coordinate=(coordonee_t){
		.x=arena_size.col-1,
		.y=arena_size.row/2,
	};

	joueur_vie   = 1000;
	joueur_ressources = 500;
	joueur_score = 0;

	cursor_pos=base_coordinate;
	build_defense(&la_base);

	cursor_pixel = (pixel_t)
	{
		.c1               = ' ',
		.c2               = '\0',
		.color            = COL_DEFAULT,
		.background_color = COL_CURSOR,
	};

	EXIT_MSG = "Crashing whithout more precision while game was running";

	//on lance le jeu
	game_state=GAME_PLAYING;
	turn=0;
	main_loop(10);
	EXIT_MSG = "";
	return EXIT_SUCCESS;
}


//obtient et traite les inputs claviers
void    treat_input(void)
{
	char input;
	while ( read(STDIN_FILENO, &input, 1) )    // read se comporte comme scanf("%c",&input), a l'éxeption de ne pas etre bugée
	{
		switch (input)
		{
		case '\33':
			//Le caractère d'échapement est présent devant plein de trucs spéciaux (Eg F1)
			//trop compliquer a parser, on détruit l'input
			clear_input();
			break;
		case KEY_QUIT:
			EXIT_MSG = "Interupted by user, quiting";
			exit(130);
		case KEY_UP:
			if (game_state==GAME_PLAYING)
				move_cursor(DIR_UP);
			else if (game_state==GAME_SELECT_DEF)
				augment_selection();
			break;
		case KEY_DOWN:
			if (game_state==GAME_PLAYING)
				move_cursor(DIR_DOWN);
			else if (game_state==GAME_SELECT_DEF)
				diminish_selection();
			break;

		case KEY_LEFT:
			if (game_state==GAME_PLAYING)
				move_cursor(DIR_LEFT);
			break;
		case KEY_RIGHT:
			if (game_state==GAME_PLAYING)
				move_cursor(DIR_RIGHT);
			if(game_state==GAME_SELECT_DEF)
			{
				select_defense();
			}
			break;
		case KEY_BUILD:
			select_defense();
			break;
		}
	}
}
//construit une defense a la position du curseur
void build_defense(const defense_type_t *defense_type){
	if (joueur_ressources<defense_type->cost) {
		//not enough resources
		return;
	}
	joueur_ressources-=defense_type->cost;
	defense_array[offset_of(cursor_pos, arena_size.stride)] = (defense_t){
		.type=defense_type,
		.life=defense_type->max_life,
	};
	compose_disp_pix(defense_type->sprite, COMPOSE_ARENA, cursor_pos);
	update_pathfinder_from(cursor_pos);
}

void select_defense(void){
	if (game_state==GAME_PLAYING)
	{
		game_state=GAME_SELECT_DEF;
		shown_tree=&main_selection_tree;
		sel_index=0;
		display_selection();
		return;
	}
	if (game_state==GAME_SELECT_DEF){
		hide_selection();
		//sel_index devrait etre une valeure légale car on a bien codé le reste
		if (sel_index<shown_tree->sub_category_count){
			sel_index=0;
			shown_tree=shown_tree->sub_categories[sel_index];
			display_selection();
			return;
		} //else
		sel_index-=shown_tree->sub_category_count;
		build_defense(shown_tree->defenses[sel_index]);
		game_state=GAME_PLAYING;
	}
}

// Affiche un item de choix
void    display_defense_selection_item(pixel_t icon, uint32_t indice)
{
	int32_t posx=termsize.col-reserved+1; //a gauche de la barre de droite
	int32_t posy=termsize.row-indice*3-3; //en bas, par pas de 3 (taille d'un icone)
	compose_disp_pict(frame, COMPOSE_UI, (coordonee_t){posx,posy});
	compose_disp_pix(icon, COMPOSE_UI, (coordonee_t){posx + 1, posy + 1});
}

// Affiche le menu de selection de defense
void    display_selection(void)
{
	uint32_t indice=0;
	for(int i = 0; i < shown_tree->sub_category_count; i++)
	{
		display_defense_selection_item(shown_tree->sub_categories[i]->icon, indice);
		indice += 1;
	}

	for(int i = 0; i < shown_tree->defense_count; i++)
	{
		display_defense_selection_item(shown_tree->defenses[i]->sprite, indice);
		indice += 1;
	}


	//affichage de la selection de l'élément en bas
	compose_disp_pix(selection_indicator, COMPOSE_UI, (coordonee_t){termsize.col-reserved, termsize.row-2});

}
void hide_selection(void){
	//on clean l'entièreté de la colone de droite (ou il n'y a normalement que ca dans le niveau UI)
	compose_del_area(COMPOSE_UI, (coordonee_t){termsize.col-reserved, 0}, (coordonee_t){termsize.col-1, termsize.row-1});
}


void augment_selection(void)
{
	//on cache l'ancienne selection
	compose_del_pix(COMPOSE_UI, (coordonee_t){termsize.col-reserved, termsize.row-2-3*sel_index});
	sel_index++;
	//si on dépasse le maximum, on retourne a 0
	if (sel_index >= shown_tree->defense_count + shown_tree->sub_category_count){
		sel_index=0;
	}
	//on affiche la nouvelle selection
	compose_disp_pix(selection_indicator, COMPOSE_UI, (coordonee_t){termsize.col-reserved, termsize.row-2-3*sel_index});
}
void diminish_selection(void)
{
	//on cache l'ancienne selection
	compose_del_pix(COMPOSE_UI, (coordonee_t){termsize.col-reserved, termsize.row-2-3*sel_index});
	sel_index--;
	//si passe en dessous de 0, on retourne au maximum
	if (sel_index < 0){
		sel_index = shown_tree->defense_count + shown_tree->sub_category_count-1;
	}
	//on affiche la nouvelle selection
	compose_disp_pix(selection_indicator, COMPOSE_UI, (coordonee_t){termsize.col-reserved, termsize.row-2-3*sel_index});
}

//main game loop
//run until quit or die
void    main_loop(uint difficulty)
{
	while (joueur_vie>0)
	{
		if (turn%2)blink_cursor();
		treat_input();
		compose_refresh();

		wait(100);
		turn+=1;
	}
	return;
}

/*****************************
 *** MONSTER POOL UTILITIES ***
 ******************************/


// Augmente la taille de la mémoire de monstre
void    monster_pool_expand(uint32_t expand_size)
{
	//allocation d'un nouveau morceau de mémoire pour les monstre
	monster_memories_count += 1;
	monster_memories        = safe_realloc( monster_memories, monster_memories_count * sizeof(monster_t *) );

	monster_t *new_pool = (monster_t *)safe_malloc( expand_size * sizeof(monster_t) );
	monster_memories[monster_memories_count - 1] = (void *)new_pool;

	max_monsters += expand_size;

	//remplissage de la nouvelle mémoire avec des header (monstres ne possédant qu'un pointeur)
	for (uint32_t i = 0; i<expand_size - 1; i++)
	{
		new_pool[i] = (monster_t)
		{
			.next_monster_in_room = &new_pool[i + 1],
		};
	}
	//on connecte la nouvelle pool au reste
	new_pool[expand_size - 1] = (monster_t)
	{
		.next_monster_in_room = monster_pool_head,
	};
	monster_pool_head = new_pool;
}


// Creates and initialized the pool of all monsters
void    monster_pool_create(uint32_t pool_size)
{
	// Allocate the memory to store the monsters
	monster_memories       = NULL;
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
	for(uint i = 0; i<monster_memories_count; i++)
	{
		free(monster_memories[i]);
	}
	free(monster_memories);
	monster_pool_head      = NULL;
	monster_memories       = NULL;
	alloced_monsters       = 0;
	max_monsters           = 0;
	monster_memories_count = 0;
}

// Allocates a monster in the monster pool
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
	if(monster == NULL )
	{
		return;
	}

	alloced_monsters--;
	monster->next_monster_in_room = monster_pool_head;
	monster_pool_head             = monster;
}

// Returns the count of alloced monsters in the pool
uint32_t    monster_pool_count(void)
{
	return alloced_monsters;
}

