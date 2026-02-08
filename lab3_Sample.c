////////////
//Lab 3 UART 
// Based on code from realdigital .org
//Mark W. Welker
//////////
#include <stdint.h> // so I can use the integers I want
#include <stdio.h>  // so I can use sprintf
#include "sleep.h" // for delay functions

#include <stdlib.h>

#define UART1_CR    *((uint32_t *) 0xE0001000)// UART1 control register
#define UART1_MR    *((uint32_t *) 0xE0001004)// UART1 Mode register
#define UART1_BAUDGEN	*((uint32_t *) 0xE0001018)
#define UART1_BAUDDIV	*((uint32_t *) 0xE0001034)

#define UART1_SR	*((uint32_t *) 0xE000102C)	//UART1 status reg
#define UART1_DATA	*((uint32_t *) 0xE0001030)	//UART1 TX/RX FIFO DATA


#define BaudGen115200 0x7c
#define BaudDiv115200 6

//(T1)UART = 64bytes; SPI = 128bytes; I2C = 16bytes
//(T4)UART = 3 signals; SPI = 5 signals; I2C = 3 signals
//(T5)read - 2 byte phases. 1st byte, 1 for read bit followed by 7 bit register address
//2nd byte, sent out n byte to receive n byte to be read
//    write - 2 byte phases, 1st byte, 0 for write bit followed by 7 bit register address
//2nd byte , will be written to the requested register
//    flush recieve FIFO by reading

//(GUARENTEED FINAL QUESTION)how many bytes dies the microprocessor have to send to read 1 byte back from the sensor: 2 bytes

//check if room for bytes in tx fifo
int uart1_tx_full()
{
	return (UART1_SR & 16)!=0;			//Check SR register bit4, return 1 if fifo full
}

void uart1_put_char (char dat)
{
	while(uart1_tx_full());//wait for room in FIFO
	UART1_DATA = dat;				//write to TX FIFO
}

void uart1_putstr(char *ToSend){ // Takes a pointer to a null terminated string

// send the string to put_char 1 char at a time. 
// obtained from chatgpt using prompt:
//write void uart1_putstr(char *ToSend) subroutine using uart1_put_char(char *dat)
	while (*ToSend != '\0')
	{
		uart1_put_char(*ToSend);
		ToSend++;
	}
}

int UART1_RXEmpty (){
	uint32_t UARTData; // general variable to hold data  to/from UART
	UARTData = UART1_SR;
	if ((UARTData & 0x02) == 2){
		return 1;
	}else {
		return 0;
	}
}

char uart1_get_char(){
	while(UART1_RXEmpty()); 	//wait for room something in FIFO
	return (char)UART1_DATA; // return data from the FIFO
}

// Need to initialize the UART
//
//
void ResetUART1(){
// as part of reset disable the tx and rx to prevernt errant things from going out.     
	UART1_CR = 0x17;  // assert the reset of the UART / disable transmit and recv
	while ((UART1_CR & 0x2)  == 2){} // wait till xmit reset clears
	while ((UART1_CR & 0x1)  == 1){} // wait till rcv reset cleared by the UART
	}

void Configure_UART1 (){

	uint32_t UARTData; // general variable to hold data  to/from UART
	//configure the mode register
	//normal mode bits [9:8] = 2b00
	// mode = auto echo bits [9:8] =01
	//one stop bit, bits [7:6] = 2b00
	//8 data bits, bits [2:1] = 2b0x
	//use reference clock, bit0 = 0
	//disable parity, bits[5:3] = 3b1xx
	//6b100000 == 0x20 // no echoe
	// 9b100000 == 0x120 // auto echo

	UART1_MR = 0x20;
	// enable the transmit and recieve  OR 0x14 to CR register
	UARTData = UART1_CR; //get the current setting and OR in the
	UARTData |= 0x14;    // Enable TX and RX
	UART1_CR = UARTData;

}

void SetBaudrate1(){
	// set to 115200 baudrate
	UART1_BAUDGEN = BaudGen115200;
	UART1_BAUDDIV = BaudDiv115200;

}

void initUart1() {
	//Set to 115200 8,n,1
	ResetUART1(); // reset the uart to start and disable the TX & RX
	SetBaudrate1(); // set baudrate to 115200
	Configure_UART1(); // configure to 8,n,1
}

int GetInteger(){ // retrieves an integer from the serial port.

	char OneChar;

	OneChar = uart1_get_char(); // get one character WAIT FOR IT.

// build up the string from the user, then convert it to an integer. 
// make sure you only accept sacii values that are integers
// wait for the 0x0d to know the use has presed enter.

	int numstrSize = 90; //max suze of the null terminated string
	char numStr[numstrSize]; // null terminated string variable
	int i = 0; // numStr index

	//build null terminated string with only integers the user inputs
	while(OneChar != 0x0d && i < (numstrSize - 1))
	{
		if((OneChar >= '0' && OneChar <= '9') || (OneChar == '-' && i == 0))
		{
			numStr[i++] = OneChar;
			//uart1_put_char(OneChar); //echo
		}

		OneChar = uart1_get_char();
	}

	numStr[i] = '\0';

	// return integer string into interger data type and return
	return atoi(numStr);
	//utilized chatgpt to help brainstorm code
}

//obtained from chatgpt using prompt: write a sqrt subroutine
uint32_t int_sqrt(uint32_t n) {
    if (n == 0 || n == 1) return n;

    uint32_t start = 1, end = n, ans = 0;

    while (start <= end) {
        uint32_t mid = start + (end - start) / 2;

        if (mid <= n / mid) {  // mid*mid <= n, avoid overflow
            ans = mid;
            start = mid + 1;
        } else {
            end = mid - 1;
        }
    }

    return ans;
}

int main (){

	char SendString[90];
	int userNum;

	initUart1(); // set uart1 to 115200 8 n 1

	for (;;){
		// ask user for a number 
		uart1_putstr("Enter a prime number:\r\n");
		// get the number
		userNum = GetInteger();

		// check to see if it is prime
		double root = int_sqrt(userNum);
		int r = 0;
		int isprime = 1;

		for(int i = 2; i <= root; ++i)
		{
			r = userNum % i;
			if(r == 0)
			{
				isprime = 0;
			}
		}

		if(isprime)
		{
			sprintf(SendString, "\r\n%d", userNum);
			uart1_putstr(SendString);
			uart1_putstr(" is a prime number\r\n");
		}else{
			while(!(isprime))
			{
				userNum--;
				root = int_sqrt(userNum);
				r = 0;
				isprime = 1;
			
				for(int i = 2; i <= root; ++i)
				{
					r = userNum % i;
					if(r == 0)
					{
						isprime = 0;
					}
				}
			}

			sprintf(SendString, "\r\n%d\r\n", userNum);
			uart1_putstr(SendString);
		}

	}

	return (1);
}




