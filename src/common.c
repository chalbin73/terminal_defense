#include "common.h"

void* safe_malloc(size_t size){
	//obtient de la place mémoire et vérifie qu'elle a bien été alouée
	void* ptr;
	ptr=malloc(size);
	if (ptr!=NULL){
		return ptr;
	}
	fprintf(stderr, "malloc a fail!!! sortie de programme!!!");
	//la fonction cleanup *devrait* clean notre bordel
	exit(254);
}
//attend ms milliseconde
int wait(unsigned long ms){
	#if SYSTEM_POSIX
		//nanosleep accept un struct en seconds et nanoseconds
		//on convertit donc l'entrée
		struct timespec ts;
		ts.tv_sec = ms / 1000;
		ts.tv_nsec = (ms % 1000) * 1000000;
		return nanosleep(&ts, &ts);
	#else
		return Sleep(ms);
	#endif
}
