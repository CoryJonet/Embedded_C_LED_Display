#include "hw4_gpio.h"
#include "led_chars.h"
#include "UART.h"

#define OUTPUT_ENABLE_B 0xEF
#define ENABLES_B 0x0F
#define nROW_0 ~(1 << 0)
#define ROW_EN (1 << 7)
#define ENABLES_OFF 0x00

#define LATCH_ALL_ON_M     0xF0
#define LATCH_ALL_OFF_M    0x00
#define LATCH_OE_B_M    ~(1 << 4)
//******************************************************
// Global Variables provided by other files
// Do not modify
//******************************************************
extern GPIO_PORT *PortA;
extern GPIO_PORT *PortB;
extern GPIO_PORT *PortC;
extern GPIO_PORT *PortD;
extern GPIO_PORT *PortE;
extern GPIO_PORT *PortF;

//******************************************************
// Functions provided by ECE staff
// Do not modify
//******************************************************
extern void uartTxPoll(uint32_t base, char *data);
extern void StartCritical(void);
extern void EndCritical(void);

extern volatile uint16_t RefreshRate;

volatile bool AlertDebounce;
volatile bool AlertRowUpdate;
char Color = 'R';
volatile uint8_t Row = 0;

volatile uint32_t countSw300;
volatile uint32_t countSw301;
volatile uint32_t countSw302;
volatile uint32_t countSw303;

volatile bool hasBeenPressed = false;

//*****************************************************************************
// The ISR sets AlertDebounce to true if the buttons should be examined.  
// If AlertDebounce is false, simply return
//
// If the ISR indicates that the buttons should be examined, the routine will
// debounces SW300-303.  If a button press is detected, it will print out a 
// message indicating with button was pressed.  
//
// When a button is pressed, update set Color to the appropriate value
//      SW300 – Display Off
//      SW301 – Refresh rate displayed in RED
//      SW302 – Refresh rate displayed in BLUE
//      SW303 – Refresh rate displayed in GREEN

//*****************************************************************************
void examineButtons(void)
{
	static int debounce_timer = 1440;// set close to the 14.4 we need
	// Interupt the debounce at 1440 reload
	
	// return if no need to be examined
	if(!AlertDebounce){
		hasBeenPressed = false;
		return;
	}
	
	//Check Switch 300
	if((PortA->Data & 0x40) > 0 || hasBeenPressed){
		countSw300 = 0;
	}
	else{
		countSw300++;
	}
	
	//Check switch 301
	if((PortA->Data & 0x80) > 0 || hasBeenPressed){
		countSw301 = 0;
	}
	else{
		countSw301++;
	}
	
	//Check switch 302
	if((PortD->Data & 0x04) > 0 || hasBeenPressed){
		countSw302 = 0;
	}
	else{
		countSw302++;
	}
	
	//Check switch 303
	if((PortD->Data & 0x08) > 0 || hasBeenPressed){
		countSw303 = 0;
	}
	else{
		countSw303++;
	}
	
	//Display off
	if(countSw300 == debounce_timer) {
		//Turn off display
		
		Color = 'Q'; 
//		PortF->Data &= (0<<4);
		countSw300 = 0;
		hasBeenPressed = true;
		uartTxPoll(UART0,"SW300 was pressed (Display off)\n\r");
	}
	
	//Color Red
	if(countSw301 == debounce_timer) {
		Color = 'R';
		//Turn on display
	//	PortF->Data &= (1<<4);
		countSw301 = 0;
		hasBeenPressed = true;
		uartTxPoll(UART0,"SW301 was pressed (Color Red)\n\r");
	}
	
	//Color Blue
	if(countSw302 == debounce_timer) {
		Color = 'B';
		//Turn on display
//		PortF->Data &= (1<<4);
		countSw302 = 0;
		hasBeenPressed = true;
		uartTxPoll(UART0,"SW302 was pressed (Color Blue)\n\r");
	}
	
	//Color Green
	if(countSw303 == debounce_timer) {
		Color = 'G';
		//Turn on display
//		PortF->Data &= (1<<4);
		countSw303 = 0;
		hasBeenPressed = true;
		uartTxPoll(UART0,"SW303 was pressed (Color Green)\n\r");
	}
}

