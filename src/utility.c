#include <stdint.h>
#include "inc/tm4c123gh6pm.h"
#include "utility.h"

/**
 * @brief 	Used to enable the PLL and set the system clock.
 * 			Use Table 5-6 (page 224) in datasheet to calculate
 * 			the "div" value (assume that SYSDIV2LSB is the LSB of
 * 			the value you pass in). E.g., a div of "4" would be an
 * 			80MHz clock (a div of 4 being equivalent to a SYSDIV2 of
 * 			0x02 plus a SYSDIV2LSB of 0, for 0b100 = 80MHz).
 *
 * @param 	div The divider value for the PLL
 *
 * @retval 	none
 */
void InitPLL(uint8_t div)
{
	SYSCTL_RCC2_R |= 0x80000000;	// Use RCC2 bit
	SYSCTL_RCC2_R |= 0x00000800;	// BYPASS2, PLL Bypass
	SYSCTL_RCC_R = (SYSCTL_RCC_R & ~0x7C0) + 0x540;	// Use 16MHz crystal
	SYSCTL_RCC2_R &= ~0x70;			// Configure for MOSC
	SYSCTL_RCC2_R &= ~0x2000;		// Activate PLL by clearing PWRDN
	SYSCTL_RCC2_R |= 0x40000000;	// Use 400MHz PLL
	SYSCTL_RCC2_R = (SYSCTL_RCC2_R & ~0x1FC00000) + (div << 22);

	// Wait for the PLL to lock by polling PLLRIS bit
	while((SYSCTL_RIS_R & 0x40) == 0);

	// Enable the PLL by clearing BYPASS
	SYSCTL_RCC2_R &= ~0x800;
}

/**
 * Creates a busy-wait loop delay using the 24-bit SysTick timer.
 * The timer increments one count every system clock cycle.
 *
 * @param delay The 24-bit value to use as the timer "reload" value.
 *
 * 				Remember, the maximum delay you can have is
 * 					(2^24 - 1) * (1 / SYSCLK)
 *
 * 				So, the faster the clock, the smaller your max delay.
 *
 * @retval none
 */
void SysTickDelay(uint32_t delay)
{
	NVIC_ST_RELOAD_R = delay - 1;
	NVIC_ST_CURRENT_R = 0;	// Clear the current register and COUNT bit
	NVIC_ST_CTRL_R = 5;		// Turn on timer

	// Wait for timer to loop around
	while((NVIC_ST_CTRL_R & 0x10000) == 0);

	NVIC_ST_CTRL_R = 0;		// Turn off timer
}
