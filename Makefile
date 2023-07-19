CC = clang
LIBS = -llua5.2 -lSDL2 -lm -ldl
CFLAGS = -Wall -Wextra -O2 -g
OUT = bin/main
IN = src/main.c src/dtexture.c src/event_queue.c src/utils.c src/sdl.c src/lua.c src/config.c src/fdcontrol.c

all:
	$(CC) -o $(OUT) $(IN) $(CFLAGS) $(LIBS)

run:
	cd bin && ./main; cd ..

g:
	$(CC) -o $(OUT) $(IN) -Og -ggdb $(LIBS)
	cd bin && gdb -q ./main; cd ..

v:
	$(CC) -o $(OUT) $(IN) -O0 -g $(LIBS)
	cd bin && valgrind ./main; cd ..
