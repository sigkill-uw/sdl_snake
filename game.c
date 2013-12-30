#define _GAME_C_

#include <stdlib.h>	/* malloc, rand, srand */
#include <time.h>	/* time, clock */

#include "SDL.h"
#include "SDL_thread.h"

#include "common.h"

#include "game.h"

/* Game loop */
int game_loop(void *game)
{
	/* I dunno if there's a better way of doing this. It's such a simple game that the line between rendering and logic is pretty fine. */
	for(;;)
		game_update((gamestate_t *)game);
}

void game_place_goal(gamestate_t *game)
{
	game -> goal.x = random_integer(GOAL_RADIUS, FIELD_WIDTH - GOAL_RADIUS - 1);
	game -> goal.y = random_integer(GOAL_RADIUS, FIELD_HEIGHT - GOAL_RADIUS - 1);
}

void game_init(gamestate_t *game, SDL_Window *window)
{
	game -> lock = SDL_CreateSemaphore(1);
	if(!game -> lock)
		die("Couldn't create lock semaphore\n");
	
	/* Game isn't paused */
	game -> paused = false;
	
	/* Set up the SDL stuff */
	game -> window = window;
	game -> surface = SDL_GetWindowSurface(window);
	if(!game -> surface)
		die("Couldn't grab window surface");
	
	/* Ostensibly these don't cause errors */
	game -> field_color = SDL_MapRGB(game -> surface -> format, from_hex(FIELD_COLOR));
	game -> snake_color = SDL_MapRGB(game -> surface -> format, from_hex(SNAKE_COLOR));
	game -> goal_color = SDL_MapRGB(game -> surface -> format, from_hex(GOAL_COLOR));
	
	game -> snake.head = (segment_t *)malloc(sizeof(segment_t));
	if(!game -> snake.head)
		die("Couldn't allocate segment");
	game -> snake.tail = game -> snake.head;
	
	game -> snake.head -> x = (double)FIELD_WIDTH / 2;
	game -> snake.head -> y = (double)FIELD_HEIGHT / 2;
	game -> snake.head -> direction = DIRECTION_NONE;
	game -> snake.head -> length = (double)0;
	game -> snake.head -> next = NULL;
	
	/* Place the goal */
	game_place_goal(game);
	
	/* Indicate the the snake doesn't need extending */
	game -> snake.goal_feed = (double)0;
}

void game_redraw(gamestate_t *game)
{
	/* Rather than errorchecking each call individually, we'll just do a single check at the end of the function */
	SDL_ClearError();
	
	/* Clear the screen */
	SDL_FillRect(game -> surface, NULL, game -> field_color);
	
	SDL_Rect r;
	
	/* Fill the snake head */
	r.x = (int)game -> snake.head -> x - SNAKE_HALFWIDTH;
	r.y = (int)game -> snake.head -> y - SNAKE_HALFWIDTH;
	r.w = r.h = SNAKE_WIDTH;
	SDL_FillRect(game -> surface, &r, game -> snake_color);
	
	/* Now that the head has been isolated and filled, we don't have to fill the entirety of every other segment */
	
	/* Iterate through the list of segments, filling the un-filled portions */
	segment_t *segment;
	for(segment = game -> snake.head; segment; segment = segment -> next)
	{
		/* This code is a little impenetrable, but I'm pretty sure it works */
		switch(segment -> direction)
		{
			case DIRECTION_WEST:
				r.x = (int)segment -> x - (int)segment -> length - SNAKE_HALFWIDTH;
				r.y = (int)segment -> y - SNAKE_HALFWIDTH;
				r.w = (unsigned int)segment -> length;
				r.h = SNAKE_WIDTH;
				break;
			case DIRECTION_SOUTH:
				r.x = (int)segment -> x - SNAKE_HALFWIDTH;
				r.y = (int)segment -> y + SNAKE_HALFWIDTH;
				r.w = SNAKE_WIDTH;
				r.h = (unsigned int)segment -> length;
				break;
			case DIRECTION_EAST:
				r.x = (int)segment -> x + SNAKE_HALFWIDTH;
				r.y = (int)segment -> y - SNAKE_HALFWIDTH;
				r.w = (unsigned int)segment -> length;
				r.h = SNAKE_WIDTH;
				break;
			case DIRECTION_NORTH:
				r.x = (int)segment -> x - SNAKE_HALFWIDTH;
				r.y = (int)segment -> y - (int)segment -> length - SNAKE_HALFWIDTH;
				r.w = SNAKE_WIDTH;
				r.h = (unsigned int)segment -> length;
				break;
		}
		
		/* Draw the current segment */
		SDL_FillRect(game -> surface, &r, game -> snake_color);
	}
	
	/* Update the window */
	SDL_UpdateWindowSurface(game -> window);
	
	/* Check for errors */
	const char *err = SDL_GetError();
	if(err && err[0])
		die(err);
}

void game_update(gamestate_t *game)
{
	game_lock(game);
	
	if(game -> redraw)
	{
		game -> redraw = false;
		game_redraw(game);
	}
	
	if(!game -> paused && game -> snake.head -> direction != DIRECTION_NONE)
	{
		clock_t time = clock();
		double delta = (double)(time - game -> timer) / CLOCKS_PER_SEC * SNAKE_MOVE_SPEED;
		game -> timer = time;
		
		switch(-game -> snake.head -> direction)
		{
			case DIRECTION_WEST:
				game -> snake.head -> x -= delta;
				break;
			case DIRECTION_SOUTH:
				game -> snake.head -> y += delta;
				break;
			case DIRECTION_EAST:
				game -> snake.head -> x += delta;
				break;
			case DIRECTION_NORTH:
				game -> snake.head -> y -= delta;
				break;
		}
		
		game -> snake.head -> length += delta;
		game -> snake.tail -> length -= delta;
		
		game_redraw(game);
	}
	
	game_unlock(game);
}

void game_handle_direction_change(gamestate_t *game, direction_t direction)
{
	if(direction == DIRECTION_NONE)
		return;
	
	if(game -> snake.head -> direction == DIRECTION_NONE)
	{
		game -> snake.head -> direction = -direction;
		game -> timer = clock();
	}
	else
	{
		/* We can't allow the player to turn too sharply */
		//if(game -> snake.head -> length < SNAKE_WIDTH)
		//	return;
		
		if(game -> snake.head -> direction != direction)
		{
			segment_t *new_segment = (segment_t *)malloc(sizeof(segment_t));
			if(!new_segment)
				die("Fatal: unable to allocate new segment\n");
			
			new_segment -> x = game -> snake.head -> x;
			new_segment -> y = game -> snake.head -> y;
			
			new_segment -> direction = -direction;
			new_segment -> length = 0;
			
			new_segment -> next = game -> snake.head;
			game -> snake.head = new_segment;
		}
	}
}

inline void game_lock(gamestate_t *game)
{
	if(SDL_SemWait(game -> lock))
		die("Couldn't lock semaphore");
}

inline void game_unlock(gamestate_t *game)
{
	if(SDL_SemPost(game -> lock))
		die("Couldn't unlock semaphore");
}

unsigned int random_integer(unsigned int f, unsigned int c)
{
	return (unsigned int)(f + rand() / (RAND_MAX / (f - c + 1.0) + 1.0));
}
