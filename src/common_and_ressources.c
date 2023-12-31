//fonctions utilitaires
#include "common.h"

//ressources
#include "terminaldefense.h"

/****************************
 *** FONCTIONS UTILITAIRES ***
 *****************************/
// définitions dans common.h
#if SYSTEM_POSIX
struct timespec last_frame_time =
{
    0, 0
};
#elif SYSTEM_WINDOWS
clock_t last_frame_time = 0;
#endif

void   *safe_malloc(size_t    size)
{
    //obtient de la place mémoire et vérifie qu'elle a bien été allouée
    void *ptr;
    ptr = malloc(size);
    if (ptr!=NULL)
    { //si la place est allouée, on renvoie le pointeur
        return ptr;
    } //sinon, on ferme le programme (cleanup va free ce qui a déjà été alloué)
    EXIT_MSG = "malloc a fail ! sortie de programme !";
    exit(254);
}

void   *safe_realloc(void *ptr, size_t new_size)
{
    //obtient de la place mémoire et vérifie qu'elle a bien été allouée
    void *new_ptr;
    new_ptr = realloc(ptr, new_size);
    if (new_ptr!=NULL)
    { //si la place est allouée, on renvoie le pointeur
        return new_ptr;
    } //sinon, on ferme le programme (cleanup va free ce qui a déjà été alloué)
    EXIT_MSG = "realloc a fail ! sortie du programme !";
	exit(254);
}
int32_t    min(int32_t a, int32_t b)
{
    if (a<b)
        return a;
    else
        return b;
}
int32_t    max(int32_t a, int32_t b)
{
    if (a>b)
        return a;
    else
        return b;
}
//renvoie v, borné entre min_v et max_v
int32_t    clamp(int32_t v, int32_t min_v, int32_t max_v)
{
    return min( max_v, max(v, min_v) );
}

//attend ms milliseconde
int        td_wait(long    ms)
{
    // Si le système cible est linux
    #if SYSTEM_POSIX
    //nanosleep accept un struct en seconds et nanosecondes
    //on convertit donc l'entrée
    struct timespec ts;
    ts.tv_sec  = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;
    return nanosleep(&ts, &ts);
    #else // Si le système cible est Windows
    return Sleep(ms);
    #endif
}

//attend la prochaine frame
void    wait_for_next_frame(void)
{
#if SYSTEM_POSIX
	//on regarde combien de temps s'est écoulé depuis la dernière frame
    struct timespec actual_time;
    clock_gettime(CLOCK_MONOTONIC, &actual_time);
    long int diff_in_ms = (actual_time.tv_nsec - last_frame_time.tv_nsec) / 1000000 + //nanoseconde component
                          (actual_time.tv_sec - last_frame_time.tv_sec) * 1000; //second component
	
	//si le temps est aberrant (ou qu'on lague)
    if (diff_in_ms<0 || diff_in_ms>FRAME_TIME)
    {
        //on n'attend pas
        last_frame_time = actual_time;
    }
    else
    {
		//sinon, on update le temps de last_frame_time a la fin de temps de la frame actuelle
        last_frame_time.tv_nsec += FRAME_TIME * 1000000;  //on ajoute FRAME_TIME millisecondes (convertit en nanosecondes)
        if (last_frame_time.tv_nsec>=1000000000)
        {
            last_frame_time.tv_nsec -= 1000000000;
            last_frame_time.tv_sec  += 1;
        }
		//puis on attend la fin de la frame
        td_wait(FRAME_TIME - diff_in_ms);
    }
#elif SYSTEM_WINDOWS
	//pareil, mais avec des appels système différent
	clock_t now=clock();
	long int diff_in_ms=(now-last_frame_time) / CLOCKS_PER_MSEC;
	if (diff_in_ms<0 || diff_in_ms>FRAME_TIME)
    {
        //heavily lagging,
        //clock uninitialized or looped around (32-bits maybe)
        last_frame_time = now;
    }
    else
    {
        last_frame_time += FRAME_TIME * CLOCKS_PER_MSEC ;
        td_wait(FRAME_TIME - diff_in_ms);
    }

#endif
}

/*****************
 *** RESSOURCES ***
 ******************/
//définitions dans terminaldefense.h

// Types de monstres (caractéristiques)
const monster_type_t runner =
{
    .speed            = 2,
    .damage           = 5,
    .base_life        = 15,
    .given_ressources = 20,
    .sprite           =
    {
        .color            = COL_RED,
        .background_color = COL_DEFAULT,
        .c1               = '\xe2', //≫
        .c2               = '\x89',
        .c3               = '\xab',
        .c4               = '\0',
    },
};
const monster_type_t armored =
{
    .speed            = 10,
    .damage           = 30,
    .base_life        = 100,
    .given_ressources = 30,
    .sprite           =
    {
        .color            = COL_CYAN,
        .background_color = COL_DEFAULT,
        .c1               = '\xe2', //⇛
        .c2               = '\x87',
        .c3               = '\x9b',
        .c4               = '\0',
    }
};

