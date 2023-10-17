#include <uchar.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <locale.h>
#include "config.h"
enum CHARS {
	CHARS_BLOCK_UP = U'▀',
	CHARS_BLOCK_DOWN = U'▄',
	CHARS_BLOCK_FULL = U'█',
	CHARS_BLOCK_LEFT = U'▌',
	CHARS_BLOCK_RIGHT = U'▐',
	CHARS_BLOCK_LIGHT = U'░',
	CHARS_BLOCK_MEDIUM = U'▒',
	CHARS_BLOCK_DARK = U'▓',
	CHARS_TRIANGLE_LEFT = U'◀',
	CHARS_TRIANGLE_RIGHT = U'▶',
	CHARS_TRIANGLE_DOWN = U'▼',
	CHARS_TRIANGL_UP = U'▼',
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
typedef struct monster monster;
