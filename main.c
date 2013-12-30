#include <stdio.h>	/* puts, fputs */
#include <stdlib.h>
#include <time.h>

#include "SDL.h"

#include "game.h"
#include "common.h"

direction_t scancode_to_direction(SDL_Scancode key);

void cleanup(void);

int main(void)
{
	puts("sdl_snake v2.0, by sigkill\n"
		"Steer with arrow keys or WASD.\n"
		"Space toggles pause. Pressing in any direction unpauses.\n"
		"Chase the red dots while avoiding walls and your tail.");
	
	SDL_Window *window;
	
	if(SDL_Init(SDL_INIT_VIDEO) == -1)
		die("Couldn't initialize SDL");
	atexit(cleanup);
	
	window = SDL_CreateWindow(
		"SDL Snake",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		FIELD_WIDTH, FIELD_HEIGHT,
		SDL_WINDOW_OPENGL);
	
	if(!window)
		die("Couldn't create window\n");

	srand(time(NULL));
	
	gamestate_t game;
	game_init(&game, window);
	
	SDL_Thread *thread = SDL_CreateThread(game_loop, "game", (void *)&game);
	if(!thread)
		die("Couldn't create thread\n");
	
	/* Loop forever */
	for(;;)
	{
		/* Block for an event */
		SDL_Event event;
		if(!SDL_WaitEvent(&event))
			die("Error processing window events\n");
		
		/* Handle event */
		switch(event.type)
		{
			case SDL_QUIT:
				SDL_DestroyWindow(window);
				return 0;	/* Cleanup will occur in atexit function */
			case SDL_KEYDOWN:
				game_lock(&game);
				if(event.key.keysym.scancode == SDL_SCANCODE_SPACE)
					game_toggle_pause(&game);
				else
					game_handle_direction_change(&game, scancode_to_direction(event.key.keysym.scancode));
				game_unlock(&game);
				break;
			case SDL_WINDOWEVENT:
				if(event.window.event == SDL_WINDOWEVENT_EXPOSED || event.window.event == SDL_WINDOWEVENT_SHOWN)
				{
					game_lock(&game);
					game.redraw = true;
					game_unlock(&game);
				}
				break;
		}
	}
	
	/* Unreachable */
	/* return 0; */
}

void cleanup(void)
{
	puts("Quitting...");
	SDL_Quit();
}

direction_t scancode_to_direction(SDL_Scancode key)
{
	switch(key)
	{
		case SDL_SCANCODE_A:
		case SDL_SCANCODE_LEFT:
			return DIRECTION_WEST;
		case SDL_SCANCODE_S:
		case SDL_SCANCODE_DOWN:
			return DIRECTION_SOUTH;
		case SDL_SCANCODE_D:
		case SDL_SCANCODE_RIGHT:
			return DIRECTION_EAST;
		case SDL_SCANCODE_W:
		case SDL_SCANCODE_UP:
			return DIRECTION_NORTH;
		default:
			return DIRECTION_NONE;
	}
}
