#ifndef COMMONH
#define COMMONH

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>

//malloc, en vérifiant que tout s'est bien passé
//plante "proprement" si ce n'est pas le cas
void* safe_malloc(size_t size);
#endif //def COMMONH
