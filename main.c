#include <stdio.h>
#include <time.h>

#include "SDL.h"

#include "game.h"
#include "common.h"

tile_t scancode_to_direction(SDL_Scancode key);

void cleanup(void);

int main(void)
{
	puts("sdl_snake v3.0, by sigkill\n"
		"Steer with arrow keys or WASD.\n"
		"Space toggles pause. Pressing in any direction unpauses.\n"
		"Chase the red dots while avoiding walls and your tail.");
	
	SDL_Window *window;
	
	if(SDL_Init(SDL_INIT_VIDEO) == -1)
		die("Couldn't initialize SDL");
	
	window = SDL_CreateWindow(
		"SDL Snake",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		FIELD_WIDTH, FIELD_HEIGHT,
		SDL_WINDOW_OPENGL);
	
	if(!window)
		die("Couldn't create window");
	
	srand(time(NULL));
	
	gamestate_t game;
	gamestate_init(&game, window);
	
	unsigned int timer = SDL_GetTicks() + MS_PER_FRAME;
	for(;;)
	{
		SDL_Event event;
		tile_t new_direction = TILE_NONE;
		bool redraw = false;
		
		while(SDL_PollEvent(&event))
		{
			switch(event.type)
			{
				case SDL_QUIT:
					goto end;
				case SDL_KEYDOWN:
					if(event.key.keysym.scancode == SDL_SCANCODE_SPACE)
						gamestate_toggle_pause(&game);
					else
						new_direction = scancode_to_direction(event.key.keysym.scancode);
					break;
				case SDL_WINDOWEVENT:
					if(event.window.event == SDL_WINDOWEVENT_EXPOSED || event.window.event == SDL_WINDOWEVENT_SHOWN)
						redraw = true;
					break;
			}
		}
		
		while(SDL_GetTicks() < timer);
		timer += MS_PER_FRAME;
		
		gamestate_steer(&game, new_direction);
		gamestate_tick(&game);
		if(game.game_over || game.victory)
			break;
		gamestate_render(&game, redraw);
	}
	
	end:
	
	if(game.game_over)
	{
		puts("Game over - you lose!");
	}
	else if(game.victory)
	{
		puts("You win!");
	}
	else
	{
		puts("Exiting.");
	}
	
	SDL_DestroyWindow(window);
	SDL_Quit();
	
	return 0;
}

tile_t scancode_to_direction(SDL_Scancode key)
{
	switch(key)
	{
		case SDL_SCANCODE_A:
		case SDL_SCANCODE_LEFT:
			return TILE_WEST;
		case SDL_SCANCODE_S:
		case SDL_SCANCODE_DOWN:
			return TILE_SOUTH;
		case SDL_SCANCODE_D:
		case SDL_SCANCODE_RIGHT:
			return TILE_EAST;
		case SDL_SCANCODE_W:
		case SDL_SCANCODE_UP:
			return TILE_NORTH;
		default:
			return TILE_NONE;
	}
}
