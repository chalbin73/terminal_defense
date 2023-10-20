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

picture_t background_pict;
uint joueur_vie, joueur_score;

typedef struct monster_pool_empty_header
{
    struct monster_pool_empty_header   *next;
} monster_pool_empty_header;

monster_pool_empty_header *monster_pool_head;
void *monster_memory      = NULL;
uint32_t max_monsters     = 0;
uint32_t alloced_monsters = 0;


int     main(int argc, char **argv, char **env)
{
    //***setup initial***

    init_graphical();
    //initialize randomness using system time
    srand( (unsigned int)time(NULL) );
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
    joueur_vie   = 1000;
    joueur_score = 0;
	EXIT_MSG="Crashing whithout more precision while game was running";

    //on lance le jeu
    main_loop(10);
    return EXIT_SUCCESS;
}


void    print_monster(monster_t   *monster)
{
    //affiche un monstre
    pict_display(monster->type->sprite, monster->posx, monster->posy);
}
void    move_monster(monster_t *monster, uint new_x, uint new_y)
{
    picture_t monster_sprite = monster->type->sprite;
    //on efface l'ancien monstre en affichant le background a son ancienne position.
    picture_t to_print = pict_crop_size(background_pict, monster->posx, monster_sprite.col, monster->posy, monster_sprite.row);
    pict_display(to_print, monster->posx, monster->posy);
    //puis on réaffiche le nouveau monstre, et on change sa position
    pict_display(monster_sprite, new_x, new_y);
    monster->posx = new_x;
    monster->posy = new_y;
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

/*
 * @brief Creates and initialized the pool of all monsters
 *
 * @param pool_size The maximum amount of monster that can be allocated inside the pool
 */
void    monster_pool_create(uint32_t    pool_size)
{
    // Allocate the memory to store the monsters
    monster_memory    = safe_malloc(sizeof(monster_t) * pool_size);
    monster_pool_head = monster_memory;
    max_monsters      = pool_size;

    // Write empty header
    for (int i = 0; i < pool_size; i++)
    {
        void *ptr = &( (uint8_t *)monster_memory )[i * sizeof(monster_t)];

        monster_pool_empty_header *node = (monster_pool_empty_header *)ptr;
        // Push free node onto thte free list
        node->next        = monster_pool_head;
        monster_pool_head = node;
    }

}


/*
 * @brief Cleans up and frees the memory of the monster pool
 *
 */
void    monster_pool_destroy(void)
{
    if(monster_memory == NULL)
        return;

    free(monster_memory);
    monster_pool_head = NULL;
    monster_memory    = NULL;
    alloced_monsters  = 0;
    max_monsters      = 0;
}

/*
 * @brief Allocates a monster in the monster pool
 *
 * @returns A pointer to a empty monster_t structure which can be used freely
 * @note Returns null pointer if the pool is full or not initialized
 */
monster_t   *monster_pool_alloc(void)
{
    if (
        max_monsters == alloced_monsters
        || monster_memory == NULL
        || monster_pool_head == NULL
        )
        return NULL;
    void *res = monster_pool_head;
    monster_pool_head = monster_pool_head->next;
    alloced_monsters++;
    return res;

}

/*
 * @brief Deallocates a monster slot in the monster pool
 *
 * @param ptr A valid pointer to a slot in the monster pool which is to be freed
 * @note Nothing happens if pool is unitialized or empty or ptr is null.
 *       if ptr is not a valid pointer given by monster_pool_alloc, behavior is unspecified.
 */
void    monster_pool_dealloc(monster_t   *ptr)
{
    if(
        ptr == NULL ||
        monster_memory == NULL ||
        monster_pool_head == NULL ||
        max_monsters == 0 ||
        alloced_monsters == 0
        )
    {
        return;
    }

    alloced_monsters--;
    *( (monster_pool_empty_header *) ptr ) = (monster_pool_empty_header)
    {
        .next = monster_pool_head
    };
    monster_pool_head = (monster_pool_empty_header *)ptr;
}

/*
 * @brief Returns the count of alloced monsters in the pool
 *
 * @return The number of monsters alloced in monster pool, 0 if the pool is empty
 *
 */
uint32_t    monster_pool_count(void)
{
    return alloced_monsters;
}

