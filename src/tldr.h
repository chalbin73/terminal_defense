#include <uchar.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <locale.h>
#include "config.h"
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
};

struct monster {
	char32_t sprite;
	int direction;
	int posx;
	int posy;
	int id;
};
typedef struct monster monster ;
