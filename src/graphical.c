#include "graphical.h"

struct winsize winsize;
struct termios orig_term_settings;
enum COLOR cur_color;

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
	ioctl(0, TIOCGWINSZ, &winsize);
	//sauvegarde les attributs terminal (pour restauration future) et mets le term en raw mode
	tcgetattr(STDIN_FILENO, &orig_term_settings);
	struct termios raw = orig_term_settings;
	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	raw.c_oflag &= ~(OPOST);
	raw.c_cc[VMIN] = 1; raw.c_cc[VTIME] = 0;
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
	//set terminal locale to make behavior more robust accros different settings
	setlocale(LC_ALL, "");
	//set la valeur par default de certaine variables globales
	cur_color=COL_DEFAULT;
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
void set_color(enum COLOR color){
	//si la couleur est différente de l'actuelle
	//set la nouvelle couleur a l'aide de carctères d'échapement
	if (color != cur_color){
		cur_color=color;
		printf("\e[%im",color);
	}
}

void pict_display(picture_t pict, uint x, uint y){
	//affiche sur le terminal une image a une position donnée

	//pour chaque ligne
	for (int i=0; i<pict.row; i++){
		//on se mets au debut de la ligne (en x,y+i)
		go_to(x,y+i);
		//pour chaque colonne
		for (int j=0; j<pict.col; i++){
			//on obtient le pixel a afficher et on l'affiche
			pixel_t* pixel=&pict.data[i+j*pict.stride];
			set_color(pixel->color);
			printf("%.4s",&(pixel->c1));
		}
	}
}
