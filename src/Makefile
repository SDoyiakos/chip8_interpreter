all: ../bin/chip8.out

../bin/chip8.out: chip8.c stack.c chip8.h stack.h 
	gcc chip8.c stack.c -std=gnu99 -Werror -Wall -o ../bin/chip8.out

clean:
	rm -rf *~
	rm -rf ../bin/chip8.out
