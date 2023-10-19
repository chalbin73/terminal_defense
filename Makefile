CFLAGS=-std=c99 -Wall -Werror -Wvla -ggdb -Isrc/
LDFLAGS=

SOURCES=
SOURCES+=common.c
SOURCES+=graphical.c
SOURCES+=tldr.c

.PHONY: all

all: tldr

help:
	@echo "TLDR; Makefile help"
	@echo "    make             Compiles and links the program"
	@echo "    make clean       Cleans object files and executable"
	@echo "    make help        Displays this help"

tldr: $(foreach source,$(SOURCES),build/$(patsubst %.c,%.o,$(source)))
	$(CC) $? -o tldr 

build_dir:
	@mkdir -p build
		
build/%.o: src/%.c | build_dir
	$(CC) -o $@ $(CFLAGS) -c $<

clean:
	rm -rf build
	rm tldr
