all: sdl_snake

sdl_snake: main.c
	gcc -o sdl_snake `sdl2-config --cflags --libs` main.c
