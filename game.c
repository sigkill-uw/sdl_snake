#define _GAME_C_

#include <stdlib.h>
#include <limits.h>

#include "SDL.h"
#include "common.h"
#include "game.h"

#define N_TILES_X		(FIELD_WIDTH / TILE_SIZE)
#define N_TILES_Y		(FIELD_HEIGHT / TILE_SIZE)

#define SNAKE_SPEED		1

#define SNAKE_GOAL_FEED	6

#define SNAKE_INITIAL_X	(N_TILES_X / 2)
#define SNAKE_INITIAL_Y	(N_TILES_Y / 2)

#define MAX_PLACEMENT_TRIALS	4096

#define FIELD_COLOR		0xA0A0A0
#define SNAKE_COLOR		0x2020A0
#define GOAL_COLOR		0xA02020

#define TILE_IS_DIRECTION(t)		((t) && abs(t) <= TILE_EAST)
#define TILES_ARE_ORTHOGONAL(m, n)	(((m) ^ (n)) & 0x01)
#define TILES_ARE_PARALLEL(m, n)	(!TILES_ARE_ORTHOGONAL((m), (n)))

#define FROM_HEX(h)		(((h) >> 16) & 0xff), (((h) >> 8) & 0xff), ((h) & 0xff), 0xff

void gamestate_init(gamestate_t *game, SDL_Window *window)
{
	game -> victory		= false;
	game -> game_over	= false;
	game -> paused		= false;
	game -> redraw_goal	= false;
	
	int i = FIELD_WIDTH;
	while(i --)
	{
		int j;
		for(j = 0; j < FIELD_HEIGHT; j ++)
			game -> map[i][j] = TILE_NONE;
	}
	
	game -> snake.head.x = game -> snake.tail.x = SNAKE_INITIAL_X;
	game -> snake.head.y = game -> snake.tail.y = SNAKE_INITIAL_Y;
	game -> snake.feed = 0;
	game -> map[SNAKE_INITIAL_X][SNAKE_INITIAL_Y] = TILE_START;
	
	do
	{
		game -> goal.x = rand() % N_TILES_X;
		game -> goal.y = rand() % N_TILES_Y;
	} while(game -> goal.x == SNAKE_INITIAL_X && game -> goal.y == SNAKE_INITIAL_Y);
	game -> map[game -> goal.x][game -> goal.y] = TILE_GOAL;
	
	game -> erase.x = game -> erase.y = UINT_MAX;
	
	game -> window = window;
	game -> renderer = SDL_CreateRenderer(window, -1, 0);
	
	if(!game -> renderer)
		die("Couldn't initialize SDL renderer");
}

void gamestate_steer(gamestate_t *game, tile_t direction)
{
	if(direction)
	{
		if(	game -> map[game -> snake.head.x][game -> snake.head.y] == TILE_START ||
			TILES_ARE_ORTHOGONAL(game -> map[game -> snake.head.x][game -> snake.head.y], direction))
		{
			game -> map[game -> snake.head.x][game -> snake.head.y] = direction;
		}
		
		game -> paused = false; 
	}
}

void gamestate_toggle_pause(gamestate_t *game)
{
	if(game -> map[game -> snake.head.x][game -> snake.head.y] != TILE_START)
		game -> paused = !game -> paused;
}

