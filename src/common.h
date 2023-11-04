#define _POSIX_C_SOURCE 199309L //sinon, nanosleep n'est pas définie (et usleep non plus)
                                //(et je ne comprend pas quelle fonction on devrait utiliser)

#ifndef COMMONH
#define COMMONH

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>


#if defined(__unix__) || (defined (__APPLE__) && defined ( __MACH__) )
	#define SYSTEM_POSIX    1
	#define SYSTEM_WINDOWS  0
	#include <unistd.h>
	#include <time.h>
#else
	#ifdef _WIN64
		#define SYSTEM_WINDOWS  1
		#define SYSTEM_POSIX    0
		#include <windows.h>
	#else
		#error "Only Linux and Windows 64bit is supported"
	#endif
#endif

//raccourcis d'écriture
typedef unsigned int uint;

//associé a un tableau
//renseigne sa largeur (nb_col), hauteur(nb_row),et espacement entre chaque ligne (stride)
//dans un tableau "complet", stride==nb_col, mais stocker stride a part
//permet de faire des vues partielles des tableaux (un masque/cropping ...)
typedef struct {
	int32_t col;
	int32_t row;
	int32_t stride;
} tab_size_t;
typedef struct {
	int32_t x;
	int32_t y;
} coordonee_t;
//Global var
extern const char* EXIT_MSG;

//attend ms milliseconde
int wait(long int ms);

//malloc, en vérifiant que tout s'est bien passé
//plante "proprement" si ce n'est pas le cas
void* safe_malloc(size_t size);
//pareil pour realloc
void* safe_realloc(void* ptr,size_t new_size);
/* @brief renvoi l'offset associé a un couple coo-stride
 *
 * @param coo coordonée de demande
 * @param stride stride (écart entre chaque ligne) du tableau associé
 * @return l'indice d'accès dans le tableau
 */
int32_t offset_of(coordonee_t coo,int32_t stride);
#endif //def COMMONH
