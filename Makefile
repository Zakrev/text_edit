LIBS := -lpthread

all:
	mkdir -p bin
	gcc -Wall -o bin/main src/*.c $(LIBS)

PHONY: help all clean

help:
	cat doc/Makefile_help

	
clean:
	rm -R -f bin
