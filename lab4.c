//
// The goal is to read the Pmod channels in debug mode and hand wire the pmod pins to see where
// they show up on the GPIO.
// We will read all the general GPIO pins and trace where the digital and analog in pins go.
//

#include <stdint.h>
#include "sleep.h"

#define LED_Data	    *((uint32_t *)0x41210000) // LED are the lower 10 bits
#define Switch_Data     *((uint32_t *)0x41220000)  // switches are the lower 12 bits
#define Button_Data     *((uint32_t *)0x41200000) // buttons are the lower 4 bits
 // define the addresses for the GPIO inputs directly
#define Bank0_Input     *((uint32_t *)0xE000A060) // bank0 inputs
#define Bank1_Input     *((uint32_t *)0xE000A064) // bank1 inputs
#define Bank2_Input     *((uint32_t *)0xE000A068) // bank2 inputs
#define Bank3_Input     *((uint32_t *)0xE000A06C) // bank3 inputs
// the GPIO BANK output data
#define Bank0_Output     *((uint32_t *)0xE000A040) // bank0 outputs
#define Bank1_Output     *((uint32_t *)0xE000A044) // bank1 outputs
#define Bank2_Output     *((uint32_t *)0xE000A048) // bank2 outputs
#define Bank3_Output     *((uint32_t *)0xE000A04C) // bank3 outputs
// the GPIO BANK Direction data
#define Bank0_Dir     	*((uint32_t *)0xE000A204) // bank0 direction
#define Bank1_Dir     	*((uint32_t *)0xE000A244) // bank1 direction
#define Bank2_Dir     	*((uint32_t *)0xE000A284) // bank2 direction
#define Bank3_Dir     	*((uint32_t *)0xE000A2C4) // bank3 direction
//
#define Bank0_Enable    *((uint32_t *)0xE000A208) // bank0 enable
#define Bank1_Enable    *((uint32_t *)0xE000A248) // bank1 enable
#define Bank2_Enable    *((uint32_t *)0xE000A288) // bank2 enable
#define Bank3_Enable    *((uint32_t *)0xE000A2C8) // bank3 enable

 int main (void) {
	 uint32_t InTemp2;

	 uint32_t direction;

		while(1){

		direction = Switch_Data;

		// bank 2 is where all the PMODC pins live.
		// If we set bank2 direction and enable  to 0x78000 we should be able to write to 0x78000 to output
		Bank2_Dir = 0x78000;
		Bank2_Enable = 0x78000;

// turn on LED 12 to be white This lives at bank 0 bits 16 17 18
		// B 1 , R 2, G 4,
		//set LED 12 bits to output
		Bank0_Dir = 0x70000;
		Bank0_Enable = 0x70000;

		if(direction & 0x01) // N/S
		{
			LED_Data = 0x0;
			LED_Data = Switch_Data;

			Bank0_Output = 0x20000; //LED red
			Bank2_Output = 0x8000; //signal out to turn red

			sleep(2);
	
			Bank0_Output = 0x40000; // LED green

			sleep(2);

			Bank0_Output = 0x60000; // LED yellow

			sleep(1);

			Bank0_Output = 0x20000; //LED red

			sleep(2);
			Bank2_Output = 0x10000;//signal out to turn green

			sleep(2);
			Bank2_Output = 0x20000;//signal out to turn yellow

			sleep(1);
		}else // E/W
		{
			LED_Data = 0x0;
			LED_Data = Switch_Data;
			InTemp2 = (Bank2_Input & 0x780000) >> 19; //read in bank 2 data and put bits for pins into  bits 3:0

			if(InTemp2 & 0x01) //set to red signal
			{
				Bank0_Output = 0x20000;
			}else if(InTemp2 & 0x02) //set to green signal
			{
				Bank0_Output = 0x40000;
			}else if(InTemp2 & 0x04) //set to yellow signal
			{
				Bank0_Output = 0x60000;
			}
		}

	}

	return 1;

}