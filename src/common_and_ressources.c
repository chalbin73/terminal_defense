//fonctions utilitaires
#include "common.h"

//ressources
#include "tldr.h"

/****************************
 *** FONCTIONS UTILITAIRES ***
 *****************************/
#if SYSTEM_POSIX
struct timespec last_frame_time={0,0};
#elif SYSTEM_WINDOWS
long last_frame_time=0;
#endif

void   *safe_malloc(size_t size)
{
	//obtient de la place mémoire et vérifie qu'elle a bien été alouée
	void *ptr;
	ptr = malloc(size);
	if (ptr!=NULL)
	{
		return ptr;
	}
	EXIT_MSG = "malloc a fail! sortie de programme!";
	//la fonction cleanup *devrait* clean notre bordel
	exit(254);
}

void   *safe_realloc(void *ptr, size_t new_size)
{
	//obtient de la place mémoire et vérifie qu'elle a bien été alouée
	void *new_ptr;
	new_ptr = realloc(ptr, new_size);
	if (new_ptr!=NULL)
	{
		return new_ptr;
	}
	EXIT_MSG = "realloc a fail! sortie du programme!";
	//la fonction cleanup *devrait* clean notre bordel
	exit(254);
}
int32_t min(int32_t a, int32_t b){
	if (a<b) return a;
	else return b;
}
int32_t max(int32_t a, int32_t b){
	if (a>b) return a;
	else return b;
}


//attend ms milliseconde
int    tldr_wait(long ms)
{
    // Si le système cible est linux
	#if SYSTEM_POSIX
	//nanosleep accept un struct en seconds et nanoseconds
	//on convertit donc l'entrée
	struct timespec ts;
	ts.tv_sec  = ms / 1000;
	ts.tv_nsec = (ms % 1000) * 1000000;
	return nanosleep(&ts, &ts);
	#else // Si le système cible est autre chose (windows)
	return Sleep(ms);
	#endif
}

//attend la prochaine frame
void wait_for_next_frame(void){
#if SYSTEM_POSIX
	struct timespec actual_time;
	clock_gettime(CLOCK_MONOTONIC, &actual_time);
	long int diff_in_ms=(actual_time.tv_nsec-last_frame_time.tv_nsec)/1000000 + //nanoseconde component
	                     (actual_time.tv_sec - last_frame_time.tv_sec)*1000; //second component
	if (diff_in_ms<0 || diff_in_ms>FRAME_TIME) {
		//heavily lagging,
		//clock uninitialized or looped around (32-bits maybe)
		last_frame_time=actual_time;
	} else {
		last_frame_time.tv_nsec+= FRAME_TIME * 1000000;  //on ajoute FRAME_TIME millisecondes
		if (last_frame_time.tv_nsec>=1000000000){
			last_frame_time.tv_nsec-=1000000000;
			last_frame_time.tv_sec+=1;
		}
		tldr_wait(FRAME_TIME-diff_in_ms);
	}
#elif SYSTEM_WINDOWS
#error Clock not yet implemented on windows.
#endif
}

/*****************
 *** RESSOURCES ***
 ******************/

// Types de monstres
const monster_type_t runner =
{
	.speed    = 4,
	.damage   = 10,
	.max_life = 200,
	.sprite   =
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
	.speed    = 10,
	.damage   = 30,
	.max_life = 1000,
	.sprite   =
	{
		.color            = COL_CYAN,
		.background_color = COL_DEFAULT,
		.c1               = '\xe2', //⇛
		.c2               = '\x87',
		.c3               = '\x9b',
		.c4               = '\0',
	}
};

// Types de defenses
const defense_type_t basic_wall =
{
	.max_life = 1000,
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
	.ui_txt               = "Basic walls"
};
const defense_type_t basic_turret =
{
	.max_life = 100,
	.damage   = 30,
	.range    = 10,
	.cost     = 200,
	.sprite   =
	{
		.color            = COL_CYAN,
		.background_color = COL_DEFAULT,
		.c1               = '\xc2', //¶
		.c2               = '\xb6',
		.c3               = '\0',
		.c4               = '\0',
	},
	.ui_txt               = "Basic turret"
};
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
	.ui_txt               = "Your base, defend it!"
};

// Un cadre pour afficher les choix de selection
/*  ╭─╮
 *  │ │
 *  ╰─╯
 */
const picture_t frame =
{
	.size                     ={3, 3, 3},
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


// L'arbre de selection des tourelles
const defence_choice_tree_t main_selection_tree =
{
	.icon               = {
		.c1               = 'D',
		.c2               = '\0',
		.c3               = '\0',
		.c4               = '\0',
		.color            = COL_TEXT,
		.background_color = COL_DEFAULT
	},
	.ui_txt             = "Defenses",

	.defense_count = 2,
	.defenses      = (const defense_type_t * [2] )
	{
		[0] = &basic_turret,
		[1] = &basic_wall,
	},

	.sub_category_count = 1,
	.sub_categories     = (const defence_choice_tree_t * [1])
	{
		[0] = &main_selection_tree, // LOOP FOR TESTING PURPOSES
	}
};

