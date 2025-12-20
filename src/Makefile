all: ../bin/chip8.out

../bin/chip8.out: chip8.c 
	gcc chip8.c -std=gnu99 -Werror -Wall -o ../bin/chip8.out -lSDL2

clean:
	rm -rf *~
	rm -rf ../bin/chip8.out
