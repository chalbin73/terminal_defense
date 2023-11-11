#define _POSIX_C_SOURCE 199309L //sinon, nanosleep n'est pas définie (et usleep non plus)
                                //(et je ne comprends pas quelle fonction on devrait utiliser)

#ifndef COMMONH
#define COMMONH

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

//nombre de cycle de clock en une milliseconde
#define CLOCKS_PER_MSEC ( (CLOCKS_PER_SEC) / 1000 )
//temps entre chaque tour, en milliseconde
#define FRAME_TIME 100

#if defined(__unix__) || (defined (__APPLE__) && defined ( __MACH__) )
    #define SYSTEM_POSIX       1
    #define SYSTEM_WINDOWS     0
    #include <unistd.h>
    #include <time.h>
#else
    #ifdef _WIN64
        #define SYSTEM_WINDOWS 1
        #define SYSTEM_POSIX   0
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
typedef struct
{
    int32_t    col;
    int32_t    row;
    int32_t    stride;
} tab_size_t;
typedef struct
{
    int32_t    x;
    int32_t    y;
} coordonee_t;

// Mot clé extern :
/*
 * Le mot clé extern est utilisé plusieurs fois dans ce projet:
 *
 * Il est utilisé dans une déclaration dans un fichier .h pour indiquer au compilateur
 * que la définition de sa valeur se fait dans un fichier .c .
 *
 * Ainsi, une variable peut être simplement déclarée (sans valeur) dans .h avec le mot clé extern
 * et sa valeur peut être assigné dans un autre fichier .c.
 *
 * Cela permet d'avoir des variables globales statiques (qui ne sont pas vouées à changer).
 *
 */

//Global var
// Message de sortie du programme
extern const char *EXIT_MSG;

// @brief renvoie l'argument le plus petit
int32_t    min(int32_t a, int32_t b);
// @brief renvoie l'argument le plus grand
int32_t    max(int32_t a, int32_t b);

/* @brief Ramène v dans l'intervalle min-max
 *
 * @param v entier à borner
 * @param min_v borne inférieure
 * @param max_v borne supérieure
 *
 * @return v si min<v<max, la borne la plus proche sinon
 */
int32_t clamp(int32_t v, int32_t min_v, int32_t max_v);

// @brief attend ms millisecondes
int td_wait(long int ms);
// @brief attend la prochaine frame
void wait_for_next_frame(void);

/* @brief malloc, avec vérification de retour intégrée
 *
 * @param size Taille à allouer, en octet
 * @return un pointeur sur un tableau de size octet, toujours valide
 *
 * @note appelle exit (et ne retourne donc jamais) en cas de problème d'allocation
 */
void      *safe_malloc(size_t    size);
// @brief comme safe_malloc, mais pour realloc
void      *safe_realloc(void *ptr, size_t new_size);

/* @brief renvoi l'offset associé a un couple coo-stride
 *
 * @param coo coordonée de demande
 * @param stride stride (écart entre chaque ligne) du tableau associé
 * @return l'indice d'accès dans le tableau
 */
int32_t    offset_of(coordonee_t coo, int32_t stride);
#endif //def COMMONH

