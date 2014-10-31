#ifndef UART_H_
#define UART_H_

#include <stdint.h>
#include "utility.h"

#define BAUD_RATE 9600	// Wanted baud rate
//#define IBRD (SYSCLK / (16 * BAUD_RATE))	// Integer baud rate divisor

// Initialize the UART and GPIO needed to use the UART
void InitUART(uint16_t IBRD);

// Send a byte of data over the UART
void UARTTransmit(uint8_t data);

#endif /* UART_H_ */