// Types de défenses
const defense_type_t basic_wall =
{
    .max_life = 600,
    .damage   = 0,
    .cost     = 100,
    .range    = 0,
    .sprite   =
    {
        .color            = COL_RED,
        .background_color = COL_DEFAULT,
        .c1               = '\xE2', //⣿
        .c2               = '\xA3',
        .c3               = '\xBF',
        .c4               = '\0',
    },
    .desc_txt  = "Mur basique", //affiché au survol
    .short_txt = "Mur\nbasique" //affiché dans le menu construction (a coté des choix)
};
const defense_type_t advanced_wall =
{
	.max_life = 3500,
	.damage   = 0,
	.cost     = 500,
	.range    = 0,
	.sprite   =
	{
		.color            = COL_GREEN,
		.background_color = COL_DEFAULT,
		.c1               = '\xE2', //⣿
		.c2               = '\xA3',
		.c3               = '\xBF',
		.c4               = '\0',
	},
	.desc_txt  = "Mur bien plus\nresistant",
	.short_txt = "Mur++"
};
const defense_type_t electric_wall =
{
	.max_life = 1500,
	.damage   = 45,
	.cost     = 750,
	.range    = 1,
	.sprite   =
	{
		.color            = COL_CYAN,
		.background_color = COL_DEFAULT,
		.c1               = '\xE2', //⣿
		.c2               = '\xA3',
		.c3               = '\xBF',
		.c4               = '\0',
	},
	.desc_txt  = "Mur electrique:\nfait des degats\nau contact",
	.short_txt = "Mur\nelectrique"
};
const defense_type_t basic_turret =
{
    .max_life = 50,
    .damage   = 15,
    .range    = 5,
    .cost     = 200,
    .sprite   =
    {
        .color            = COL_MAGENTA,
        .background_color = COL_DEFAULT,
        .c1               = '\xc2', //¶
        .c2               = '\xb6',
        .c3               = '\0',
        .c4               = '\0',
    },
    .desc_txt  = "Tourelle\nde base",
    .short_txt = "Tourelle\nbasique"
};
const defense_type_t medium_turret =
{
	.max_life = 50,
	.damage   = 25,
	.range    = 7,
	.cost     = 475,
	.sprite   =
	{
		.color            = COL_GREEN,
		.background_color = COL_DEFAULT,
		.c1               = '\xc2', //¶
		.c2               = '\xb6',
		.c3               = '\0',
		.c4               = '\0',
	},
	.desc_txt  = "Tourelle un peu +\npuissante",
	.short_txt = "Tourelle\nmoyenne"
};
const defense_type_t long_range_turret =
{
	.max_life = 50,
	.damage   = 25,
	.range    = 16,
	.cost     = 750,
	.sprite   =
	{
		.color            = COL_WHITE,
		.background_color = COL_DEFAULT,
		.c1               = '\xc2', //¶
		.c2               = '\xb6',
		.c3               = '\0',
		.c4               = '\0',
	},
	.desc_txt  = "Tourelle avancee\ngrande portee",
	.short_txt = "Tourelle\ndistance"
};
const defense_type_t heavy_turret =
{
	.max_life = 50,
	.damage   = 60,
	.range    = 8,
	.cost     = 1200,
	.sprite   =
	{
		.color            = COL_BLUE,
		.background_color = COL_DEFAULT,
		.c1               = '\xc2', //¶
		.c2               = '\xb6',
		.c3               = '\0',
		.c4               = '\0',
	},
	.desc_txt  = "Tourelle avancee\ninfligeant\nde gros degats",
	.short_txt = "Tourelle\npuissante"
};
//la base est juste une défense qu'on ne peut pas construire, et qui est un gros mur
const defense_type_t la_base =
{
	.max_life = 10000,
	.damage   = 0,
	.range    = 0,
	.cost     = 0,
	.sprite   =
	{
		.color            = COL_GREEN,
		.background_color = COL_DEFAULT,
		.c1               = '\xe2', //⚑
		.c2               = '\x9a',
		.c3               = '\x91',
		.c4               = 0,
	},
	.desc_txt  = "Votre base,\ndefendez la!",
	.short_txt = "Base!"
};

// Un cadre pour afficher les choix de sélection
/*  ╭─╮
 *  │ │
 *  ╰─╯
 */
