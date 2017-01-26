#include "hw4_adc.h"

//******************************************************
// Global Variables provided by other files
// Do not modify
//******************************************************
extern GPIO_PORT *PortA ;
extern GPIO_PORT *PortB ;
extern GPIO_PORT *PortC ;
extern GPIO_PORT *PortD ;
extern GPIO_PORT *PortE ;
extern GPIO_PORT *PortF ;

//******************************************************
// Functions provided by ECE staff
// Do not modify
//******************************************************
extern void StartCritical(void);
extern void EndCritical(void);

volatile uint16_t RefreshRate = 0;
volatile bool AlertADC0 = false;

/****************************************************************************
 * Initialization code for the ADC
 * with SS3, Software trigger, 125KHz
 ****************************************************************************/
void initializeADC(void)
{
	uint32_t delay; // delay for initialization
	uint32_t i; // counter for delay in initialization
	GPIO_PORT *GpioPortE = (GPIO_PORT *)PortE;
	
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R4;
	delay = SYSCTL_RCGCGPIO_R;
	
	    // Set the direction as an input
	GpioPortE->Direction = 0x00;	

    // Set the alternate function
	GpioPortE->AlternateFunctionSelect = 0x0C;

    // Disable the Digital Enable
	GpioPortE->DigitalEnable = 0x00;

    // Enable Analog 
	GpioPortE->AnalogSelectMode = 0x0C;
	
	SYSCTL_RCGCADC_R |= SYSCTL_RCGCADC_R0;
	
	//Busy wait while ADC initializes
	i = 0;
	while(i < 1000)
	{
		i++;
	}	

  // Make sure to look at the #defines found in lm4f120h5qr.h.  
  // Line 3367 starts the bit mask definitions for the ADC.
  // (Step 1 from Data Sheet)
  // Disable Sample Sequencer #3 
  ADC0_ACTSS_R &= (~ADC_ACTSS_ASEN3);
    

    // ((Step 2 from Data Sheet)
    // Configure the sample sequencer so Sample Sequencer #3 (EM3) 
    // is triggered by the processor
		ADC0_EMUX_R = ADC_EMUX_EM3_PROCESSOR;
    
    // (Step 4 from Data Sheet)
    // Clear the Sample Input Select for Sample Sequencer #3
    ADC0_SSMUX3_R &= ~ADC_SSMUX3_MUX0_M;

    // (Step 4 from Data Sheet)
    // Configure the Sample Sequencer #3 control register.  Make sure to set 
    // the 1st Sample Interrupt Enable and the End of Sequence bits
    ADC0_SSCTL3_R = ADC_SSCTL0_END0 | ADC_SSCTL0_IE0;
    
    // Not shown in the data sheet.  See SAC register
    // Clear Averaging Bits
    ADC0_SAC_R &= ADC_SAC_AVG_OFF;

    // Not shown in the data sheet.  See SAC register
    // Average 64 samples
    ADC0_SAC_R  |= ADC_SAC_AVG_64X;
    
    // Do NOT enable the Sequencer after this.  This is done in GetADCval

}

//*****************************************************************************
// Get the ADC value of a given ADC Channel
//*****************************************************************************
uint32_t GetADCval(uint32_t Channel)
{
	
	uint32_t result; // ADC sample result

  ADC0_SSMUX3_R = Channel;      // Set the channel
  ADC0_ACTSS_R  |= ADC_ACTSS_ASEN3; // Enable SS3
  ADC0_PSSI_R = ADC_PSSI_SS3;     // Initiate SS3

	
	// Do we need to change this while loop to be a systick interrupt?
	
	
  while(0 == (ADC0_RIS_R & ADC_RIS_INR3)); // Wait for END of conversion
  result = ADC0_SSFIFO3_R & 0x0FFF;     // Read the 12-bit ADC result
  ADC0_ISC_R = ADC_ISC_IN3;         // Acknowledge completion

  return result;
}

//*****************************************************************************
// The refresh rate will go from 0-75Hz.
//*****************************************************************************
void updateRefreshRate(void)
{
	
	// get the value from the left potentiometer if alertADC0 is true
	if(AlertADC0){
		 RefreshRate = GetADCval(LEFT_POT)/41;
	}
	return;
	
}
