#include <uchar.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <locale.h>
#include <time.h>
#include <stdlib.h>

#include "config.h"

<<<<<<< HEAD
enum CHARS {
	CHARS_BLOCK_UP = L'▀',
	CHARS_BLOCK_DOWN = L'▄',
	CHARS_BLOCK_FULL = L'█',
	CHARS_BLOCK_LEFT = L'▌',
	CHARS_BLOCK_RIGHT = L'▐',
	CHARS_BLOCK_LIGHT = L'░',
	CHARS_BLOCK_MEDIUM = L'▒',
	CHARS_BLOCK_DARK = L'▓',
	CHARS_TRIANGLE_LEFT = L'◀',
	CHARS_TRIANGLE_RIGHT = L'▶',
	CHARS_TRIANGLE_DOWN = L'▼',
	CHARS_TRIANGL_UP = L'▼',
};
enum DIRECTIONS {
	DIR_UP = 0,
	DIR_DOWN = 1,
	DIR_RIGHT = 2,
	DIR_LEFT = 3,
=======
enum CHAR
{
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
>>>>>>> b9055a789423a9031674896c8d63b6fdf76ce04a
};

typedef char32_t sprite_t;

enum DIRECTION
{
    DIR_UP    = 0,
    DIR_DOWN  = 1,
    DIR_RIGHT = 2,
    DIR_LEFT  = 3,
};
<<<<<<< HEAD
typedef struct monster monster;

typedef struct {
	int col;
	int row;
	int stride;
	char32_t* data;
} picture;
=======

struct monster
{
    sprite_t     sprite;
    DIRECTION    direction;
    int32_t      posx;
    int32_t      posy;
    uint32_t     id;
};
typedef struct monster monster_t;


>>>>>>> b9055a789423a9031674896c8d63b6fdf76ce04a
