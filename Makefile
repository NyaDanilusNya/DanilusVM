CC = clang
LIBS = -llua5.2 -lSDL2 -lm -ldl
CFLAGS = -Wall -Wextra -O2 -g
OUT = bin/main
IN = src/main.c src/dtexture.c src/event_queue.c src/utils.c src/sdl.c src/lua.c

all:
	$(CC) -o $(OUT) $(IN) $(CFLAGS) $(LIBS)

run:
	cd bin && ./main; cd ..
g:
	cd bin && gdb ./main; cd ..

