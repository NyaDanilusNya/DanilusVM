CC = clang
LIBS = -llua5.2 -lSDL2 -lm -ldl
CFLAGS = -Wall -Wextra -O2 -g
OUT = bin/main
IN = src/main.c src/libdtexture.c

all:
	$(CC) -o $(OUT) $(IN) $(CFLAGS) $(LIBS)

run:
	cd bin && ./main; cd ..
g:
	cd bin && gdb ./main; cd ..

