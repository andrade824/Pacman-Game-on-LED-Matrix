#include <stdint.h>
#include <string.h>
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_timer.h"
#include "inc/hw_gpio.h"
#include "LedMatrix.h"

// Global display variables (initialized to zero thanks to C standard!)
static uint8_t cur_row;	// Current row
static Color cur_draw_color;	// Current color to draw with
static Color matrix[MAX_ROWS][MAX_COLS];	// The framebuffer for the display

// Variables needed to perform binary coded modulation (BCM)
static uint8_t cur_bcm_cycle;	// 0-7, which cycle we're currently on

// Correct demux values based on the current row
static const uint8_t demux_vals[] = { 0, 1, 4, 5, 2, 3, 6, 7, 8, 9, 12, 13, 10, 11, 14, 15 };

// How long each bcm cycle should be (in terms of SYSCLK cycles)
static const uint32_t bcm_length[] = {
		240,
		480,
		960,
		1920,
		3840,
		7680,
		15360,
		30720
 };

// Maximum number of binary coded modulation cycles
static const uint8_t MAX_BCM = 6;

/**
 * @brief	Function that initializes the timer and GPIO
 * 			ports needed to drive the LED Matrix.
 *
 * 			Port and pin declarations are in "LedMatrix.h"
 *
 * 			You still need to enable the timer and clocks outside of this function.
 *
 * @param none
 *
 * @retval none
 */
void InitMatrixDriver(void)
{
	// Timer initialization
	TIMER0_CTL_R &= ~0x1;	// Disable timer
	TIMER0_CFG_R = 0;		// 32-bit timer
	TIMER0_TAMR_R |= 0x32;	// Set it to periodic mode, counting up, interrupt enabled
	TIMER0_TAILR_R = 8000;	// count up to 1_333_333 before reloading
	TIMER0_TAMATCHR_R = 8000;	// Trigger interrupt at 60Hz (0.01667s)
	TIMER0_IMR_R |= 0x10;	// Enable timer A match interrupt

	// Enable Timer0A interrupt in NVIC
	NVIC_EN0_R |= 0x80000;

	// Set DATAPORT and CTRLPORT pins as outputs
	DATAPORT_DIR |= ALL_DATAPORT_PINS;
	CTRLPORT_DIR |= ALL_CTRLPORT_PINS;

	// Set DATAPORT and CTRLPORT pins to have 8mA drive strength (probably don't need this but whatever)
	DATAPORT_8MA |= ALL_DATAPORT_PINS;
	CTRLPORT_8MA |= ALL_CTRLPORT_PINS;

	// Initializing DATAPORT and CTRLPORT pins as low
	DATAPORT &= ~ALL_DATAPORT_PINS;
	CTRLPORT &= ~ALL_CTRLPORT_PINS;

	// Enable DATAPORT and CTRLPORT pins as digital pins
	DATAPORT_DEN |= ALL_DATAPORT_PINS;
	CTRLPORT_DEN |= ALL_CTRLPORT_PINS;
}

/**
* @brief	Interrupt for Timer0 Subtimer A when it reaches its match value
*
* @param 	none
*
* @retval 	none
*/
void Timer0AInt(void)
{
	uint8_t i = 0;
	int cur_row_other = cur_row + 16;	// The other row that needs to be displayed

	// Disable timer
	TIMER0_CTL_R &= ~0x1;	// Disable timer

	// Clear interrupt flags
	TIMER0_ICR_R |= TIMER_ICR_TAMCINT; // Clear the interrupt flag
	NVIC_UNPEND0_R |= 0x80000;			// Clear interrupt pending flag in NVIC

	// Set Demux pins based off of row, and set OE high (turn off display)
	CTRLPORT |= OE;
	CTRLPORT &= ~(DEMUXA | DEMUXB | DEMUXC | DEMUXD | SCLK | LATCH);
	CTRLPORT |= demux_vals[cur_row] & 0xF;
	// Latch is currently low because pins were cleared

	// Clear dataport values
	DATAPORT &= ~ALL_DATAPORT_PINS;

	for(i = 0; i < 32; i++)
	{
		DATAPORT = 	(((matrix[cur_row][i].R >> cur_bcm_cycle) & 1) << R0S) |
					(((matrix[cur_row][i].G >> cur_bcm_cycle) & 1) << G0S) |
					(((matrix[cur_row][i].B >> cur_bcm_cycle) & 1) << B0S) |
					(((matrix[cur_row_other][i].R >> cur_bcm_cycle) & 1) << R1S) |
					(((matrix[cur_row_other][i].G >> cur_bcm_cycle) & 1) << G1S) |
					(((matrix[cur_row_other][i].B >> cur_bcm_cycle) & 1) << B1S);

		CTRLPORT |= SCLK;
		CTRLPORT &= ~SCLK;
	}

	// Strobe the latch signal
	CTRLPORT |= LATCH;
	CTRLPORT &= ~LATCH;

	// Clear the OE, aka, turn these two rows on
	CTRLPORT &= ~OE;

	// Set the period for the next binary coded modulation cycle
	TIMER0_TAV_R = 0;
	TIMER0_TAILR_R = bcm_length[cur_bcm_cycle];
	TIMER0_TAMATCHR_R = bcm_length[cur_bcm_cycle];

	// Enable timer
	TIMER0_CTL_R |= 0x1;

	// Increment binary coded modulation cycle, and if at the end, proceed to drawing the next row
	if(cur_bcm_cycle >= MAX_BCM)
	{
		if(cur_row == 15)
			cur_row = 0;
		else
			cur_row++;

		cur_bcm_cycle = 0;
	}
	else
		cur_bcm_cycle++;
}

