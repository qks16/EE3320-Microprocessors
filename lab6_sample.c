// 5/27/2024
// MarkW Welker
// Lab 6
// read the potentiometer
// display the value on the 7 segments
// Use the potentiometer to control servo position
//////////////////////////////

#include "sleep.h" // For delay functions
#include <stdint.h> // For specific integer types
#include <stdio.h> // For input/output functions

// Memory-mapped register definitions
#define  SEG_CTL *((uint32_t *)0x43c10000)      // 7-segment control register 
#define  SEG_DATA *((uint32_t*)0x43C10004)    // 7-segment digit data register
#define  Xadc_Data *((uint32_t*)0x43c5020C)   // VP input for the ADC 
#define  Xadc_Cfg *((uint32_t*)0x43c50300)   // VP input for the ADC 

////////////////////////////////////
// Servo Defines
///////////////////////////////
#define TTC0_ClkCntl_0 *((uint32_t *) 0xF8001000) //config register
#define TTC0_OpMode_0 *((uint32_t *) 0xF800100C) //Operating Mode Configuration
#define TTC0_Interval_0 *((uint32_t *) 0xF8001024) //Interval; count
#define TTC0_Match_0 *((uint32_t *) 0xF8001030) //Match Count
#define TTC0_InterruptEn_0 *((uint32_t *) 0xF8001060) //Interrupt Enable
#define TTC0_EvntCntl_0 *((uint32_t *) 0xF800106C) //Event Control
////////////////////////////////

#define sleep_delay 10000 // Just a number so we done read the ADC so fast. 

void Display_Digit (uint8_t pos, uint8_t val ) {
	// pos is the position to write the digit on the 4 digit 7 segment display values 1-4
	// val is the value to write to the digit 0-f

	//We must accomodate if val is greater than 1 by adding 1 to the next position and not rolling past 9.

   uint32_t temp=0; // used to mask off the digit to be written.
	//set display enabled in standard (hex) mode
	SEG_CTL = 1; // this mode will also display A_F in Hex but this routine only sends BCD.

	temp = SEG_DATA; // get the current values.
	switch (pos){ // using a switch to create a mask in the right position to clear the target digit
	case 1: temp &= 0xfffffff0; // you are working with a 32 bit value. decimal is 7 or each digit.
			break;
	case 2: temp &= 0xfffff0ff;
			break;
	case 3: temp &= 0xfff0ffff;
			break;
	case 4: temp &= 0xf0ffffff;
			break;
	default: break;
	}
	temp |= (val&0xF)<<((pos-1)*8); //put digit data in appropriate bitfield
	//disable all decimal points
	temp|=0x80808080;

	SEG_DATA = temp;
}

	void Disp_BCD(uint16_t value){
		// input is a BCD number assumed to be less than 9999 ( 4 digit display)
		char bcdstr[20]  ; // 4 digits plus null
		int numchars,Strlen ; // hold teh number of cahracters converted
//toSend temp variable that is to be sent to display.
		numchars = sprintf(bcdstr,"%d",value);

		Strlen = numchars; // use this for stringlength to reverse send the number.

		 while (numchars != 0){
			 Display_Digit(numchars,(bcdstr[Strlen-numchars]-'0'));
			 numchars --;
		 }


	}

int main(void)
{
   uint16_t ADC_Val =0;

//////////////////
// Servo Initi code
//
////////////
   TTC0_OpMode_0 = 0x11; // make sure it is off before we program it.
   usleep(2000); // wit 200 mS between register programming.  just in case. 
     // It really should be a matter of setting up teh timer and watching it on a scope.
    TTC0_ClkCntl_0 = 0x13;
   usleep(2000); // wit 200 mS between register programming.  just in case. 
   TTC0_Interval_0 = 2160; // This sets it to 20 MS intervals
    // interval = 2160 
  usleep(2000); // wit 200 mS between register programming.  just in case. 
    TTC0_Match_0 = 275; // 162 This sets it to a 1.5mS pulse width  50 min 275 max
    TTC0_InterruptEn_0 =0; // Do nto gneerate any interrupts
    TTC0_EvntCntl_0 = 0; // do not count events
   // setup for 20mS interval. prescale = 9 En = 1, CS = 0, CE = 0
   usleep(2000); // wit 200 mS between register programming.  just in case. 
   TTC0_OpMode_0 = 0x4A;
    // PL(bit6) = 1, OW(bit5) = 1, CR(bit4) = 1 (reset the counter), ME(bit3) = 1 Match enable
    // DC = 0, count up, IM = 1 OD = 0count enable
 
 while(1){

  Xadc_Cfg = 0x3803; // point to read the potentiometer 256 samples. with settlein. 
  // We may have to change the averaging of samplesv defualt is no averaging

    ADC_Val = (Xadc_Data >> 8) ; // chop off lower bits to make it more stable 
    Disp_BCD(ADC_Val);
    
    TTC0_Match_0 = 0.87890625 * ADC_Val + 50; // You will need to find the range
    //maps 0-256 to 50-275

    usleep ( sleep_delay); 

 }
 
}