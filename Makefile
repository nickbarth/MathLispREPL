calc: FORCE
	gcc -o calc calc.c -g
debug:
	sudo gdb calc
test:
	valgrind --leak-check=yes --track-origins=yes ./calc
clean:
	rm -rf calc calc.dSYM
.PHONY: FORCE
FORCE:
default: clean calc
