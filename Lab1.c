// M Welker Attempt to convert lab 1 from assembly to C.

#include <stdint.h>



#define LED_DATA_ADDR	0x41210000 // LED are the lower 10 bits
#define SW_DATA         0x41220000  // switches are the lower 12 bits
#define Button_Data     0x41200000 // buttons are the lower 4 bits



 int main (void) {

		uint32_t *dataLED, *dataSW, *dataBut ;
		uint32_t valSW;


		dataLED = (uint32_t *)LED_DATA_ADDR;	//set the address of dataLED to led control
		dataSW = (uint32_t *)SW_DATA;	//set the address of dataSW to Switches
		dataBut = (uint32_t *)Button_Data;	//set the address of dataSW to Buttons




		while(1){

		valSW = *dataSW; // read the switches
		*dataLED = 0;	//write '0' to the LED's
		*dataLED = valSW;	//write the switch value to the register used for sw and button

		}



    return 1;
}
