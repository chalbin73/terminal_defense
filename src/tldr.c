#include "tldr.h"

struct winsize winsize;
struct termios orig_term_settings;
enum COLOR cur_color;


void cleanup() {
	//restore terminal attributes on exit
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_term_settings);
	//réaffiche le curseur, et reset la couleur
	printf("\e[?25h \e[0m \n");
}


int main (int argc,char **argv,char **env){
	//***setup initial***
	//maximise le terminal
	printf("\e[8;99999;99999t");
	//obtention de la taille de l'écran
	ioctl(0, TIOCGWINSZ, &winsize);
	printf("%u\u2200u",winsize.ws_col,winsize.ws_row);
	//sauvegarde les attributs terminal (pour restauration future) et mets le term en raw mode
	tcgetattr(STDIN_FILENO, &orig_term_settings);
	//set les nouveaux attributs (meilleurs pour faire un jeu)
	struct termios raw = orig_term_settings;
	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	raw.c_oflag &= ~(OPOST);
	raw.c_cc[VMIN] = 1; raw.c_cc[VTIME] = 0;
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
	//set terminal locale to make behavior more robust accros different settings
	setlocale(LC_ALL, "");
	//initialize randomness using system time
	srand((unsigned int)time(NULL));
	//renseigne la fonction a éxécuter a la sortie du programme
	atexit(cleanup);
	//initialise les variables globales
	cur_color=COL_DEFAULT;

	return rand();
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



void pict_display(picture_t pict, uint x, uint y){
	//affiche sur le terminal une image a une position donnée
	
	//pour chaque ligne
	for (int i=0;i<pict.row;i++){
		
	}
}
