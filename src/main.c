#include <stdint.h>
#include <string.h>
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_timer.h"
#include "inc/hw_gpio.h"
#include "utility.h"
#include "LedMatrix.h"
#include "UART.h"
#include "Pacman.h"

//const char msg[] = "Fuck NES Controllers! (From TivaC)";

int main(void)
{
	// Enable clocks
	SYSCTL_RCGCTIMER_R = 0x3;	// Enable clock for timers 0 and 1
	SYSCTL_RCGCGPIO_R |= 0x00000007;	// Activate clock for Port A, B, and C
	SYSCTL_RCGCUART_R |= SYSCTL_RCGCUART_R4;	// Activate clock for UART 4

	// Initialize PLL to give us an 80MHz SYSCLK
	InitPLL(4);

	// Initialize hardware to drive the led matrix
	InitMatrixDriver();

	// Initialize the hardware needed to play the game
	InitGame();

	// Initialize the UART
	InitUART(521);

	// Enable timer that drives the LED matrix
	TIMER0_CTL_R |= 0x1;

	// Enable timer that drives the main game loop
	TIMER1_CTL_R |= 0x1;

	//SetColor(0, 127, 127);
	//DrawGridArray(level);

	// Main While Loop
	while(1)
	{
		/*for(i = 0; i < strlen(msg); i++)
			UARTTransmit(msg[i]);

		UARTTransmit('\n');*/


	}
}
