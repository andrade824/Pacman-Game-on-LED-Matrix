#ifndef PACMAN_H_
#define PACMAN_H_
#include "LedMatrix.h"

typedef struct Character_t
{
	uint8_t row;
	uint8_t col;
	Color color;
} Character;

// Pacman's direction of movement
typedef enum {NONE, LEFT, RIGHT, UP, DOWN} PacmanDir;

// Initializes the hardware needed to play the game
void InitGame(void);

extern GridArray level;

#endif /* PACMAN_H_ */