void gamestate_tick(gamestate_t *game)
{
	if(game -> map[game -> snake.head.x][game -> snake.head.y] == TILE_START || game -> paused)
		return;
	
	tile_t last = game -> map[game -> snake.head.x][game -> snake.head.y];
	
	switch(last)
	{
		case TILE_WEST:
			game -> snake.head.x --;
			break;
		case TILE_SOUTH:
			game -> snake.head.y ++;
			break;
		case TILE_NORTH:
			game -> snake.head.y --;
			break;
		case TILE_EAST:
			game -> snake.head.x ++;
			break;
	}
	
	if(game -> snake.head.x >= N_TILES_X || game -> snake.head.y >= N_TILES_Y)
	{
		game -> game_over = true;
	}
	else
	{
		if(game -> map[game -> snake.head.x][game -> snake.head.y] == TILE_GOAL)
		{
			game -> snake.feed += SNAKE_GOAL_FEED - 1;
			
			game -> map[game -> snake.head.x][game -> snake.head.y] = last;
			
			game -> redraw_goal = true;
			
			int i = 0;
			do
			{
				game -> goal.x = rand() % N_TILES_X;
				game -> goal.y = rand() % N_TILES_Y;
			} while(game -> map[game -> goal.x][game -> goal.y] && ++ i < MAX_PLACEMENT_TRIALS);
			
			if(i == MAX_PLACEMENT_TRIALS)
			{
				for(game -> goal.x = 0; game -> goal.x < N_TILES_X; game -> goal.x ++)
				{
					for(game -> goal.y = 0; game -> goal.y < N_TILES_Y && game -> map[game -> goal.x][game -> goal.y]; game -> goal.y ++);
					
					if(!game -> map[game -> goal.x][game -> goal.y])
						break;
				}
				
				if(game -> goal.x == N_TILES_X)
					game -> victory = true;
			}
			
			game -> map[game -> goal.x][game -> goal.y] = TILE_GOAL;
		}
		else if(game -> map[game -> snake.head.x][game -> snake.head.y])
		{
			game -> game_over = true;
		}
		else
		{
			game -> map[game -> snake.head.x][game -> snake.head.y] = last;
			
			if(game -> snake.feed)
			{
				game -> snake.feed --;
			}
			else
			{
				last = game -> map[game -> snake.tail.x][game -> snake.tail.y];
				
				game -> map[game -> snake.tail.x][game -> snake.tail.y] = TILE_NONE;
				game -> erase = game -> snake.tail;
				
				switch(last)
				{
					case TILE_WEST:
						game -> snake.tail.x --;
						break;
					case TILE_SOUTH:
						game -> snake.tail.y ++;
						break;
					case TILE_NORTH:
						game -> snake.tail.y --;
						break;
					case TILE_EAST:
						game -> snake.tail.x ++;
						break;
				}
			}
		}
	}
}

void gamestate_render(gamestate_t *game, bool redraw)
{
	SDL_Rect rectangle;
	rectangle.w = rectangle.h = TILE_SIZE - 2;
	
	if(redraw)
	{
		SDL_ClearError();
		
		SDL_SetRenderDrawColor(game -> renderer, FROM_HEX(FIELD_COLOR));
		SDL_RenderFillRect(game -> renderer, NULL);
		
		SDL_SetRenderDrawColor(game -> renderer, FROM_HEX(GOAL_COLOR));
		rectangle.x = 1 + game -> goal.x * TILE_SIZE;
		rectangle.y = 1 + game -> goal.y * TILE_SIZE;
		SDL_RenderFillRect(game -> renderer, &rectangle);
		
		SDL_SetRenderDrawColor(game -> renderer, FROM_HEX(SNAKE_COLOR));
		
		int i, j;
		for(i = 0; i < N_TILES_X; i ++)
			for(j = 0; j < N_TILES_Y; j ++)
				if(game -> map[i][j] && game -> map[i][j] != TILE_GOAL)
				{
					rectangle.x = 1 + i * TILE_SIZE;
					rectangle.y = 1 + j * TILE_SIZE;
					SDL_RenderFillRect(game -> renderer, &rectangle);
				}
		
		SDL_RenderPresent(game -> renderer);
		
		const char *error;
		if((error = SDL_GetError()) && *error)
			die("Error rendering");
	}
	else if(!game -> paused && game -> map[game -> snake.head.x][game -> snake.head.y] != TILE_START)
	{
		SDL_ClearError();
		
		rectangle.x = 1 + game -> snake.head.x * TILE_SIZE;
		rectangle.y = 1 + game -> snake.head.y * TILE_SIZE;
		SDL_SetRenderDrawColor(game -> renderer, FROM_HEX(SNAKE_COLOR));
		SDL_RenderFillRect(game -> renderer, &rectangle);
		
		if(game -> redraw_goal)
		{
			rectangle.x = 1 + game -> goal.x * TILE_SIZE;
			rectangle.y = 1 + game -> goal.y * TILE_SIZE;
			
			SDL_SetRenderDrawColor(game -> renderer, FROM_HEX(GOAL_COLOR));
			SDL_RenderFillRect(game -> renderer, &rectangle);
			
			game -> redraw_goal = false;
		}
		
		if(game -> erase.x != UINT_MAX)
		{
			rectangle.x = 1 + game -> erase.x * TILE_SIZE;
			rectangle.y = 1 + game -> erase.y * TILE_SIZE;
			SDL_SetRenderDrawColor(game -> renderer, FROM_HEX(FIELD_COLOR));
			SDL_RenderFillRect(game -> renderer, &rectangle);
			
			game -> erase.x = game -> erase.y = UINT_MAX;
		}

		SDL_RenderPresent(game -> renderer);
		
		const char *error;
		if((error = SDL_GetError()) && *error)
			die("Error rendering");
	}	
}