/**
 * @brief	Sets the current drawing color
 *
 * @param	r The red component of the drawing color
 * @param	g The green component of the drawing color
 * @param	b The blue component of the drawing color
 *
 * @retval	none
 */
void SetColor(uint8_t r, uint8_t g, uint8_t b)
{
	Color new_color = {r, g, b};
	cur_draw_color = new_color;
}

/**
 * @brief	Clears every pixel on the display
 *
 * @param 	none
 *
 * @retval 	none
 */
void ClearMatrix()
{
	memset(matrix, 0, sizeof(matrix));
}

/**
 * @brief	Draws a solid color to every pixel on the display
 *
 * 			If you want to clear (turn off) every pixel,
 * 			use the ClearMatrix() function, it's more optimized
 * 			for clearing every pixel.
 *
 * 	@param	none
 *
 * 	@retval none
 */
void DrawSolidColor()
{
	int row = 0, col = 0;

	for (; row < MAX_ROWS; ++row)
	{
		for (col = 0; col < MAX_COLS; ++col)
			matrix[row][col] = cur_draw_color;
	}
}

/**
 * @brief	Sets a line (straight along a row) to a certain color
 *
 * @param 	rownum 	The row number (0 to MAX_ROWS - 1). If this number is above
 * 					MAX_ROWS - 1, the result is undefined.
 *
 * @param	startcol The column number (0 to MAX_COLS - 1) to start at. If this
 * 					 number is above MAX_COLS - 1, the result is undefined
 *
 * @param	length The number of pixels to set after startcol. If this number
 * 				   is above MAX_COLS - startcol - 1, the result is undefined
 *
 * @retval none
 */
void DrawRowLine(uint8_t rownum, uint8_t startcol, uint8_t length)
{
	int curcol = 0;

	for(; curcol < length; curcol++)
	{
		matrix[rownum][startcol + curcol] = cur_draw_color;
	}
}

/**
 * @brief	Sets a line (straight along a column) to a certain color
 *
 * @param 	colnum 		The column number (0 to MAX_COLS - 1). If this number is above
 * 						MAX_COLS - 1, the result is undefined.
 *
 * @param	startrow 	The row number (0 to MAX_ROWS - 1) to start at. If this
 * 					 	number is above MAX_ROWS - 1, the result is undefined
 *
 * @param	length 		The number of pixels to set after startrow. If this number
 * 				   		is above MAX_ROWS - startrow - 1, the result is undefined
 *
 * @retval none
 */
void DrawColumnLine(uint8_t colnum, uint8_t startrow, uint8_t length)
{
	int currow = 0;

	for(; currow < length; currow++)
	{
		matrix[startrow + currow][colnum] = cur_draw_color;
	}
}

/**
 * @brief	Sets a pixel to the passed in color
 *
 * @param	color The color to change the pixel to
 *
 * @param	rownum The row number of the pixel
 *
 * @param	colnum The column number of the pixel
 *
 * @retval	none
 */
void DrawPixel(uint8_t rownum, uint8_t colnum)
{
	matrix[rownum][colnum] = cur_draw_color;
}

/**
 *  @brief	Draws an array of bits (if bit is zero, dont display, if one, display color)
 *
 *  @param 	array The 32bit by 32bit array to draw from
 *  @param 	color The color to make each "one" bit in the array
 *
 *  @retval none
 */
void DrawGridArray(GridArray array)
{
	uint8_t row, col;

	for(row = 0; row < 32; ++row)
	{
		for(col = 0; col < 32; ++col)
		{
			if(GET_GRIDARRAY_BIT(array, row, col) == 1)
				matrix[row][col] = cur_draw_color;
		}
	}
}
