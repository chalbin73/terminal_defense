WARNINGS := -Wall -Wextra -pedantic -Wshadow -Wpointer-arith -Wcast-align \
            -Wwrite-strings -Wmissing-prototypes -Wmissing-declarations \
            -Wredundant-decls -Wnested-externs -Winline -Wno-long-long \
            -Wstrict-prototypes
			#-Wconversion

CFLAGS=-std=c99 $(WARNINGS) -Isrc/ 
LDFLAGS=${WARNINGS}

RELEASE_FLAGS=-O3
DEBUG_FLAGS=-ggdb -g3 -O0 -fno-inline

SOURCES=
SOURCES+=common_and_ressources.c
SOURCES+=graphical.c
SOURCES+=terminaldefense.c


OBJECTS  := $(patsubst %.c,build/%.o,$(SOURCES))
DEPFILES := $(patsubst %.o,%.d,$(OBJECTS))

.PHONY: all debug help clean mpropper

all: CFLAGS+=$(RELEASE_FLAGS)
all: LDFLAGS+=-s $(RELEASE_FLAGS)
all: tldr

-include $(DEPFILES)

debug: CFLAGS+=$(DEBUG_FLAGS)
debug: LDFLAGS+=$(DEBUG_FLAGS)
debug: tldr

help:
	@echo "TLDR; Makefile help"
	@echo "    make             Compiles and links the program (stripped)"
	@echo "    make debug       Compiles and links the program with debugging flags"
	@echo "    make clean       Cleans compilation temporary files (slow down subsequent ones)"
	@echo "    make mpropper    Cleans compilation files and executable"
	@echo "    make help        Displays this help"

tldr: $(OBJECTS)
	@$(CC) $^ $(LDFLAGS) -o terminal_defense
	@echo "La compilation a réussie! Le programme s'apelle terminal_defense, lancez le en terminal avec la commande ./terminal_defence"

build:
	@mkdir -p build
		
build/%.o: src/%.c Makefile build
	@$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

clean:
	-rm -rf build

mpropper: clean
	-rm terminal_defense
