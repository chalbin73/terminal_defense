//fonctions utilitaires
#include "common.h"

//ressources
#include "tldr.h"

/****************************
 *** FONCTIONS UTILITAIRES ***
 *****************************/


void   *safe_malloc(size_t    size)
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

//attend ms milliseconde
int    wait(unsigned long    ms)
{
    #if SYSTEM_POSIX
    //nanosleep accept un struct en seconds et nanoseconds
    //on convertit donc l'entrée
    struct timespec ts;
    ts.tv_sec  = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;
    return nanosleep(&ts, &ts);
    #else
    return Sleep(ms);
    #endif
}

/*****************
 *** RESSOURCES ***
 ******************/

const monster_type runner =
{
    .speed    = 4,
    .damage   = 10,
    .max_life = 200,
    .sprite   =
    {
        .color            = COL_RED,
        .background_color = COL_DEFAULT,
        .c1               = 0xe2, //≫
        .c2               = 0x89,
        .c3               = 0xab,
        .c4               = '\0',
    },
};
const monster_type armored =
{
    .speed    = 10,
    .damage   = 30,
    .max_life = 1000,
    .sprite   =
    {
        .color            = COL_CYAN,
        .background_color = COL_DEFAULT,
        .c1               = 0xe2, //⇛
        .c2               = 0x87,
        .c3               = 0x9b,
        .c4               = '\0',
    }
};


const defense_type_t wall =
{
    .max_life = 1000,
    .damage   = 0,
    .cost     = 100,
    .range    = 0,
    .sprite   =
    {
        .color            = COL_MAGENTA,
        .background_color = COL_DEFAULT,
        .c1               = 226, //█
        .c2               = 150,
        .c3               = 136,
        .c4               = 0,
    },
    .ui_txt               = "Basic walls"
};
const defense_type_t basic_turret =
{
    .max_life = 100,
    .damage   = 150,
    .range    = 10,
    .cost     = 200,
    .sprite   =
    {
        .color            = COL_BLUE,
        .background_color = COL_YELLOW,
        .c1               = 0xc2, //¶
        .c2               = 0xb6,
        .c3               = '\0',
        .c4               = '\0',
    },
    .ui_txt               = "Basic turret"
};
const defense_type_t base =
{
    .max_life = 10000,
    .damage   = 0,
    .range    = 0,
    .cost     = 0,
    .sprite   =
    {
        .color            = COL_GREEN,
        .background_color = COL_DEFAULT,
        .c1               = 226, //⚑
        .c2               = 154,
        .c3               = 145,
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
            .c1               = 0xe2,
            .c2               = 0x95,
            .c3               = 0xad,
            .c4               = 0x00,
        },
        [1] =
        {
            .color            = COL_TEXT,
            .background_color = COL_DEFAULT,
            .c1               = 0xe2,
            .c2               = 0x94,
            .c3               = 0x80,
            .c4               = 0x00,
        },
        [2] =
        {
            .color            = COL_TEXT,
            .background_color = COL_DEFAULT,
            .c1               = 0xe2,
            .c2               = 0x95,
            .c3               = 0xae,
            .c4               = 0x00,
        },

        // LIGNE 1
        [3] =
        {
            .color            = COL_TEXT,
            .background_color = COL_DEFAULT,
            .c1               = 0xe2,
            .c2               = 0x94,
            .c3               = 0x82,
            .c4               = 0x00,
        },
        [4] = // ESPACE
        {
            .color            = COL_TEXT,
            .background_color = COL_DEFAULT,
            .c1               = 0x20,
            .c2               = 0x00,
            .c3               = 0x00,
            .c4               = 0x00,
        },
        [5] =
        {
            .color            = COL_TEXT,
            .background_color = COL_DEFAULT,
            .c1               = 0xe2,
            .c2               = 0x94,
            .c3               = 0x82,
            .c4               = 0x00,
        },

        // LIGNE 2
        [6] =
        {
            .color            = COL_TEXT,
            .background_color = COL_DEFAULT,
            .c1               = 0xe2,
            .c2               = 0x95,
            .c3               = 0xb0,
            .c4               = 0x00,
        },
        [7] =
        {
            .color            = COL_TEXT,
            .background_color = COL_DEFAULT,
            .c1               = 0xe2,
            .c2               = 0x94,
            .c3               = 0x80,
            .c4               = 0x00,
        },
        [8] =
        {
            .color            = COL_TEXT,
            .background_color = COL_DEFAULT,
            .c1               = 0xe2,
            .c2               = 0x95,
            .c3               = 0xaf,
            .c4               = 0x00,
        }
    }
};

const defence_choice_tree_t main_selection_tree =
{
    .icon               = (pixel_t) {
        .c1             = 0x44, .color = COL_TEXT, .background_color = COL_DEFAULT
    },
    .ui_txt             = "Defenses",

    .defense_count = 1,
    .defenses      = ( const defense_type_t * [1] )
    {
        [0] = &basic_turret,
    },

    .sub_category_count = 1,
    .sub_categories     = (const defence_choice_tree_t * [1])
    {
        [0] = &main_selection_tree, // LOOP FOR TESTING PURPOSES
    }
};

