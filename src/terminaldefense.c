#include "terminaldefense.h"

/* ************************
 ***VARIABLES GLOBALES***
 ************************/
const char *EXIT_MSG;

int64_t *joueur_vie;
uint64_t joueur_score, turn, joueur_ressources;
tab_size_t arena_size;
coordonee_t cursor_pos; //position du curseur
bool cursor_is_shown;
pixel_t cursor_pixel;
picture_t background;

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
GAME_STATE game_state = GAME_STOPED;
//arbre de séléction des défense actuellement affiché
const defence_choice_tree_t *shown_tree = NULL;
// Index selectionné dans le menu
int32_t sel_index = 0;
const defense_type_t *derniere_construction;

static const pixel_t selection_indicator =
{
    .background_color = COL_BLUE,
    .color            = COL_RED,
    .c1               = '*',
    .c2               = 0,
};
static const coordonee_t NO_COORDINATE =
{
    .x = -1,
    .y = 0,
};

/***********************************
 ***FONCTIONS UTILITAIRES DE BASES***
 ************************************/

//renvoi l'offset associé a un couple coo-stride
int32_t        offset_of(coordonee_t coo, int32_t stride)
{
    return coo.x + coo.y * stride;
}

//renvoie les coordonée du voisin (positions dans l'arène)
coordonee_t    neighbor_of(coordonee_t coo, DIRECTION neighbor)
{
    switch (neighbor)
    {
    case DIR_LEFT:
        //a gauche => posx-=1
        if (coo.x==0)
            return NO_COORDINATE;
        coo.x -= 1;
        return coo;
    case DIR_RIGHT:
        //a droite => posx+=1
        if (coo.x==arena_size.col - 1)
            return NO_COORDINATE;
        coo.x += 1;
        return coo;
    case DIR_UP:
        //en haut => posy-=1
        if (coo.y==0)
            return NO_COORDINATE;
        coo.y -= 1;
        return coo;
    case DIR_DOWN:
        //en bas => posy+=1
        if (coo.y==arena_size.row - 1)
            return NO_COORDINATE;
        coo.y += 1;
        return coo;
    default:
        return NO_COORDINATE;
    }
}

