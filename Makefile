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
SOURCES+=tldr.c


OBJECTS  := $(patsubst %.c,build/%.o,$(SOURCES))
DEPFILES := $(patsubst %.o,%.d,$(OBJECTS))

.PHONY: all debug help build_dir

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
	@echo "    make clean       Cleans object files and executable"
	@echo "    make help        Displays this help"

tldr: $(OBJECTS)
	$(CC) $^ $(LDFLAGS) -o tldr 

build_dir:
	@mkdir -p build
		
build/%.o: src/%.c Makefile build_dir
	@$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

clean:
	rm -rf build
	rm tldr
