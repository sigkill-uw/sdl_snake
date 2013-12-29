#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "SDL.h"

#define WINDOW_WIDTH	600
#define WINDOW_HEIGHT	400

#define TILE_SIZE		8

#define N_TILES_X		(WINDOW_WIDTH / TILE_SIZE)
#define N_TILES_Y		(WINDOW_HEIGHT / TILE_SIZE)

#define TICS_PER_GRAIN	(CLOCKS_PER_SEC / 60)

void die(char *string);

void place_goal(void);

void redraw_window(void);
void handle_keypress(SDL_Scancode key);

SDL_Window *window;
SDL_Surface *surface;
Uint32 white, black;

enum
{
	TILE_EMPTY,
	TILE_GOAL,
	TILE_LEFT,
	TILE_RIGHT,
	TILE_UP,
	TILE_DOWN,
	TILE_START
} map[N_TILES_X][N_TILES_Y] = {{TILE_EMPTY}};

int x = N_TILES_X / 2, y = N_TILES_Y / 2;
int tx = N_TILES_X / 2, ty = N_TILES_Y / 2;
int gx, gy;

int pause = 1;

int main(int argc, char **argv)
{
	puts("sdl_snake by sigkill");
	
	SDL_Init(SDL_INIT_VIDEO);
	
	window = SDL_CreateWindow(
		"SDL Snake",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		WINDOW_WIDTH, WINDOW_HEIGHT,
		SDL_WINDOW_OPENGL);
	
	if(!window)
		die("Could not create SDL window");
	
	surface = SDL_GetWindowSurface(window);
	white = SDL_MapRGB(surface -> format, 255, 255, 255);
	black = SDL_MapRGB(surface -> format, 0, 0, 0);
	
	map[N_TILES_X][N_TILES_Y] = TILE_START;
	
	srand(time(NULL));
	
	place_goal();
	
	clock_t timer = clock() + TICS_PER_GRAIN;
	for(;;)
	{
		SDL_Event e;
		while(SDL_PollEvent(&e))
		{
			int kflag = 0;
			
			switch(e.type)
			{
				case SDL_KEYDOWN:
					if(!kflag)
					{
						handle_keypress(e.key.keysym.scancode);
						kflag = 1;
					}
					break;
				case SDL_WINDOWEVENT:
					if(e.window.event == SDL_WINDOWEVENT_EXPOSED || e.window.event == SDL_WINDOWEVENT_SHOWN)
						redraw_window();
					break;
				case SDL_QUIT:
					goto quit;
					break;
			}
		}
			
		if(clock() >= timer)
		{
			timer += TICS_PER_GRAIN;
			
			if(!pause)
			{
				int nx = x, ny = y;
				
				switch(map[x][y])
				{
					case TILE_LEFT: nx --;
						break;
					case TILE_RIGHT: nx ++;
						break;
					case TILE_UP: ny --;
						break;
					case TILE_DOWN: ny ++;
						break;
				}
					
				if(nx < 0 || ny < 0 || nx > N_TILES_X - 1 || ny > N_TILES_Y - 1 ||
				 (map[nx][ny] != TILE_GOAL && map[nx][ny] != TILE_EMPTY))
					goto game_over;
				
				int goal = map[nx][ny] == TILE_GOAL;
				map[nx][ny] = map[x][y];
				x = nx;
				y = ny;
				
				if(goal)
				{
					place_goal();
				}
				else
				{
					nx = tx;
					ny = ty;
									
					switch(map[nx][ny])
					{
						case TILE_LEFT: nx --;
							break;
						case TILE_RIGHT: nx ++;
							break;
						case TILE_UP: ny --;
							break;
						case TILE_DOWN: ny ++;
							break;
					}
					
					map[tx][ty] = TILE_EMPTY;
					tx = nx;
					ty = ny;
				}
				
				redraw_window();
			}
		}
	}
	
	game_over:
	
	puts("Game over!");
	
	quit:
	
	SDL_DestroyWindow(window);
	
	SDL_Quit();
	
	return 0;
}

void die(char *string)
{
	fputs(string, stderr);
	exit(1);
}

void redraw_window(void)
{
	int i, j;
	
	SDL_Rect r = {0, 0, TILE_SIZE, TILE_SIZE};
	
	for(i = 0; i < N_TILES_X; r.x += TILE_SIZE, i ++)
		for(r.y = j = 0; j < N_TILES_Y; r.y += TILE_SIZE, j ++)
			SDL_FillRect(surface, &r, map[i][j] ? white : black);
	
	SDL_UpdateWindowSurface(window);
}

void handle_keypress(SDL_Scancode key)
{	
	switch(key)
	{
		case SDL_SCANCODE_LEFT:
			pause = 0;
			if(map[x][y] != TILE_RIGHT)
				map[x][y] = TILE_LEFT;
			break;
		case SDL_SCANCODE_RIGHT:
			pause = 0;
			if(map[x][y] != TILE_LEFT)
				map[x][y] = TILE_RIGHT;
			break;
		case SDL_SCANCODE_UP:
			pause = 0;
			if(map[x][y] != TILE_DOWN)
				map[x][y] = TILE_UP;
			break;
		case SDL_SCANCODE_DOWN:
			pause = 0;
			if(map[x][y] != TILE_UP)
				map[x][y] = TILE_DOWN;
			break;
	}
	
	if(key == SDL_SCANCODE_SPACE && map[x][y] != TILE_START)
		pause = !pause;
}

void place_goal(void)
{
	int gx, gy;
	
	do
	{
		gx = rand() % N_TILES_X;
		gy = rand() % N_TILES_Y;
	} while(map[gx][gy]);
	
	map[gx][gy] = TILE_GOAL;
}
