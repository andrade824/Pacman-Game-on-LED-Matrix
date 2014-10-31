#include <stdint.h>
#include "inc/tm4c123gh6pm.h"

/**
* @brief	Initialize the hardware needed to use the UART
*
* @param	IBRD The integer baud rate divisor to generate the baudrate from.
*				 The equation for calculating this value is as follows:
*					IBRD = SYSCLK / (16 * BAUD_RATE)
*
* @retval	none
*/
void InitUART(uint16_t IBRD)
{
	// Set PC4 and PC5 to their alternate function (UART4) and enable them
	GPIO_PORTC_AFSEL_R |= 0x30;
	GPIO_PORTC_PCTL_R |= 0x110000;
	GPIO_PORTC_DEN_R |= 0x30;

	// Disable the UART
	UART4_CTL_R &= ~1;

	// Set the baud rate
	UART4_IBRD_R = IBRD;

	// 8 data bits, one stop bit, no parity
	UART4_LCRH_R |= 0x60;

	UART4_IM_R |= 0x10;		// Enable interrupt in UART for receive operations
	NVIC_EN1_R |= 0x10000000; 	// enable interrupt in NVIC
	NVIC_PRI15_R |= 0x40;	// Set UART4 interrupt to priority level 2

	// Enable the receiver, transmitter and the UART as a whole
	UART4_CTL_R |= 0x101;
}

/**
* @brief 	Send a byte of data over the UART
*
* @param	data The 8 bits of data you want to send
*
* @retval	none
*/
void UARTTransmit(uint8_t data)
{
	int uart_enabled = (UART4_CTL_R & 1);

	// Wait until the UART is done transmitting
	while(uart_enabled && (UART4_FR_R & 0x8));

	// Send the data
	UART4_DR_R = data;
}
