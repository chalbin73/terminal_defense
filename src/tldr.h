#include <uchar.h>

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

struct sprite_t {
	char16_t up_left;
	char16_t down_left;
	char16_t up_right;
	char16_t down_right;
};
typedef struct sprite_t sprite_t ;

struct monstre {
	sprite_t sprite;
	int orientation;
	int posx;
	int posy;
};
typedef struct monstre monstre ;
