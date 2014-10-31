#include <stdint.h>
#include "pacman.h"
#include "LedMatrix.h"

// Pacman's current direction
static PacmanDir cur_pacman_dir = RIGHT;

// Pacman himself
static Character pacman = {1, 1, {127, 127, 0} };

// The 32x32 grid array that defines the walls of the level
GridArray level =
{
	0xFFFFFFFF,
	0x80000001,
	0x80000001,
	0x80000001,
	0x80000001,
	0x80000001,
	0x80000001,
	0x80000001,
	0x80000001,
	0x80000001,
	0x80000001,
	0x80000001,
	0x80000001,
	0x80000001,
	0x80FFFF01,
	0x80800101,
	0x80800101,
	0x80FFFF01,
	0x80000001,
	0x80000001,
	0x80000001,
	0x80000001,
	0x80000001,
	0x80000001,
	0x80000001,
	0x80000001,
	0x80000001,
	0x80000001,
	0x80000001,
	0x80000001,
	0x80000001,
	0xFFFFFFFF,
};

// The 32x32 grid that defines where the pellets will be
static GridArray pellets = {
	0x00000000,
	0x000FF000,
	0x000FF000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x000FF000,
	0x000FF000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x000FF000,
	0x000FF000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x000FF000,
	0x000FF000,
	0x00000000,
};

/**
 * @brief	Initializes the hardware needed to play the game.
 * 			This function must be called before the game can
 * 			run.
 *
 * @param	none
 *
 * @retval	none
 */
void InitGame(void)
{
	// Timer initialization
	TIMER1_CTL_R &= ~0x1;	// Disable timer
	TIMER1_CFG_R = 0;		// 32-bit timer
	TIMER1_TAMR_R |= 0x32;	// Set it to periodic mode, counting up, interrupt enabled
	TIMER1_TAILR_R = 20000000;	// count up to 12_000_000 before reloading
	TIMER1_TAMATCHR_R = 20000000;	// Trigger interrupt after 150ms has passed
	TIMER1_IMR_R |= 0x10;	// Enable timer A match interrupt

	// Enable Timer1A interrupt in NVIC
	NVIC_EN0_R |= 0x200000;

	// Set Timer1 to priority level 1
	NVIC_PRI5_R |= 0x2000;
}

/**
 * @brief	Receive interrupt for UART4. Grabs information
 * 			used to determine which direction pacman will move.
 *
 * @param	none
 *
 * @retval	none
 */
void UART4Int(void)
{
	//UART4_ICR_R |= 0x10;	// Clear interrupt flag in the UART
	NVIC_UNPEND1_R |= 0x10000000; // Clear interrupt flag in NVIC

	uint8_t cur_data = 0;

	// If there was a receive operation, handle it
	if(UART4_MIS_R & 0x10)
	{
		cur_data = UART4_DR_R;

		switch(cur_data)
		{
			case 'w':
				cur_pacman_dir = UP;
				break;
			case 'a':
				cur_pacman_dir = LEFT;
				break;
			case 'd':
				cur_pacman_dir = RIGHT;
				break;
			case 's':
				cur_pacman_dir = DOWN;
				break;
			default:
				break;
		}
	}
}

/**
 * @brief	Draws a character to the matrix
 *
 * @param	character Which character to draw
 *
 * @retval	none
 */
static void DrawCharacter(Character character)
{
	SetColor(character.color.R, character.color.G, character.color.B);
	DrawPixel(character.row, character.col);
}

// Perform movement and collision detection
/**
 * @brief	Perform movement and collision detection for pacman.
 *
 * @param	none
 *
 * @retval	none
 */
static void MovePacman(void)
{
	switch(cur_pacman_dir)
	{
		case LEFT:
			// if currently at edge of map, and are able to loop around, then do so
			if(pacman.col == 0 && GET_GRIDARRAY_BIT(level, pacman.row, (MAX_COLS - 1)) != 1)
				pacman.col = MAX_COLS - 1;
			// Otherwise, if there's no wall, move to the left
			else if (GET_GRIDARRAY_BIT(level, pacman.row, (pacman.col - 1)) != 1)
				pacman.col -= 1;

			break;

		case RIGHT:
			// if currently at edge of map, and are able to loop around, then do so
			if(pacman.col == MAX_COLS - 1 && GET_GRIDARRAY_BIT(level, pacman.row, 0) != 1)
				pacman.col = 0;
			// Otherwise, if there's no wall, move to the right
			else if (GET_GRIDARRAY_BIT(level, pacman.row, (pacman.col + 1)) != 1)
				pacman.col += 1;

			break;

		case UP:
			// if currently at edge of map, and are able to loop around, then do so
			if(pacman.row == 0 && GET_GRIDARRAY_BIT(level, MAX_ROWS - 1, pacman.col) != 1)
				pacman.row = MAX_ROWS - 1;
			// Otherwise, if there's no wall, move up
			else if (GET_GRIDARRAY_BIT(level, pacman.row - 1, pacman.col) != 1)
				pacman.row -= 1;

			break;

		case DOWN:
			// if currently at edge of map, and are able to loop around, then do so
			if(pacman.row == MAX_ROWS - 1 && GET_GRIDARRAY_BIT(level, 0, pacman.col) != 1)
				pacman.row = 0;
			// Otherwise, if there's no wall, move up
			else if (GET_GRIDARRAY_BIT(level, pacman.row + 1, pacman.col) != 1)
				pacman.row += 1;

			break;

		case NONE:
		default:
			break;
	}
}

/**
 * @brief	Timer1 interrupt, used as the main game loop
 *
 * @param 	none
 *
 * @retval	none
 */
void Timer1Int(void)
{
	// Clear interrupt flags
	TIMER1_ICR_R |= TIMER_ICR_TAMCINT; // Clear the interrupt flag
	NVIC_UNPEND0_R |= 0x200000;	// Clear interrupt pending flag in NVIC

	// Move pacman and perform collision detection
	MovePacman();

	// Clear out the bit in the pellet grid array where pacman is currently at
	CLEAR_GRIDARRAY_BIT(pellets, pacman.row, pacman.col);

	// Check if pacman won and do something

	// Clear out the screen
	ClearMatrix();

	// Draw level
	SetColor(0, 127, 127);
	DrawGridArray(level);

	// Draw pellets
	SetColor(127, 127, 127);
	DrawGridArray(pellets);

	// Draw Pacman
	DrawCharacter(pacman);
}
