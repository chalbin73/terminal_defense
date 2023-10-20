#include "graphical.h"

struct winsize termsize;
struct termios orig_term_settings;
COLOR cur_color,cur_color_background;

const char* CHARS_BLOCK_UP = "▀";
const char* CHARS_BLOCK_DOWN = "▄";
const char* CHARS_BLOCK_FULL = "█";
const char*  CHARS_BLOCK_LEFT = "▌";
const char* CHARS_BLOCK_RIGHT = "▐";
const char* CHARS_BLOCK_LIGHT = "░";
const char* CHARS_BLOCK_MEDIUM = "▒";
const char* CHARS_BLOCK_DARK = "▓";
const char* CHARS_TRIANGLE_LEFT = "◀";
const char* CHARS_TRIANGLE_RIGHT = "▶";
const char* CHARS_TRIANGLE_DOWN = "▼";
const char* CHARS_TRIANGLE_UP = "▲";


void graphical_cleanup() {
	//appellé a la sortie, résponsable de clean les graphisme

	//restore les attributs du terminal
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_term_settings);
	//réaffiche le curseur, et reset la couleur
	printf("\e[?25h \e[0m \n");

}

void init_graphical(){
	//mets en place les graphismes
	//maximise le terminal
	printf("\e[8;99999;99999t");
	//obtention de la taille de l'écran
	ioctl(0, TIOCGWINSZ, &termsize);

	//setup du terminal sur linux
	#if SYSTEM_POSIX
		//sauvegarde les attributs terminal (pour restauration future) et mets le term en raw mode
		tcgetattr(STDIN_FILENO, &orig_term_settings);
		struct termios raw = orig_term_settings;
		raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
		raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
		raw.c_oflag &= ~(OPOST);
		raw.c_cc[VMIN] = 0; raw.c_cc[VTIME] = 1; //VTIME a 1 permet de timeout scanf au bout de 0.1 secondes (jeu a 10 fps)
		tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
	#endif //SYSTEM_POSIX
	//setup windows
	#if SYSTEM_WINDOWS
		// Set output mode to handle virtual terminal sequences
		HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
		if (hOut == INVALID_HANDLE_VALUE)
		{
			printf("Terminal setup failed, please use a compatible terminal");
			exit(128);
		}
		HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
		if (hIn == INVALID_HANDLE_VALUE)
		{
			printf("Terminal setup failed, please use a compatible terminal");
			exit(128);
		}

		DWORD dwOriginalOutMode = 0;
		DWORD dwOriginalInMode = 0;
		if (!GetConsoleMode(hOut, &dwOriginalOutMode))
		{
			printf("Terminal setup failed, please use a compatible terminal");
			exit(128);
		}
		if (!GetConsoleMode(hIn, &dwOriginalInMode))
		{
			printf("Terminal setup failed, please use a compatible terminal");
			exit(128);
		}

		DWORD dwRequestedOutModes = ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN;
		DWORD dwRequestedInModes = ENABLE_VIRTUAL_TERMINAL_INPUT;

		DWORD dwOutMode = dwOriginalOutMode | dwRequestedOutModes;
		if (!SetConsoleMode(hOut, dwOutMode))
		{
			// we failed to set both modes, try to step down mode gracefully.
			dwRequestedOutModes = ENABLE_VIRTUAL_TERMINAL_PROCESSING;
			dwOutMode = dwOriginalOutMode | dwRequestedOutModes;
			if (!SetConsoleMode(hOut, dwOutMode))
			{
				// Failed to set any VT mode, can't do anything here.
				printf("Terminal setup failed, please use a compatible terminal");
				exit(128);
			}
		}

		DWORD dwInMode = dwOriginalInMode | dwRequestedInModes;
		if (!SetConsoleMode(hIn, dwInMode))
		{
			// Failed to set VT input mode, can't do anything here.
			printf("Terminal setup failed, please use a compatible terminal");
			exit(128);
		}

   	#endif //SYSTEM_WINDOWS
	
	//set terminal locale to make behavior more robust accros different settings
	setlocale(LC_ALL, "");
	//set la valeur par default de certaine variables globales
	cur_color=COL_DEFAULT;
	cur_color_background=COL_DEFAULT;
	//chaine vide (des 0) pour l'input_string
}


picture_t pict_crop_bound(picture_t pict, uint xmin, uint xmax, uint ymin, uint ymax){
	//coupe les cotés d'une image, sans copier les données
	//(une modification de cette image modifiera l'image de départ)
	return (picture_t){
			   .col=xmax-xmin+1,
			   .row=ymax-ymin+1,
			   .stride=pict.stride,
			   .data=pict.data+xmin+pict.stride*ymin
	};
}
picture_t pict_crop_size(picture_t pict, uint xmin, uint col, uint ymin, uint row){
	//coupe les cotés d'une image, sans copier les données
	return (picture_t){
			   .col=col,
			   .row=row,
			   .stride=pict.stride,
			   .data=pict.data+xmin+pict.stride*ymin
	};
}

void go_to(int x,int y){
	//mets le curseur a la position x,y sur le terminal
	//en utilisant des sequence échapée
	printf("\e[%i;%iH",x,y);
}
void set_color(int color){
	//si la couleur est différente de l'actuelle
	//set la nouvelle couleur a l'aide de carctères d'échapement
	if (color != cur_color){
		cur_color=color;
		if (color==-1){
			printf("\e[39m");
		} else {
			printf("\e[38;5;%im",color);
		}
	}
}
void set_color_background(int color){
	//si la couleur est différente de l'actuelle
	//set la nouvelle couleur a l'aide de carctères d'échapement
	if (color != cur_color_background){
		cur_color_background=color;
		if (color==-1){
			printf("\e[49m");
		} else {
			printf("\e[48;5;%im",color);
		}
	}
}

void pict_display(picture_t pict, uint x, uint y){
	//affiche sur le terminal une image a une position donnée

	//pour chaque ligne
	for (int i=0; i<pict.row; i++){
		//on se mets au debut de la ligne (en x,y+i)
		go_to(x,y+i);
		//pour chaque colonne
		for (int j=0; j<pict.col; j++){
			//on obtient le pixel a afficher et on l'affiche
			pixel_t* pixel=&pict.data[j+i*pict.stride];
			set_color(pixel->color);
			set_color_background(pixel->background_color);
			printf("%.4s",&(pixel->c1));
		}
	}
}
