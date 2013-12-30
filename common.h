#ifndef _COMMON_H_
#define _COMMON_H_

/* bool enum */
typedef enum
{
	false,
	true
} bool;

/* Quit with error message */
void die(const char *message);

#endif
