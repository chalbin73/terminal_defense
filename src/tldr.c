#include "tldr.h"

struct winsize winsize;

int main (int argc,char **argv,char **env){
	ioctl(0, TIOCGWINSZ, &winsize);
	printf("%u %u",winsize.ws_col,winsize.ws_row);
	setlocale(LC_CTYPE, "");
	srand((unsigned int)time(NULL));

	return rand();
}

void printscreen(int x,int y,int *monstre,char* background){
	
}

picture pict_crop_bound(picture pict, uint xmin, uint xmax, uint ymin, uint ymax){
	//coupe les cotés d'une image, sans copier les données 
	//(une modification de cette image modifiera l'image de départ)
	return (picture){
		.col=xmax-xmin+1,
		.row=ymax-ymin+1,
		.stride=pict.stride,
		.data=pict.data+xmin+pict.stride*ymin
	};
}
picture pict_crop_size(picture pict, uint xmin, uint col, uint ymin, uint row){
	//coupe les cotés d'une image, sans copier les données
	return (picture){
		.col=col,
		.row=row,
		.stride=pict.stride,
		.data=pict.data+xmin+pict.stride*ymin
	};
}

