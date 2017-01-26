#include <stdbool.h>
#include "lm4f120h5qr.h"
#include "systick.h"

extern volatile bool AlertDebounce;  
extern volatile bool AlertRowUpdate;
extern volatile bool AlertADC0;
extern volatile uint16_t RefreshRate;
extern volatile uint8_t Row;

 /****************************************************************************
 * The SysTick Handler 
 ****************************************************************************/
void SYSTICKIntHandler(void)
{

		static uint16_t rowUpdateCount = 0; // Counter variable used to count until row refresh
		static uint32_t refreshRow = 0; // Calculated refresh for the rows

		rowUpdateCount++;

		if(RefreshRate != 0)
		{
			// 800 would be one second, so the higher the refresh rate,
			//	the faster this value will be reached
			refreshRow = 100 / RefreshRate; 
		}
		else// No divide by 0
		{
			refreshRow = 0;
		}
	
		// Updates the AlertADC0 if the potentiometer data has changed
		AlertADC0 = true;
	
		// Updates the AlertDebounce if any button data has changed
		if((GPIO_PORTA_DATA_R & 0x40) == 0 || (GPIO_PORTA_DATA_R & 0x80) == 0 || (GPIO_PORTD_DATA_R & 0x04) == 0 || (GPIO_PORTD_DATA_R & 0x08) == 0)
		{
			
			AlertDebounce = true;
		
		}
		else
		{
			AlertDebounce = false;
		}
		// Updates the AlertRowUpdate if any once the counter hits 
		//	the correct value based on the refresh rate
		if (rowUpdateCount >= refreshRow && refreshRow != 0)
		{
			Row++;
			AlertRowUpdate = true;
			rowUpdateCount = 0;
		}
		
		Row = Row % 8; // makes sure the row is within the correct range
		
    // clear the SysTick Timer Interrupt
		NVIC_ST_CTRL_R &= ~NVIC_ST_CTRL_COUNT;

}


/****************************************************************************
 * Initialize the SysTick timer to a given count.
 * Optionally turn on interrupts
 ****************************************************************************/
void initializeSysTick(uint32_t count, bool enableInterrupts)
{
	NVIC_ST_CTRL_R  = 0;
	NVIC_ST_RELOAD_R = count;
	NVIC_ST_CURRENT_R = 0;
	
	// If enableInterrupts is true, enable the timer with interrupts
  // else enable the timer without interrupts
  if(enableInterrupts){
		NVIC_ST_CTRL_R |= 0x7;
	}
	else{
		NVIC_ST_CTRL_R |= 0x5;
	}
}