//*****************************************************************************
// The ISR sets AlertRowUpdate to true if the display should be updated.
// The routine will call getLCDRow() (led_chars.c) to determine  the 8-bit 
// data value that will be written out to PORTB.
//
// If AlertRowUpdate is false, simply return
//
//*****************************************************************************
void updateDisplay(void)
{

	uint8_t data = 0; 
	
	if(!AlertRowUpdate){
		return;
	}
	
	getLCDRow(RefreshRate, Row, &data);

	PortC->Data = LATCH_ALL_ON_M;
	
	
	  // Write out 0xFF to set all latches to 0xFF
    // All of the Rows and LEDs are active low!
  PortB->Data = 0xFF;
    
    // Disable all the latches
  PortC->Data = LATCH_ALL_OFF_M;
    
    // Enable Row latch
  PortC->Data = ROW_EN;
    
    // Write the data to the row latch.  
    // See the example above.
  PortB->Data = ~(1<<Row);
    
    // Decide which color latch to enable
	if( Color == 'R'){
		PortC->Data = RED_EN;
		}
	else if( Color == 'G'){
			PortC->Data = GRN_EN;
		}
	else if( Color == 'B'){
			PortC->Data = BLU_EN;
		}
	else// Display is off
		{
			PortC->Data = LATCH_ALL_OFF_M;
		}
	
   // Write the active LED into the selected color latch
   PortB->Data = data;
    
   // Disable all the latches
   PortC->Data = LATCH_ALL_OFF_M;
   
   // Enable Output
   PortF->Data &= LATCH_OE_B_M;
}

//*****************************************************************************
// Initializes all of the GPIO pins used for the LED matrix, push 
// buttons, and the left potentiometer.
//*****************************************************************************
void initializeGpioPins(void)
{

	uint32_t delay; // delay for initialization
	uint32_t i;
	// using the board_config.h to configure all ports needed
	
	//Enable A
	SYSCTL_RCGCGPIO_R   |= SYSCTL_RCGCGPIO_R0;
	for(i = 0; i < 10000; i++)
	{
		
	}
	
	GPIO_PORTA_DIR_R = 0x00; 
	GPIO_PORTA_PUR_R = 0xC0;
	GPIO_PORTA_AFSEL_R = 0x00;   
	GPIO_PORTA_DEN_R = 0xC0;
	
	//Enable B
	SYSCTL_RCGCGPIO_R   |= SYSCTL_RCGCGPIO_R1;
  delay = SYSCTL_RCGCGPIO_R;
	
	GPIO_PORTB_DIR_R = 0xFF; 
	GPIO_PORTB_PUR_R = 0x00;
	GPIO_PORTB_DEN_R = 0xFF;
	
	//Enable D
  SYSCTL_RCGCGPIO_R   |= SYSCTL_RCGCGPIO_R3;
  delay = SYSCTL_RCGCGPIO_R;
     
	GPIO_PORTD_DIR_R = 0x00;
	GPIO_PORTD_PUR_R = 0x0C;
	GPIO_PORTD_DEN_R = 0x0C; 
		 
	//Enable E
  SYSCTL_RCGCGPIO_R   |= SYSCTL_RCGCGPIO_R4;
  delay = SYSCTL_RCGCGPIO_R;
	
	GPIO_PORTE_DIR_R = 0x00; 
	GPIO_PORTE_PUR_R = 0x0C;
	GPIO_PORTE_AMSEL_R = 0x0C;

	//Enable F
  SYSCTL_RCGCGPIO_R   |= SYSCTL_RCGCGPIO_R5;
  delay = SYSCTL_RCGCGPIO_R;

  // Port F has a lock register, so we need to unlock it before making modifications
  GPIO_PORTF_LOCK_R = 0x4C4F434B;
  GPIO_PORTF_CR_R = 0xFF;
	
	GPIO_PORTF_DIR_R = 0x10;
	GPIO_PORTF_PUR_R = 0x00;
	GPIO_PORTF_DEN_R = 0x10; 

 }

