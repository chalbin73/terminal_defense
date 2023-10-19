#ifndef COMMONH
#define COMMONH

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>


#if defined(__unix__) || (defined (__APPLE__) && defined ( __MACH__) )
    #define SYSTEM_POSIX    1
	#define SYSTEM_WINDOWS  0
#else
	#ifdef _WIN64
		#define SYSTEM_WINDOWS  1
		#define SYSTEM_POSIX    0
	#else
		#error "Only Linux and Windows 64bit is supported"
	#endif
#endif

//malloc, en vérifiant que tout s'est bien passé
//plante "proprement" si ce n'est pas le cas
void* safe_malloc(size_t size);
#endif //def COMMONH
