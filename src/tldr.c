#include "tldr.h"


struct winsize winsize;

int main (int argc,char **argv,char **env){
	ioctl(0, TIOCGWINSZ, &winsize);
	setlocale(LC_CTYPE, "");
    printf("%lc",CHARS_BLOCK_UP);
	*(int*)0;
}

void printscreen(int x,int y,int *monstre,char* background){
	
}
