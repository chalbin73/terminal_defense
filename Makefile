CFLAGS=-std=c99 -Wall -Werror -Wvla -Isrc/ 
LDFLAGS=

RELEASE_FLAGS=-O3
DEBUG_FLAGS=-g -Og

SOURCES=
SOURCES+=common_and_ressources.c
SOURCES+=graphical.c
SOURCES+=tldr.c

.PHONY: all debug help

all: CFLAGS+=$(RELEASE_FLAGS)
all: LDFLAGS+=-s $(RELEASE_FLAGS)
all: tldr

debug: CFLAGS+=$(DEBUG_FLAGS)
debug: LDFLAGS+=$(DEBUG_FLAGS)
debug: tldr

help:
	@echo "TLDR; Makefile help"
	@echo "    make             Compiles and links the program (stripped)"
	@echo "    make debug       Compiles and links the program with debugging flags"
	@echo "    make clean       Cleans object files and executable"
	@echo "    make help        Displays this help"

tldr: $(foreach source,$(SOURCES),build/$(patsubst %.c,%.o,$(source)))
	$(CC) $^ $(LDFLAGS) -o tldr 

build_dir:
	@mkdir -p build
		
build/%.o: src/%.c | build_dir
	$(CC) -o $@ $(CFLAGS) -c $<

clean:
	rm -rf build
	rm tldr
