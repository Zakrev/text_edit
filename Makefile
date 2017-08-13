LIBS := -lpthread
GCC_OPT := -Wall

all: clean terminal

PHONY: help all clean test_core configure core

configure:
	mkdir bin

help:
	cat doc/Make_help

clean:
	rm -R -f bin/*
	rm -f *.o

core:
	rm -f *.o
	cd src/core
	gcc ${GCC_OPT} -c src/core/*.c src/encoding/utf_8.c ${LIBS}
	ar cr bin/libcore.a *.o
	rm -f *.o

terminal: core
	gcc ${GCC_OPT} -o bin/main src/terminal/main.c src/terminal/term.c bin/libcore.a
	rm -f bin/libcore.a

test_core: core
	gcc ${GCC_OPT} -o bin/main src/test_core/*.c bin/libcore.a
	rm -f bin/libcore.a
