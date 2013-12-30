#ifndef _GAME_H_
#define _GAME_H_

#include <time.h>	/* clock_t */

#include "SDL.h"
#include "SDL_thread.h"

#include "common.h"

/* Size of the playing field (and also of the client area of the game window */
#define FIELD_WIDTH				600
#define FIELD_HEIGHT			400

/* "Halfwidth" for the snake. This makes certain calculations more simple. The width of the the snake is 2 * SNAKE_HALFWIDTH + 1 */
#define SNAKE_HALFWIDTH			6
#define SNAKE_WIDTH				(2 * SNAKE_HALFWIDTH + 1)	/* See above */

/* Speed of the snake in pixels per second */
#define SNAKE_MOVE_SPEED		800

/* Hex colors for the various game elements */
#define FIELD_COLOR				0x000000
#define SNAKE_COLOR				0xffffff
#define GOAL_COLOR				0xff0000

/* Pseudo-radius of goal dots. The diameter of each dot is, in actual fact, 2 * GOAL_RADIUS + 1 */
#define GOAL_RADIUS				10

/* Degree of growth for the capture of a single goal */
#define LENGTH_GROWTH			SNAKE_WIDTH

/* Converts a hex color to comma-seperated RGB values */
#define from_hex(h)				(((h) >> 16) & 0xff), (((h) >> 8) & 0xff), ((h) & 0xff)

/* Direction enum - arranged such that -d is the direction in the opposite direction of d */
typedef enum
{
	DIRECTION_WEST = -2,
	DIRECTION_SOUTH,
	DIRECTION_NONE,
	DIRECTION_NORTH,
	DIRECTION_EAST
} direction_t;

/* Returns an unsigned integer in the range [f, c] - inclusive! */
unsigned int random_integer(unsigned int f, unsigned int c);

/* Linked list structure for snake segments */
typedef struct segment_list
{	
	double x, y;
	
	/* We need a direction and a length for a given segment. Length is floating point to allow smooth movement */
	direction_t direction;
	double length;
	
	struct segment_list *next;
} segment_t;

/* Game state structure */
typedef struct
{
	/* We need to access the gamestate from multiple threads */
	SDL_sem *lock;
	
	/* Flags */
	bool paused	:1;	/* Is the game paused? */
	bool redraw	:1;	/* Do we require a full-window redraw? */	
	
	/* Timer to allow interpolation of movement */
	clock_t timer;
	
	/* SDL stuff for drawing the game */
	SDL_Window *window;
	SDL_Surface *surface;
	Uint32 field_color, snake_color, goal_color;
	
	/* Information about the snake */
	struct
	{		
		segment_t *head;
		segment_t *tail;
		
		double goal_feed;
	} snake;
	
	/* Position of the goal. This doesn't move continuously, so we can just use integers */
	struct
	{
		unsigned int x, y;
	} goal;
} gamestate_t;

/* Game loop appropriate for use with pthreads or SDL_CreateThread */
int game_loop(void *game);

/* Initalizes a gamestate_t structure */
void game_init(gamestate_t *game, SDL_Window *window);

/* Updates a running game and redraws the surface. May or may not redraw the entirety of the screen */
void game_update(gamestate_t *game);

/* Redraws the entire screen */
void game_redraw(gamestate_t *game);

/* Toggles the game pause state */
#define game_toggle_pause(g)	((g) -> paused = !(g) -> paused)

/* Handles a direction input */
void game_handle_direction_change(gamestate_t *game, direction_t direction);

/* Locks/unlocks the gamestate_t structure */
inline void game_lock(gamestate_t *game);
inline void game_unlock(gamestate_t *game);

#endif
