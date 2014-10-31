#ifndef CONFIG_H_
#define CONFIG_H_

// System clock (MHz)
#define SYSCLK 80000000

// Used in delay calculations
#define ST_US (SYSCLK / 1000000)
#define ST_MS (SYSCLK / 1000)

// Macro to set the system clock to the defined SYSCLK
#define INITIALIZE_PLL() InitPLL((400000000 / SYSCLK) - 1)

// Initializes the PLL
void InitPLL(uint8_t div);

// Delays using the SysTick timer
void SysTickDelay(uint32_t delay);

#endif /* CONFIG_H_ */
