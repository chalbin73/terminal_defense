#ifndef CONFIG
#define CONFIG

//** raccourcis claviers **
//vous devez mettre des lettres minuscule (Ctrl+Char autorisé pour key_quit et key_pause)
enum keys {
	KEY_QUIT = '', //Ctrl+C
	KEY_UP = 'z',
	KEY_DOWN = 's',
	KEY_LEFT = 'q',
	KEY_RIGHT = 'd',
	KEY_BUILD = 'c',
	KEY_PAUSE = 'p',
};

//** COULEURS  **
//quelques couleurs standard. (a ne pas toucher, sauf si vous etes daltonien/voulez inverser toute les couleurs du jeu)
//il est toujour possible d'utiliser toute la palette de 256 couleurs terminal
//(RGB 8bit + quelques gris et couleurs standard)
typedef enum COLOR {
	COL_DEFAULT   =-1,
	COL_BLACK     =0,
	COL_RED       =1,
	COL_GREEN     =2,
	COL_YELLOW    =3,
	COL_BLUE      =4,
	COL_MAGENTA   =5,
	COL_CYAN      =6,
	COL_WHITE     =7,
	COL_GRAY_DARK =233,
	COL_GRAY      =237,
	COL_GRAY_LIGHT=240,

	//* Couleurs modifiables *
	COL_CURSOR = COL_GREEN,                  //couleur du curseur
	COL_BACKGROUND = COL_BLACK,              //couleur du fond (text)
	COL_BOARD_BACKGROUND_1 = COL_GRAY_DARK,  //couleurs des cases du damier
	COL_BOARD_BACKGROUND_2 = COL_BLACK,
	COL_TEXT = COL_WHITE,                    //couleur du texte


} COLOR;


//** Autre configs **

#endif //ifdef config
