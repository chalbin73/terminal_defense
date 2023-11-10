//fonctions utilitaires
#include "common.h"

//ressources
#include "terminaldefense.h"

/****************************
 *** FONCTIONS UTILITAIRES ***
 *****************************/
#if SYSTEM_POSIX
struct timespec last_frame_time =
{
	0, 0
};
#elif SYSTEM_WINDOWS
long last_frame_time = 0;
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


//attend ms milliseconde
int        td_wait(long ms)
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
void    wait_for_next_frame(void)
{
#if SYSTEM_POSIX
	struct timespec actual_time;
	clock_gettime(CLOCK_MONOTONIC, &actual_time);
	long int diff_in_ms = (actual_time.tv_nsec - last_frame_time.tv_nsec) / 1000000 + //nanoseconde component
	                      (actual_time.tv_sec - last_frame_time.tv_sec) * 1000; //second component
	if (diff_in_ms<0 || diff_in_ms>FRAME_TIME)
	{
		//heavily lagging,
		//clock uninitialized or looped around (32-bits maybe)
		last_frame_time = actual_time;
	}
	else
	{
		last_frame_time.tv_nsec += FRAME_TIME * 1000000;  //on ajoute FRAME_TIME millisecondes
		if (last_frame_time.tv_nsec>=1000000000)
		{
			last_frame_time.tv_nsec -= 1000000000;
			last_frame_time.tv_sec  += 1;
		}
		td_wait(FRAME_TIME - diff_in_ms);
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

// Types de defenses
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
	.desc_txt  = "Mur basique",
	.short_txt = "Mur\nbasique"
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
	.desc_txt  = "Mur bien plus\nrésistant",
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
	.desc_txt  = "Mur éléctrique:\nfait des dégats\nau contact",
	.short_txt = "Mur\néléctrique"
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
	.cost     = 500,
	.sprite   =
	{
		.color            = COL_GRAY,
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
	.range    = 14,
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
	.desc_txt  = "Tourelle avancée\ngrande portée",
	.short_txt = "Tourelle\ndistance"
};
const defense_type_t heavy_turret =
{
	.max_life = 50,
	.damage   = 60,
	.range    = 7,
	.cost     = 1250,
	.sprite   =
	{
		.color            = COL_BLUE,
		.background_color = COL_DEFAULT,
		.c1               = '\xc2', //¶
		.c2               = '\xb6',
		.c3               = '\0',
		.c4               = '\0',
	},
	.desc_txt  = "Tourelle avancée\ngros dégats",
	.short_txt = "Tourelle\npuissante"

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
	.desc_txt  = "Votre base,\ndefendez là!",
	.short_txt = "Base!"
};

// Un cadre pour afficher les choix de selection
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


// L'arbre de selection des tourelles
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
		[0] = &main_selection_tree,
	}
};
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
	.desc_txt      = "Tourelles,\nattaquent les\nà distance",
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
		[0] = &main_selection_tree,
	}
};

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
	.short_txt = "Retour",
	.desc_txt  = "Menu de\nconstruction\nprincipal",

	.defense_count = 0,
	.defenses      = NULL,

	.sub_category_count = 2,
	.sub_categories     = (const defence_choice_tree_t * [2])
	{
		[1] = &turrets,
		[0] = &walls,
	}
};

