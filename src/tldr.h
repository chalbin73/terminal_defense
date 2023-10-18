#include <wchar.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <locale.h>
#include <time.h>
#include <stdlib.h>
#include <termios.h>

#include "config.h"

typedef unsigned int uint ;

enum CHAR {
	CHARS_BLOCK_UP       = L'▀',
	CHARS_BLOCK_DOWN     = L'▄',
	CHARS_BLOCK_FULL     = L'█',
	CHARS_BLOCK_LEFT     = L'▌',
	CHARS_BLOCK_RIGHT    = L'▐',
	CHARS_BLOCK_LIGHT    = L'░',
	CHARS_BLOCK_MEDIUM   = L'▒',
	CHARS_BLOCK_DARK     = L'▓',
	CHARS_TRIANGLE_LEFT  = L'◀',
	CHARS_TRIANGLE_RIGHT = L'▶',
	CHARS_TRIANGLE_DOWN  = L'▼',
	CHARS_TRIANGL_UP     = L'▼',
};

typedef wchar_t sprite_t;

enum DIRECTION {
	DIR_UP    = 0,
	DIR_DOWN  = 1,
	DIR_RIGHT = 2,
	DIR_LEFT  = 3,
};

//Couleur pour les charactères d'échapement.
//Il faut ajouter 10 pour set le background.
enum COLOR {
	COL_BLACK  =30,
	COL_RED    =31,
	COL_GREEN  =32,
	COL_YELLOW =33,
	COL_BLUE   =34,
	COL_MAGENTA=35,
	COL_CYAN   =36,
	COL_WHITE  =37,
	COL_DEFAULT=39,
};

//un pixel: une couleur est 1 caractère UTF_8 (donc 4 char)
typedef struct {
	enum COLOR color;
	char c1;
	char c2;
	char c3;
	char c4;
} pixel_t ;

// Les images sont des tableau unidirectionels de caractères
// (on met les lignes les une après les autres)
// stride renseigne de combien on doit se décaler pour accéder a la ligne suivante
typedef struct {
	int col;
	int row;
	int stride;
	pixel_t* data;
} picture_t;

typedef struct {
	sprite_t sprite;
	enum DIRECTION direction;
	int32_t posx;
	int32_t posy;
	uint32_t id;
} monster_t;