//renvoie la direction opposé
DIRECTION    oposite_direction(DIRECTION    dir)
{
    switch (dir)
    {
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
    //affiche la raison d'éxit
    printf("%s\n", EXIT_MSG);

    monster_pool_destroy();
    free(monster_positions);
    free(defense_array);
    free(pathfinder_array);
}
//affiche le premier monstre a la position demandée
void    print_monster_at(coordonee_t    pos)
{
    print_monster(monster_positions[offset_of(pos, arena_size.stride)], pos);
}

//affiche un monstre (et clear si monster et le pointeur null)
void    print_monster(const monster_t *monster, coordonee_t pos)
{
    //affiche un monstre
    if (monster==NULL)
    {
        const defense_type_t *defense_type = defense_array[offset_of(pos, arena_size.stride)].type;
        if (defense_type==NULL)
        {
            compose_del_pix(COMPOSE_ARENA, pos);
        }
        else
        {
            compose_disp_pix(defense_type->sprite, COMPOSE_ARENA, pos);
        }
    }
    else
    {
        compose_disp_pix(monster->type->sprite, COMPOSE_ARENA, pos);
    }
}

//fais spawn un monstre
void    spawn_monster(const monster_type_t *type, coordonee_t position)
{
    monster_t *monster = monster_pool_alloc();
    *monster = (monster_t)
    {
        .type                 = type,
        .vie                  = type->base_life * (3 + turn / 110),
        .last_action_turn     = turn,
        .next_monster_in_room = monster_positions[offset_of(position, arena_size.stride)],
    };
    monster_positions[offset_of(position, arena_size.stride)] = monster;
    print_monster(monster, position);
}
//finalise la mort d'un monstre
void    kill_monster(monster_t  **monster_ptr)
{
    joueur_ressources += (*monster_ptr)->type->given_ressources;
    joueur_score      += (*monster_ptr)->type->given_ressources / 10;
    monster_t *next_monster = (*monster_ptr)->next_monster_in_room;
    monster_pool_dealloc(*monster_ptr);
    *monster_ptr = next_monster;
}

void    move_monster(monster_t **monster_ptr, coordonee_t objective)
{
    monster_t *monster = *monster_ptr;
    //on retire le montre de son ancienne case
    //en faisant pointer le précédant sur le suivant (on le retire de la liste chainée)
    *monster_ptr = monster->next_monster_in_room;
    //on ajoute le monstre au début de la liste de la case suivante
    //et on fait pointer le monstre sur les autres de la case (on l'insère dans la liste chainée)
    monster->next_monster_in_room                              = monster_positions[offset_of(objective, arena_size.stride)];
    monster_positions[offset_of(objective, arena_size.stride)] = monster;
    //on update l'affichage (celui derière nous sera déja clean
    print_monster(monster, objective);
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
void    display_range_overlay(void)
{
    //affiche la range des tourelles
    compose_disp_pict(
        background,
        COMPOSE_BACK,
        (coordonee_t){ 0, 0 }
        );
    const defense_type_t *type = defense_array[offset_of(cursor_pos, arena_size.stride)].type;
    if (type!=NULL)
    {
        int range = type->range;
        if (range>0)
        {
            pixel_t pixel = (pixel_t)
            {
                .c1               = ' ', .c2 = '\0',
                .color            = COL_DEFAULT,
                .background_color = COL_GRAY,
            };
            coordonee_t position;
            position.y = cursor_pos.y - range;
            //ligne supérieure
            if (position.y>=0)
            {
                for (position.x = cursor_pos.x - range ; position.x<=cursor_pos.x + range; position.x += 1)
                {
                    if (0<=position.x && position.x<arena_size.col)
                    {
                        compose_disp_pix(pixel, COMPOSE_BACK, position);
                    }
                }
            }
            //ligne inférieure
            position.y = cursor_pos.y + range;
            if (position.y<arena_size.row)
            {
                for (position.x = cursor_pos.x - range ; position.x<=cursor_pos.x + range; position.x += 1)
                {
                    if (0<=position.x && position.x<arena_size.col)
                    {
                        compose_disp_pix(pixel, COMPOSE_BACK, position);
                    }
                }
            }
            //colonne droite
            position.x = cursor_pos.x - range;
            if (position.x>=0)
            {
                for (position.y = cursor_pos.y - range ; position.y<=cursor_pos.y + range; position.y += 1)
                {
                    if (0<=position.y && position.y<arena_size.row)
                    {
                        compose_disp_pix(pixel, COMPOSE_BACK, position);
                    }
                }
            }
            //colonne gauche
            position.x = cursor_pos.x + range;
            if (position.x<arena_size.col)
            {
                for (position.y = cursor_pos.y - range ; position.y<=cursor_pos.y + range; position.y += 1)
                {
                    if (0<=position.y && position.y<arena_size.row)
                    {
                        compose_disp_pix(pixel, COMPOSE_BACK, position);
                    }
                }
            }
        }
    }
}
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

void    move_cursor(DIRECTION dir, bool fast)
{
    hide_cursor();
    coordonee_t new_pos = neighbor_of(cursor_pos, dir);
    if(fast)
    {
        // Si on veut bouger vite, on bouge 5 fois vers dir
        new_pos = neighbor_of(new_pos, dir);
        new_pos = neighbor_of(new_pos, dir);
        new_pos = neighbor_of(new_pos, dir);
        new_pos = neighbor_of(new_pos, dir);
    }
    //on ne peut intentionelement pas séléctionner la première ligne, pour que les mobs puisse spawn
    if (new_pos.x>0)
    {
        cursor_pos = new_pos;
    }
    show_cursor();
    display_range_overlay();
}
/*** PATHFINDER ***/
// (re) initilaise le pathfinder array
void    path_reinit(void)
{
    for (int i = 0; i<arena_size.col * arena_size.row; i++)
    {
        pathfinder_array[i] = (pathfinder_data)
        {
            .next     = DIR_NOWHERE,
            .distance = UINT64_MAX,
        };
    }
}
//update le pathfinder a partir de la position demandée
void    update_pathfinder_from(coordonee_t    position)
{
    //indice déja traité
    uint borne_inf = 0;
    //indice jusqu' auquel la liste est remplie
    uint borne_sup = 0;

    //la ou nous a demander d'update
    position_list[0]                                              = position;
    pathfinder_array[offset_of(position, arena_size.stride)].next = DIR_NOWHERE;

    //utilisé dans la boucle
    pathfinder_data here_before;
    pathfinder_data here_after;

    //tant qu'il reste des case a traiter
    while ( borne_inf!=borne_sup + 1 && !( borne_inf == pos_list_size && borne_sup==0 ) )
    {
        //position de la case a traiter
        position = position_list[borne_inf];
        //indice dans les tableau
        here_before = pathfinder_array[offset_of(position, arena_size.stride)];

        if (here_before.next!=DIR_NOWHERE)
        {
            //cette position a déja été update (présente plusieurs fois dans la liste), on skip
            borne_inf++;
            if (borne_inf==pos_list_size)
                borne_inf = 0;
            continue;
        }//else ....

        //position de la base (c'est ici que les mobs doivent arriver: distance de 0)
        if (position.x==base_coordinate.x && position.y==base_coordinate.y)
        {
            here_after = (pathfinder_data)
            {
                .distance = 0,
                .next     = DIR_RIGHT, //on sort de l'écran, mais cela n'arrive que si la base est détruite (le jeu s'arrête de toute facon)
            };
        }
        else
        {
            here_after = (pathfinder_data)
            {
                .distance = INT64_MAX,
                .next     = DIR_NOWHERE,
            };
            for (DIRECTION direction = 0; direction<DIR_NOWHERE; direction++)
            {
                //pour chaque direction cardinale
                coordonee_t neighbor = neighbor_of(position, direction);
                if (neighbor.x!=-1) //si le voisin existe
                {
                    int32_t neighbor_offset = offset_of(neighbor, arena_size.stride);
                    //si le voisin ne pointe pas sur nous et n'est pas undef/en cours de recalcul (DIR_NOWHERE)
                    if (
                        pathfinder_array[neighbor_offset].next != oposite_direction(direction)
                        && pathfinder_array[neighbor_offset].next != DIR_NOWHERE
                        )
                    {
                        //si il est intéréssant
                        if (pathfinder_array[neighbor_offset].distance < here_after.distance)
                        {
                            //on passe par lui
                            here_after = (pathfinder_data)
                            {
                                .distance = pathfinder_array[neighbor_offset].distance,
                                .next     = direction
                            };
                        }
                    }
                }
            }//fin du for

            if (defense_array[offset_of(position, arena_size.stride)].type != NULL)
            {
                //on est sur une défense, on augmente la "taille" du chemin selon la vie
                here_after.distance += max(defense_array[offset_of(position, arena_size.stride)].life / 10 - 6, 1);
            }
            else
            {
                here_after.distance += 10;
            }
        }//fin du if(pos_base) else

        pathfinder_array[offset_of(position, arena_size.stride)] = here_after;
        //si on s'est recalculé avec succès
        if ( here_after.next != DIR_NOWHERE )
        {
            for (DIRECTION direction = 0; direction<DIR_NOWHERE; direction++)
            {
                //on update jamais le voisin sur lequel on pointe
                if (direction==here_after.next)
                {
                    continue;
                }                                           //fait sauter ce tour de boucle
                coordonee_t neighbor = neighbor_of(position, direction);
                if (neighbor.x!=-1)     //si le voisin existe
                /* si
                 * - nôtre distance s'est améliorée
                 * - nôtre distance a changé et que ce voisins pointait sur nous
                 * - ce voisin était undef
                 * On l'update
                 */
                {
                    int32_t offset_voisin  = offset_of(neighbor, arena_size.stride);
                    pathfinder_data voisin = pathfinder_array[offset_voisin];
                    if (
                        ( here_after.distance<here_before.distance      ) // - amélioration de la distance
                        ||( voisin.next==DIR_NOWHERE                    ) // - voisin non défini
                        ||( here_after.next!=here_before.next             // - changement +
                            &&voisin.next==oposite_direction(direction) )
                        )                                                 //        voisin pointe sur nous

                    //on marque le voisin comme a update
                    //(ajout dans la liste plus pointe nul-part)
                    {
                        borne_sup += 1;
                        if (borne_sup==pos_list_size)
                            borne_sup = 0;
                        position_list[borne_sup]             = neighbor;
                        pathfinder_array[offset_voisin].next = DIR_NOWHERE;
                    }
                }
            }//fin du for
        } //if recalculé avec succès
        else
        {
            //si on ne s'est pas recalculé avec succès, on doit se recalculer
            borne_sup += 1;
            if (borne_sup==pos_list_size)
                borne_sup = 0;
            position_list[borne_sup] = position;
        }
        borne_inf++;
        if (borne_inf==pos_list_size)
            borne_inf = 0;
    }// fin du while
}

// Affiche le menu du jeu et renvoie la difficulté sélectionnée
uint32_t    game_menu()
{
    uint32_t half_width  = termsize.col / 2;
    uint32_t half_height = termsize.row / 2;

    char control_text[300];
    sprintf(
        control_text,
        "          %c%c%c%c - To move cursor\n"
        "  Shift + %c%c%c%c - To move cursor fast\n"
        "             %c - Construction menu\n"
        "            %c%c - Menu navigation\n"
        "           %c/%c - Menu OK\n"
        "             %c - Menu exit\n"
        "     Shift + %c - Builds last used\n"
        "      Ctrl + c - Exit\n",
        KEY_UP,
        KEY_LEFT,
        KEY_DOWN,
        KEY_RIGHT,
        KEY_UP,
        KEY_LEFT,
        KEY_DOWN,
        KEY_RIGHT,
        KEY_BUILD,
        KEY_UP,
        KEY_DOWN,
        KEY_BUILD,
        KEY_RIGHT,
        KEY_LEFT,
        KEY_LEFT
        );

    compose_disp_text(
        control_text,
        COL_CYAN,
        COL_DEFAULT,
        COMPOSE_UI,
        (coordonee_t){ .x = half_width - 19, .y = half_height + 3 },
        (coordonee_t){ .x = 40, .y = 8 }
        );

    compose_disp_rect(
        COL_GRAY_DARK,
        COMPOSE_BACK,
        (coordonee_t){ 0, 0 },
        (coordonee_t){ termsize.col, termsize.row }
        );

    const char *title = "$ TERMINAL DEFENSE";
    char play_text[100];
    sprintf( play_text, "PRESS %c TO PLAY", toupper(KEY_RIGHT) );


    compose_disp_text(
        title,
        COL_GREEN,
        COL_DEFAULT,
        COMPOSE_UI,
        (coordonee_t){ half_width - strlen(title) / 2, half_height - 6 },
        (coordonee_t){ .x = strlen(title), .y = 1 }
        );

    compose_disp_text(
        play_text,
        COL_YELLOW,
        COL_DEFAULT,
        COMPOSE_UI,
        (coordonee_t){ half_width - strlen(play_text) / 2, half_height },
        (coordonee_t){ .x = strlen(play_text), .y = 1 }
        );

    char input;
    bool show_menu                    = true;
    int32_t selected_difficulty_index = 0;

    while(show_menu)
    {
        while ( read(STDIN_FILENO, &input, 1) )    // read se comporte comme scanf("%c",&input), a l'éxeption de ne pas etre bugée
        {
            if(input == KEY_DOWN)
            {
                selected_difficulty_index = max(selected_difficulty_index - 1, 0);
            }
            if(input == KEY_UP)
            {
                selected_difficulty_index = min(selected_difficulty_index + 1, DIFFICULTY_LEVEL_COUNT - 1);
            }
            if(input == KEY_LEFT || input == '\x3')
            {
                exit(130);
            }
            if(input == KEY_RIGHT)
            {
                compose_del_area(
                    COMPOSE_UI,
                    (coordonee_t){ 0 },
                    (coordonee_t){ .x = termsize.col, .y = termsize.row }
                    );
                // Quitte le menu
                return difficulty_levels[selected_difficulty_index].difficulty_value;
            }
        }
        char text[100];
        sprintf(text, "Chose difficulty ( with %c and %c ) : ", KEY_UP, KEY_DOWN);

        uint32_t txt_center = half_width - ( strlen(text) + 8 ) / 2;

        compose_del_area(
            COMPOSE_UI,
            (coordonee_t){ .x = 0, .y = half_height - 3 },
            (coordonee_t){ .x = termsize.col, .y = half_height - 3 }
            );

        compose_disp_text(
            text,
            COL_TEXT,
            COL_DEFAULT,
            COMPOSE_UI,
            (coordonee_t){ txt_center, half_height - 3 },
            (coordonee_t){ .x = strlen(text), .y = 1 }
            );

        compose_disp_text(
            difficulty_levels[selected_difficulty_index].difficulty_name,
            difficulty_levels[selected_difficulty_index].display_color,
            COL_DEFAULT,
            COMPOSE_UI,
            (coordonee_t){ txt_center + strlen(text), half_height - 3 },
            (coordonee_t){ .x = strlen(difficulty_levels[selected_difficulty_index].difficulty_name), .y = 1 }
            );

        compose_refresh();
    }
}

/******************
 ***MOTEUR DE JEU***
 *******************/
void    sig_handler(int    _)
{
    //Pour éviter les warning d'arguments inutilisés
    (void)_;
    // Pour éviter un boucle de crash
    signal(SIGSEGV, SIG_DFL);
    signal(SIGILL, SIG_DFL);
    signal(SIGTERM, SIG_DFL);
    exit(1);
}

int    main()
{
    //si jamais ...
    EXIT_MSG = "Crash while initializing ...";

    //***setup initial***

    init_graphical();

    //restaure l'état du terminal en cas de crash
    // La fonction signal permet d'executer du code lors d'un "signal"
    // C'est une fonction du système d'exploitation qui permet d'alerter un programe
    // Lorsque le programme crashe, on veut retablir l'état normal du terminal
    signal(SIGSEGV, sig_handler);
    signal(SIGTERM, sig_handler);

    //renseigne la fonction a éxécuter a la sortie du programme
    atexit(cleanup);

    //initialize randomness using system time
    srand( (unsigned int)time(NULL) );

    // MENU
    uint32_t game_difficulty = game_menu();

    //*initialise les variables globales*
    //creation du background
    //taille de l'arène
    arena_size.col    = termsize.col - reserved;
    arena_size.stride = arena_size.col;
    arena_size.row    = termsize.row;

    //initialisation du background avec son patterne
    background.size = arena_size;
    background.data = safe_malloc(sizeof(pixel_t) * background.size.row * background.size.col);
    pixel_t pixel = (pixel_t)
    {
        .c1    = ' ',            //le caractères étant un simple ascii (un espace),
        .c2    = '\0',           //il ne prend que c1, les autres sont donc nulls
        .color = COL_DEFAULT,
    };
    for (int i = 0; i<background.size.col; i++)
    {
        for (int j = 0; j<background.size.row; j++)
        {
            if ( ( (i % 5)==2 ) || ( (j % 5)==2 ) ) //selectionne des lignes verticales et horizontales éspacé de 5 cases
            {
                pixel.background_color = COL_BOARD_BACKGROUND_1;
            }
            else
            {
                pixel.background_color = COL_BOARD_BACKGROUND_2;
            }
            background.data[i + j * background.size.stride] = pixel;
        }
    }

    compose_disp_pict(
        background,
        COMPOSE_BACK,
        (coordonee_t){ 0, 0 }
        );
    //initialisation de la colonne de droite
    pixel.background_color = COL_WHITE;
    for (int j = 0; j<termsize.row; j++)
    {
        compose_disp_pix(
            pixel,
            COMPOSE_BACK,
            (coordonee_t){ arena_size.col, j }
            );
    }
    pixel.background_color = COL_BLACK;
    for (int i = arena_size.col + 1; i<termsize.col; i++)
    {
        for (int j = 0; j<termsize.row; j++)
        {
            compose_disp_pix(
                pixel,
                COMPOSE_BACK,
                (coordonee_t){ i, j }
                );
        }
    }


    monster_pool_create(200);
    //creation (et initialisation a zero) de monster_position
    monster_positions = safe_malloc(sizeof(monster_t *) * arena_size.row * arena_size.col);
    memset(monster_positions, (long int)NULL, sizeof(monster_t *) * arena_size.row * arena_size.col);
    //autre variables globales
    defense_array    = safe_malloc( arena_size.col * arena_size.row * sizeof(defense_t) );
    pathfinder_array = safe_malloc( arena_size.col * arena_size.row * sizeof(pathfinder_data) );
    position_list    = safe_malloc( arena_size.col * arena_size.row * 10 * sizeof(coordonee_t) );
    pos_list_size    = arena_size.col * arena_size.row * 10;
    base_coordinate  = (coordonee_t)
    {
        .x = arena_size.col - 1,
        .y = arena_size.row / 2,
    };

    joueur_ressources     = 3000;
    joueur_score          = 0;
    derniere_construction = NULL;

    cursor_pos = base_coordinate;
    path_reinit();
    build_defense(&la_base);
    //vie du joueur == vie de la base
    joueur_vie = &(defense_array[offset_of(base_coordinate, arena_size.stride)].life);

    cursor_pixel = (pixel_t)
    {
        .c1               = ' ',
        .c2               = '\0',
        .color            = COL_DEFAULT,
        .background_color = COL_CURSOR,
    };

    EXIT_MSG = "Crashing whithout more precision while game was running";

    //on lance le jeu
    game_state = GAME_PLAYING;
    turn       = 1;
    main_loop(game_difficulty);
    EXIT_MSG = "You died!";

    return EXIT_SUCCESS;
}

//obtient et traite les inputs claviers
void    treat_input(void)
{
    char input;
    while ( read(STDIN_FILENO, &input, 1) )    // read se comporte comme scanf("%c",&input), a l'éxeption de ne pas etre bugée
    {
        bool is_upper = false;
        if ( isupper(input) )
        {
            is_upper = true;
            input   += 32; //on le met en minuscule (voire table ascii)
        }
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

        // HAUT
        case KEY_UP:
            if (game_state==GAME_PLAYING)
                move_cursor(DIR_UP, is_upper);
            else if (game_state==GAME_SELECT_DEF)
                augment_selection();
            break;

        // BAS
        case KEY_DOWN:
            if (game_state==GAME_PLAYING)
                move_cursor(DIR_DOWN, is_upper);
            else if (game_state==GAME_SELECT_DEF)
                diminish_selection();
            break;

        // GAUCHE
        case KEY_LEFT:
            if (game_state==GAME_PLAYING)
                move_cursor(DIR_LEFT, is_upper);
            else if (game_state==GAME_SELECT_DEF)
            {
                //on abandonne la séléction
                hide_selection();
                game_state = GAME_PLAYING;
            }
            ;
            break;

        // DROITE
        case KEY_RIGHT:
            if (game_state==GAME_PLAYING)
                move_cursor(DIR_RIGHT, is_upper);
            if(game_state==GAME_SELECT_DEF)
            {
                select_defense();
            }
            break;
        case KEY_BUILD:
            if (game_state!=GAME_PAUSED)
            {
                if (game_state==GAME_PLAYING && is_upper)
                {
                    fast_build();
                }
                else
                {
                    select_defense();
                }
            }
            break;
        case KEY_PAUSE:
            toogle_pause();
        }
    }
}
void    fast_build(void)
{
    if (derniere_construction!=NULL && defense_array[offset_of(cursor_pos, arena_size.stride)].type==NULL)
    {
        build_defense(derniere_construction);
    }
}
//construit une defense a la position du curseur
void    build_defense(const defense_type_t   *defense_type)
{
    derniere_construction = defense_type;
    if (joueur_ressources<defense_type->cost)
    {
        //not enough resources
        return;
    }
    joueur_ressources                                      -= defense_type->cost;
    defense_array[offset_of(cursor_pos, arena_size.stride)] = (defense_t)
    {
        .type = defense_type,
        .life = defense_type->max_life,
    };
    compose_disp_pix(defense_type->sprite, COMPOSE_ARENA, cursor_pos);
    update_pathfinder_from(cursor_pos);
    display_range_overlay();
}

void    select_defense(void)
{
    if (game_state==GAME_PLAYING)
    {
        if (defense_array[offset_of(cursor_pos, arena_size.stride)].type!=NULL)
        {
            //une defense éxiste déja
            return;
        }

        game_state = GAME_SELECT_DEF;
        shown_tree = &main_selection_tree;
        sel_index  = 0;
        display_selection();
        return;
    }
    if (game_state==GAME_SELECT_DEF)
    {
        hide_selection();
        //sel_index devrait etre une valeure légale car on a bien codé le reste
        if (sel_index<shown_tree->sub_category_count)
        {
            shown_tree = shown_tree->sub_categories[sel_index];
            sel_index  = 0;
            display_selection();
            return;
        } //else
        sel_index -= shown_tree->sub_category_count;
        build_defense(shown_tree->defenses[sel_index]);
        game_state = GAME_PLAYING;
    }
}

// Affiche un item de choix
void    display_defense_selection_item(pixel_t icon, const char *text, bool is_category, uint32_t indice)
{
    int32_t posx = termsize.col - reserved + 1; //a gauche de la barre de droite
    int32_t posy = termsize.row - indice * 3 - 3; //en bas, par pas de 3 (taille d'un icone)
    compose_disp_pict(
        frame,
        COMPOSE_UI,
        (coordonee_t){ posx, posy }
        );
    compose_disp_pix(
        icon,
        COMPOSE_UI,
        (coordonee_t){ posx + 1, posy + 1 }
        );
    compose_disp_text(
        (char *)text,
        is_category ? COL_GREEN : COL_BLUE,
        COL_DEFAULT,
        COMPOSE_UI,
        (coordonee_t){ posx + 3, posy },
        (coordonee_t){ reserved - 6, 3 }
        );
}

// Affiche le menu de selection de defense
void    display_selection(void)
{
    uint32_t indice = 0;
    for(int i = 0; i < shown_tree->sub_category_count; i++)
    {
        display_defense_selection_item(shown_tree->sub_categories[i]->icon, shown_tree->sub_categories[i]->short_txt, true, indice);
        indice += 1;
    }

    for(int i = 0; i < shown_tree->defense_count; i++)
    {
        display_defense_selection_item(shown_tree->defenses[i]->sprite, shown_tree->defenses[i]->short_txt, false, indice);
        indice += 1;
    }


    //affichage de la selection de l'élément en bas
    compose_disp_pix(
        selection_indicator,
        COMPOSE_UI,
        (coordonee_t){ termsize.col - reserved, termsize.row - 2 }
        );

}
void    hide_selection(void)
{
    //on clean l'entièreté de la colone de droite (ou il n'y a normalement que ca dans le niveau UI)
    compose_del_area(
        COMPOSE_UI,
        (coordonee_t){ termsize.col - reserved, 0 },
        (coordonee_t){ termsize.col - 1, termsize.row - 1 }
        );
}


void    augment_selection(void)
{
    //on cache l'ancienne selection
    compose_del_pix(
        COMPOSE_UI,
        (coordonee_t){ termsize.col - reserved, termsize.row - 2 - 3 * sel_index }
        );
    sel_index++;
    //si on dépasse le maximum, on retourne a 0
    if (sel_index >= shown_tree->defense_count + shown_tree->sub_category_count)
    {
        sel_index = 0;
    }
    //on affiche la nouvelle selection
    compose_disp_pix(
        selection_indicator,
        COMPOSE_UI,
        (coordonee_t){ termsize.col - reserved, termsize.row - 2 - 3 * sel_index }
        );
}
void    diminish_selection(void)
{
    //on cache l'ancienne selection
    compose_del_pix(
        COMPOSE_UI,
        (coordonee_t){ termsize.col - reserved, termsize.row - 2 - 3 * sel_index }
        );
    sel_index--;
    //si passe en dessous de 0, on retourne au maximum
    if (sel_index < 0)
    {
        sel_index = shown_tree->defense_count + shown_tree->sub_category_count - 1;
    }
    //on affiche la nouvelle selection
    compose_disp_pix(
        selection_indicator,
        COMPOSE_UI,
        (coordonee_t){ termsize.col - reserved, termsize.row - 2 - 3 * sel_index }
        );
}

//main game loop
//run until quit or die
void    main_loop(uint    difficulty)
{
    while (*joueur_vie>0)
    {
        wait_for_next_frame();
        if (game_state==GAME_PAUSED)
        {
            treat_input();
        }
        else
        {
            if (turn % 2)
                blink_cursor();
            treat_input();
            randomly_spawn_mobs(difficulty);
            defenses_routine();
            monsters_routine();
            right_column_refresh();
            compose_refresh();
            turn += 1;
        }
    }
    return;
}

void    single_monster_routine(monster_t **monster_ptr, coordonee_t position)
{
    monster_t *monster = *monster_ptr;

    //on ne devrait pas passer ici, mais des fois que
    if (monster->vie<=0)
    {
        //on tue le monstre
        kill_monster(monster_ptr);
        return;
    }
    //temps depuis lequel le mob n'a rien fait
    int64_t idle_time = turn - monster->last_action_turn;
    if (idle_time<=0)
    {
        //le monstre vient de faire quelque chose (on repasse dessus si il vient de bouger). Il ne fait donc rien
        return;
    }
    //on regarde vers ou le pathfinder nous indique d'aller
    DIRECTION mob_objective_dir = pathfinder_array[offset_of(position, arena_size.stride)].next;
    coordonee_t mob_objective   = neighbor_of(position, mob_objective_dir);
    if (mob_objective.x==-1)
    {
        //le pathfinder et buggé
        EXIT_MSG = "Pathfinder bug detected while trying to move, exiting";
        exit(1);
    }
    //si le chemin est bloqué par une défense
    defense_t *defence = &defense_array[offset_of(mob_objective, arena_size.stride)];
    if (defence->type!=NULL)
    {
        //on lui tappe dessus
        damage_defense(mob_objective, monster->type->damage);
        //et on a fait une action
        monster->last_action_turn = turn;
    }
    else
    {
        //le chemin n'est pas bloqué, on avance (selon la vitesse du mob)
        if (idle_time>=monster->type->speed)
        {
            move_monster(monster_ptr, mob_objective);
            monster->last_action_turn = turn;
        }
    }

}
void    monsters_routine(void)
{
    coordonee_t position;
    for (position.y = 0; position.y<arena_size.row ; position.y++)
    {
        for (position.x = 0; position.x<arena_size.col; position.x++)
        {
            //pour chaque cases, on parcour la liste chainée des monstre a cette case, et appèlle la routine sur eux
            monster_t **monster_ptr      = &monster_positions[offset_of(position, arena_size.stride)];
            monster_t **monster_ptr_next = NULL;
            while (*monster_ptr!=NULL)
            {
                //on doit obtenir le monstre suivant des maintenant car le monstre actuel risque de bouger (auquel cas le pointeur actuel ne sera plus valide)
                monster_ptr_next = &( (*monster_ptr)->next_monster_in_room );
                single_monster_routine(monster_ptr, position);
                monster_ptr = monster_ptr_next;
            }
            //puis on update les graphismes
            print_monster_at(position);
        }
    }
}
void    randomly_spawn_mobs(int    difficulty)
{
    //TODO: improve, this is realy crude
    if (rand() % 10 == 0)
    {
        int borne_sup = rand() % turn;
        for (int i = 1; i * i * i * i<borne_sup; i += 15 / difficulty + 1)
        {
            coordonee_t spawn_location =
            {
                .x = 0,
                .y = rand() % arena_size.row,
            };
            const monster_type_t *type;
            //avoid spawning runner in to early game
            if (turn<100)
            {
                type = &armored;
            }
            else
            {
                //sinon, une chance sur 2
                if (rand() % 2)
                {
                    type = &runner;
                }
                else
                {
                    type = &armored;
                }
            }
            spawn_monster(type, spawn_location);
        }
    }
}

void    damage_defense(coordonee_t target_position, uint32_t damage)
{
    defense_t *target = &defense_array[offset_of(target_position, arena_size.stride)];

    //si la défence passe un pas de 100 de vie, le pathfinder doit etre update, on stocke donc la vie précédente
    int32_t previous_life = target->life;
    int32_t new_life      = previous_life - damage;

    //si la défence est détruite, on retire son type (intérprété comme une abscence de défense)
    if (new_life<0)
    {
        target->type = NULL;
        target->life = 0;
    }
    else
        target->life = new_life;

    if (previous_life / 100!=new_life / 100)
    {
        //la vie a suffisament changée, on update le patfinder
        update_pathfinder_from(target_position);
    }
}
void    defenses_routine(void)
{
    coordonee_t position;
    for (position.x = 0; position.x<arena_size.col; position.x++)
    {
        for (position.y = 0; position.y<arena_size.row; position.y++)
        {
            if (defense_array[offset_of(position, arena_size.stride)].type!=NULL)
            {
                single_defense_routine(position);
            }
        }
    }
}
void    single_defense_routine(coordonee_t    defense_position)
{
    //update des graphismes
    defense_t defense = defense_array[offset_of(defense_position, arena_size.stride)];
    if (defense.type->damage>0)
    {
        //cherche des monstres a proximité
        int32_t range = defense.type->range;
        int32_t minx  = max(0, defense_position.x - range);
        int32_t maxx  = min(arena_size.col, defense_position.x + range + 1);
        int32_t miny  = max(0, defense_position.y - range);
        int32_t maxy  = min(arena_size.row, defense_position.y + range + 1);
        //tape le monstre le plus a droite dans la range
        for (int32_t x = maxx - 1; x>=minx; x--)
        {
            for (int32_t y = miny; y<maxy; y++)
            {
                monster_t **monster_ptr = &monster_positions[x + y * arena_size.stride];
                if ( (*monster_ptr)!=NULL )
                {
                    damage_monster(monster_ptr, defense.type->damage);
                    //on a fait des dégats; on s'arrète
                    return;
                }
            }
        }
    }
}
void    damage_monster(monster_t **monster_ptr, int32_t damage)
{
    (*monster_ptr)->vie -= damage;
    if ( (*monster_ptr)->vie<=0 )
    {
        kill_monster(monster_ptr);
    }
}

void    right_column_refresh(void)
{
    coordonee_t box_size =
    {
        .x = reserved - 1, .y = 1
    };
    coordonee_t position =
    {
        .x = termsize.col - reserved + 1, .y = 0
    };
    compose_del_area(
        COMPOSE_ARENA,
        position,
        (coordonee_t){ .x = termsize.col - 1, .y = termsize.row - 1 }
        );
    char text[50];
    //PRId64 est une macro pour print les int64_t (ld ou lld selon les systèmes)
    //PRIu64 our les unsigned
    sprintf(text, "%" PRId64, *joueur_vie);
    compose_disp_text("vie de la base:", COL_RED, COL_DEFAULT, COMPOSE_ARENA, position, box_size);
    position.y += 1;
    compose_disp_text(text, COL_RED, COL_DEFAULT, COMPOSE_ARENA, position, box_size);

    position.y += 2;
    sprintf(text, "%" PRIu64, joueur_ressources);
    compose_disp_text("ressources:", COL_TEXT, COL_DEFAULT, COMPOSE_ARENA, position, box_size);
    position.y += 1;
    compose_disp_text(text, COL_RED, COL_DEFAULT, COMPOSE_ARENA, position, box_size);

    position.y += 2;
    sprintf(text, "%" PRIu64, joueur_score);
    compose_disp_text("score:", COL_TEXT, COL_DEFAULT, COMPOSE_ARENA, position, box_size);
    position.y += 1;
    compose_disp_text(text, COL_RED, COL_DEFAULT, COMPOSE_ARENA, position, box_size);

    position.y += 3;
    box_size.y  = 3;
    //si on est en train de construire une défense, on affiche son texte
    if (game_state==GAME_SELECT_DEF)
    {
        if (sel_index<shown_tree->sub_category_count)
        {
            compose_disp_text(
                shown_tree->sub_categories[sel_index]->desc_txt,
                COL_TEXT,
                COL_DEFAULT,
                COMPOSE_ARENA,
                position,
                box_size
                );
        }
        else
        {
            defense_type_t selected_def_type = *shown_tree->defenses[sel_index - shown_tree->sub_category_count];
            sprintf(text, "coût: %u", selected_def_type.cost);
            compose_disp_text(text, COL_TEXT, COL_DEFAULT, COMPOSE_ARENA, position, box_size);
            position.y += 1;
            sprintf(text, "portée: %u", selected_def_type.range);
            compose_disp_text(text, COL_TEXT, COL_DEFAULT, COMPOSE_ARENA, position, box_size);
            position.y += 1;
            sprintf(text, "dégats: %u", selected_def_type.damage);
            compose_disp_text(text, COL_TEXT, COL_DEFAULT, COMPOSE_ARENA, position, box_size);
            position.y += 1;
            sprintf(text, "vie: %u", selected_def_type.max_life);
            compose_disp_text(text, COL_TEXT, COL_DEFAULT, COMPOSE_ARENA, position, box_size);
            position.y += 1;
            compose_disp_text(
                selected_def_type.desc_txt,
                COL_TEXT,
                COL_DEFAULT,
                COMPOSE_ARENA,
                position,
                box_size
                );
        }
    }
    else
    {
        //sinon, on affiche les détails de la défense sous le curseur
        defense_t selected_defense = defense_array[offset_of(cursor_pos, arena_size.stride)];
        if (selected_defense.type!=NULL)
        {
            sprintf(text, "vie: %" PRIu64, selected_defense.life);
            compose_disp_text(text, COL_TEXT, COL_DEFAULT, COMPOSE_ARENA, position, box_size);
            position.y += 1;
            compose_disp_text(
                selected_defense.type->desc_txt,
                COL_TEXT,
                COL_DEFAULT,
                COMPOSE_ARENA,
                position,
                box_size
                );
        }
        else
        {
            compose_disp_text("no defense under cursor", COL_TEXT, COL_DEFAULT, COMPOSE_ARENA, position, box_size);
        }
    }
}
void    toogle_pause(void)
{
    coordonee_t pos_of_text = (coordonee_t)
    {
        (termsize.col - 13) / 2, (termsize.row - 3) / 2
    };
    if (game_state==GAME_PAUSED)
    {
        game_state = GAME_PLAYING;
        //la ou on avait affiché jeu en pause
        compose_del_area(
            COMPOSE_UI,
            pos_of_text,
            (coordonee_t){ pos_of_text.x + 13, pos_of_text.y + 3 }
            );
        compose_refresh();
    }
    else
    {
        hide_selection();
        game_state = GAME_PAUSED;
        compose_disp_text(
            "*************"
            "*GAME PAUSED*"
            "*************",
            COL_MAGENTA,
            COL_GRAY_DARK,
            COMPOSE_UI,
            //au centre de l'ecran
            pos_of_text,
            (coordonee_t){ 13, 3 }
            );
    }
}

/*****************************
 *** MONSTER POOL UTILITIES ***
 ******************************/


// Augmente la taille de la mémoire de monstre
void    monster_pool_expand(uint32_t    expand_size)
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
void    monster_pool_create(uint32_t    pool_size)
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