const picture_t frame =
{
    .size                     =
    {
        3, 3, 3
    },
    .data                     = (pixel_t[9])
    {
        // LIGNE 0
        [0] =
        {
            .color            = COL_TEXT,
            .background_color = COL_DEFAULT,
            .c1               = '\xe2',
            .c2               = '\x95',
            .c3               = '\xad',
            .c4               = '\x00',
        },
        [1] =
        {
            .color            = COL_TEXT,
            .background_color = COL_DEFAULT,
            .c1               = '\xe2',
            .c2               = '\x94',
            .c3               = '\x80',
            .c4               = '\x00',
        },
        [2] =
        {
            .color            = COL_TEXT,
            .background_color = COL_DEFAULT,
            .c1               = '\xe2',
            .c2               = '\x95',
            .c3               = '\xae',
            .c4               = '\x00',
        },

        // LIGNE 1
        [3] =
        {
            .color            = COL_TEXT,
            .background_color = COL_DEFAULT,
            .c1               = '\xe2',
            .c2               = '\x94',
            .c3               = '\x82',
            .c4               = '\x00',
        },
        [4] = // ESPACE
        {
            .color            = COL_TEXT,
            .background_color = COL_DEFAULT,
            .c1               = '\x20',
            .c2               = '\x00',
            .c3               = '\x00',
            .c4               = '\x00',
        },
        [5] =
        {
            .color            = COL_TEXT,
            .background_color = COL_DEFAULT,
            .c1               = '\xe2',
            .c2               = '\x94',
            .c3               = '\x82',
            .c4               = '\x00',
        },

        // LIGNE 2
        [6] =
        {
            .color            = COL_TEXT,
            .background_color = COL_DEFAULT,
            .c1               = '\xe2',
            .c2               = '\x95',
            .c3               = '\xb0',
            .c4               = '\x00',
        },
        [7] =
        {
            .color            = COL_TEXT,
            .background_color = COL_DEFAULT,
            .c1               = '\xe2',
            .c2               = '\x94',
            .c3               = '\x80',
            .c4               = '\x00',
        },
        [8] =
        {
            .color            = COL_TEXT,
            .background_color = COL_DEFAULT,
            .c1               = '\xe2',
            .c2               = '\x95',
            .c3               = '\xaf',
            .c4               = '\x00',
        }
    }
};


// L'arbre de sélection des tourelles
const defence_choice_tree_t walls =
{
	.icon                 =
	{
		.c1               = 'M',
		.c2               = '\0',
		.c3               = '\0',
		.c4               = '\0',
		.color            = COL_TEXT,
		.background_color = COL_DEFAULT
	},
	.short_txt     = "Murs",
	.desc_txt      = "Murs, bloquent\nle passage\ndes monstres",
	.defense_count = 3,
	.defenses      = (const defense_type_t * [3] )
	{
		[0] = &electric_wall,
		[1] = &advanced_wall,
		[2] = &basic_wall,
	},
	.sub_category_count = 1,
	.sub_categories     = (const defence_choice_tree_t * [1])
	{
		[0] = &main_selection_tree, //rendu comme le bouton "Retour"
	}
};
//l'arbre de sélection des tourelles
const defence_choice_tree_t turrets =
{
	.icon                 =
	{
		.c1               = 'T',
		.c2               = '\0',
		.c3               = '\0',
		.c4               = '\0',
		.color            = COL_TEXT,
		.background_color = COL_DEFAULT
	},
	.short_txt     = "Tourelles",
	.desc_txt      = "Tourelles,\nattaquent les ennemis\na distance",
	.defense_count = 4,
	.defenses      = (const defense_type_t * [4] )
	{
		[0] = &heavy_turret,
		[1] = &long_range_turret,
		[2] = &medium_turret,
		[3] = &basic_turret,
	},
	.sub_category_count = 1,
	.sub_categories     = (const defence_choice_tree_t * [1])
	{
		[0] = &main_selection_tree, //rendu comme le bouton "Retour"
	}
};
//l'arbre initial du menu construction
const defence_choice_tree_t main_selection_tree =
{
    .icon                 =
    {
        .c1               = '\xE2',
        .c2               = '\x86',
        .c3               = '\x90',
        .c4               = '\0',
        .color            = COL_TEXT,
        .background_color = COL_DEFAULT
    },
    .short_txt = "Retour", //puisqu'on y accède depuis les sous-menus
    .desc_txt  = "Menu de construction principal",

    .defense_count = 0,
    .defenses      = NULL,

    .sub_category_count = 2,
    .sub_categories     = (const defence_choice_tree_t * [2])
    {
        [1] = &turrets,
        [0] = &walls,
    }
};
//niveau de difficultés
const difficulty_level difficulty_levels[DIFFICULTY_LEVEL_COUNT] = 
{
    [0] = 
    {
        .difficulty_name  = "Easy",
        .display_color    = COL_GREEN,
        .difficulty_value = 4,
    },

    [1] = 
    {
        .difficulty_name  = "Medium",
        .display_color    = COL_BLUE,
        .difficulty_value = 6,
    },

    [2] = 
    {
        .difficulty_name  = "Hard",
        .display_color    = COL_YELLOW,
        .difficulty_value = 8,
    },

    [3] = 
    {
        .difficulty_name  = "HARDCORE™",
        .display_color    = COL_RED,
        .difficulty_value = 10,
    }
};

