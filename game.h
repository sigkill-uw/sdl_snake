#ifndef _GAME_H_
#define _GAME_H_

#include <stdint.h>

#include "SDL.h"
#include "common.h"

#define FIELD_WIDTH		(TILE_SIZE * 51)
#define FIELD_HEIGHT	(TILE_SIZE * 41)

#define TILE_SIZE		15

#define FRAMES_PER_SEC	25
#define MS_PER_FRAME	(1000 / FRAMES_PER_SEC)

enum
{
	TILE_WEST = -2,
	TILE_SOUTH,
	TILE_NONE,
	TILE_NORTH,
	TILE_EAST,
	TILE_GOAL,
	TILE_START
};
typedef int8_t tile_t;

typedef struct
{
	unsigned int x, y;
} point_t;

typedef struct
{
	bool game_over		:1;
	bool paused			:1;
	bool redraw_goal	:1;
	
	struct
	{
		unsigned int points;
		unsigned int potential;
		unsigned int stride;
	} score;
		
	tile_t map[FIELD_WIDTH][FIELD_HEIGHT];
	
	SDL_Window *window;
	SDL_Renderer *renderer;
	
	point_t goal;
	
	struct
	{
		unsigned int length;
		point_t head, tail;
		unsigned int feed;
	} snake;
	
	point_t erase;
} gamestate_t;

void gamestate_init(gamestate_t *game, SDL_Window *window);
void gamestate_reset(gamestate_t *game);

void gamestate_steer(gamestate_t *game, tile_t direction);
void gamestate_toggle_pause(gamestate_t *game);

void gamestate_tick(gamestate_t *game);
void gamestate_render(gamestate_t *game, bool redraw);

#endif
