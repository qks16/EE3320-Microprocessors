// Mark W. Welker
// Code based on code found on realdigital.org site.
///
///
////////////////////
#include <stdint.h> // For specific integer types

#include "sleep.h"   // For delay functions
#include <stdio.h>  // For input/output functions




// Memory-mapped register definitions
#define  SEG_CTL *((uint32_t *)0x43c10000)
     // 7-segment control register
#define  SEG_DATA *((uint32_t*)0x43C10004)
    // 7-segment digit data register
#define  Button_Data *((uint32_t*)0x41200000)
// Buttons are the lower 4 bits

// Function to display a digit at a given position on the 7-segment display
void Display_Digit(uint8_t pos, uint8_t val)
{
  uint32_t temp = 0;
  SEG_CTL = 1;
  // Enable display in hex mode
  temp = SEG_DATA;
  switch(pos)
  {
    case 1: temp &= 0xfffffff0; break;
    case 2: temp &= 0xfffff0ff; break;
    case 3: temp &= 0xfff0ffff; break;
    case 4: temp &= 0xf0fffff0; break;
    default: break;
  }
  temp |= (val & 0xf) <<((pos - 1) * 8);
  temp |= 0x80808080;
  SEG_DATA = temp;
}

// Function to display a number on the 7-segment display
void Disp_BCD(uint16_t value)
{
  char bcdstr[20];
  int numchars, Strlen;

  numchars = sprintf(bcdstr, "%04d", value);
  Strlen = numchars;

  while(numchars != 0)
  {
    Display_Digit(numchars, (bcdstr[Strlen - numchars] - '0'));
    numchars--;
  }


}

int main(void){
 
uint16_t  counter = 0;
     // Counter for the stopwatch
 
int  stopwatchRunning = 1;
// Flag to check if stopwatch is running
uint32_t  buttonState; // Read button state

 
while  (1)
 {
   
// for (;;) {
  
  buttonState = Button_Data; // Read button state
  
// Check buttons and update the stopwatch state
   
	if ( (Button_Data & 0x01 ) == 0x01){
		stopwatchRunning = 0;

    }

  if ( (Button_Data & 0x02 ) == 0x02){
		stopwatchRunning = 1;

    }

  if ( (Button_Data & 0x04 ) == 0x04){
		counter = 0;
    Disp_BCD(counter);

    }

  if (stopwatchRunning) {
        counter++;
        Disp_BCD(counter);
  // Increment counter
      sleep(1);
    // Sleep for 1 second (adjust as needed for your hardware)
      }else{
        sleep(1);
      }

  }
  
Disp_BCD(counter);
// Display the counter value
 
return 0;
}


