#ifndef LEDMATRIX_H
#define LEDMATRIX_H

#include "inc/tm4c123gh6pm.h"

/**
 * GPIO MAP:
 * R0 = PA2
 * B0 = PA3
 * G0 = PA4
 * R1 = PA5
 * B1 = PA6
 * G1 = PA7
 * DEMUX A = PB0
 * DEMUX B = PB1
 * DEMUX C = PB2
 * DEMUX D = PB3
 * SCLK = PB4
 * LATCH/STB = PB5
 * OE = PB6
 */

// Max row and cols
#define MAX_ROWS 32
#define MAX_COLS 32

// Port registers for all of the data pins (RGB0 and RGB1)
#define DATAPORT GPIO_PORTA_DATA_R
#define DATAPORT_DIR GPIO_PORTA_DIR_R
#define DATAPORT_DEN GPIO_PORTA_DEN_R
#define DATAPORT_8MA GPIO_PORTA_DR8R_R

// Port registers for all of the control pins (DEMUX, SCLK, OE, and LATCH)
#define CTRLPORT GPIO_PORTB_DATA_R
#define CTRLPORT_DIR GPIO_PORTB_DIR_R
#define CTRLPORT_DEN GPIO_PORTB_DEN_R
#define CTRLPORT_8MA GPIO_PORTB_DR4R_R

// Pin maskings for data pins
#define R0 0x4
#define B0 0x8
#define G0 0x10
#define R1 0x20
#define B1 0x40
#define G1 0x80

// The bit number of the data pin in the port (used for shifting values)
#define R0S 2
#define B0S 3
#define G0S 4
#define R1S 5
#define B1S 6
#define G1S 7

// Pin masking for control lines
#define DEMUXA 0x1
#define DEMUXB 0x2
#define DEMUXC 0x4
#define DEMUXD 0x8
#define SCLK 0x10
#define OE 0x20
#define LATCH 0x40

#define ALL_DATAPORT_PINS (R0 | G0 | B0 | R1 | G1 | B1)
#define ALL_CTRLPORT_PINS (DEMUXA | DEMUXB | DEMUXC | DEMUXD | SCLK | LATCH | OE)

// Bit field for holding color information
typedef struct Color_t
{
	uint8_t R;
	uint8_t G;
	uint8_t B;
} Color;

// A type used to create an array that stores 1-bit per pixel for the LED matrix
typedef uint32_t GridArray[MAX_ROWS];

// Convenience macros for getting, setting, and clearing bits in a GridArray
#define GET_GRIDARRAY_BIT(array,row,col) ((array[row] >> col) & 1)
#define SET_GRIDARRAY_BIT(array,row,col) ((array[row]) |= (1 << col))
#define CLEAR_GRIDARRAY_BIT(array,row,col) ((array[row]) &= ~(1 << col))

// Initializes timer and gpio ports needed to drive the LED matrix
void InitMatrixDriver(void);

// Sets the current drawing color
void SetColor(uint8_t r, uint8_t g, uint8_t b);

// Clears every pixel on the display
void ClearMatrix();

// Draws every pixel on the display to a certain color
void DrawSolidColor();

// Sets a line (straight along a row) to a certain color
void DrawRowLine(uint8_t rownum, uint8_t startcol, uint8_t length);

// Sets a line (straight along a column) to a certain color
void DrawColumnLine(uint8_t colnum, uint8_t startrow, uint8_t length);

// Sets a pixel to a certain color
void DrawPixel(uint8_t rownum, uint8_t colnum);

// Draws an array of bits (if bit is zero, dont display, if one, display color)
void DrawGridArray(GridArray array);

#endif
