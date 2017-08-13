terminal:
	gcc -Wall -o main main.c term.c ../encoding/utf_8.c

.PHONY: terminal pinput

pinput:
	rm -f pinput
	gcc -Wall -o pinput parse_input.c ../encoding/utf_8.c