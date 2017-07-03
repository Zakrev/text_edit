LIBS := -lpthread

PHONY: help all clean

help:
	cat doc/Makefile_help

all:
	mkdir -p bin
	gcc -Wall -o bin/main src/*.c $(LIBS)
	
clean:
	rm -R -f bin
