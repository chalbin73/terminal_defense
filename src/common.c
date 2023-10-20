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
		return sleep(((float)ms)/1000.);
	#else
		return Sleep(ms);
	#endif
}
