//// Mark W. Welker
// This is the sample code to interface to the SPI, IIC  
//
/////
/////////////////////
#include <stdint.h> // so I can use the integers I want
#include <stdio.h> // so I can use sprintf
#include <string.h>
#include "sleep.h"


#define  Button_Data *((uint32_t*)0x41200000)
////////////
// SPI defines
/////////////
// spi registers to work with the spi IP block in zynq
//we want to talk to SPI0
// define the minimum set to use
#define SPI0_CFG *((uint32_t *) 0xE0006000)// SPI config register
#define SPI0_EN *((uint32_t *) 0xE0006014)// SPI Enable register
#define SPI0_SR *((uint32_t *) 0xE0006004)// SPI Enable register
//#define SPI0_DEL *((uint32_t *) 0xE0006018)// SPI intraframe delays register
#define SPI0_TXD *((uint32_t *) 0xE000601C)// SPI write data port register
#define SPI0_RXD *((uint32_t *) 0xE0006020)// SPI read data port register
//#define SPI0_DWELL *((uint32_t *) 0xE0006024)// SPI dwell before start register
//#define SPI0_TXWR *((uint32_t *) 0xE0006028)// SPI transmit FIFO not full level register
//#define SPI0_ID *((uint32_t *) 0xE00060FC)// SPI Module ID
/////////////////////////////////////
#define LSM9DS1_Who 0x0f
#define LSM9DS1_CTRL_Reg1 0x10
#define LSM9DS1_Temp_G_low 0x15
#define LSM9DS1_Temp_G_high 0x16
#define JUNK 0
//LSM9DS1 - 10 registers; they're accessed by passing address into WRITE/READ_SPI functions
/////////////////////////////////////////////
//
// UnLock SPI
//
//SLCR addresses for SPI reset
#define SLCR_LOCK *( (uint32_t *) 0xF8000004)
#define SLCR_UNLOCK *( (uint32_t *) 0xF8000008)
#define SLCR_SPI_RST *( (uint32_t *) 0xF800021C)
//SLCR lock and unlock keys
#define UNLOCK_KEY 0xDF0D
#define LOCK_KEY 0x767B
////////////////////////
// Useful defines
#define timedelay 200000
#define CFG_NoSS 0x0BC27
#define CFG_SS0 0x08C27
#define CFG_SS0_Start 0x18C27
////////////
// IIC defines
///////////
#define IIC_CFG *((uint32_t *) 0xE0005000)// IIC config register
#define IIC_STAT *((uint32_t *) 0xE0005004)// IIC Status config register
#define IIC_ADDR *((uint32_t *) 0xE0005008)// IIC Address register
#define IIC_DATA *((uint32_t *) 0xE000500C)// IIC Data register
#define IIC_TSIZE *((uint32_t *) 0xE0005014)// IIC Transfer Size register
#define IIC_ISR *((uint32_t *) 0xE0005010)// IIC Interupt Status register
#define IIC_Config 0x0C0F // 0x0C0E write 0x0C0F read
//all of these registers are for IIC1(for blackboard); IIC0 is for PMOD port
//IIC FIFO size: 16 bytes
// (Q6) bit 2 of IIC_CFG;1 is master, 2 is slave
//SLCR Register addresses and lock/unlock key values
#define SLCR_LOCK *( (uint32_t *) 0xF8000004)
#define SLCR_UNLOCK *( (uint32_t *) 0xF8000008)
#define SLCR_IIC_RST *( (uint32_t *) 0xF8000224)
#define UNLOCK_KEY 0xDF0D
#define LOCK_KEY 0x767B
#define TimerDelay 200000
#define LM75B_Addr 0x48
////////////
// 7-segment display defines
///////////
#define  SEG_CTL *((uint32_t *)0x43c10000)
#define  SEG_DATA *((uint32_t*)0x43C10004)
////////////
// UART defines
///////////
#define UART1_CR    *((uint32_t *) 0xE0001000)// UART1 control register
#define UART1_MR    *((uint32_t *) 0xE0001004)// UART1 Mode register
#define UART1_BAUDGEN	*((uint32_t *) 0xE0001018)
#define UART1_BAUDDIV	*((uint32_t *) 0xE0001034)

#define UART1_SR	*((uint32_t *) 0xE000102C)	//UART1 status reg
#define UART1_DATA	*((uint32_t *) 0xE0001030)	//UART1 TX/RX FIFO DATA


#define BaudGen115200 0x7c
#define BaudDiv115200 6

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

void reset_iic(void)
{
SLCR_UNLOCK = UNLOCK_KEY; //unlock SLCRs
SLCR_IIC_RST = 0x3; //assert I2C reset
SLCR_IIC_RST = 0; //deassert I2C reset
SLCR_LOCK = LOCK_KEY; //relock SLCRs
}
void reset_SPI(void)
{
int i=0; //i for delay
SLCR_UNLOCK = UNLOCK_KEY; //unlock SLCRs
SLCR_SPI_RST = 0xF; //assert SPI reset
for(i=0;i<1000;i++); //make sure Reset occurs
SLCR_SPI_RST = 0; //deassert
SLCR_LOCK = LOCK_KEY; //relock SLCRs
}
void iic_init(){

	IIC_CFG = IIC_Config;

}
/////
void WRITE_SPI(uint8_t adr,uint8_t WRITE_BYTE)
{
	SPI0_CFG = 0x8027;//Slave Select 0
	uint8_t byte_1 = (0 << 7) + adr;
	uint8_t dummy_read;

	SPI0_TXD = byte_1;
	SPI0_TXD = WRITE_BYTE;

	SPI0_CFG |= 0x00010000;
	usleep(500);

	dummy_read = SPI0_RXD;
	dummy_read = SPI0_RXD;
}

uint8_t READ_SPI(uint8_t adr)
{
SPI0_CFG = 0x8027;//Slave Select 0  !!!!!!?
uint8_t dummy_write = (1 << 7) + adr;
uint8_t dummy_read, return_read;

SPI0_TXD = dummy_write;
SPI0_TXD = dummy_write;

SPI0_CFG |= 0x00010000;
usleep(500);

dummy_read = SPI0_RXD;
return_read = SPI0_RXD;

return return_read;
}


int main(){

int8_t SPI_LO = 0;
int8_t SPI_HI = 0;
int16_t SPI_Temp = 0;
int16_t SPI_Zaxis = 0;
int LowByte,HiByte,DispBytes;
int temp16, tempc;

char SendString[90];

initUart1(); // set uart1 to 115200 8 n 1


//
//
///////////////////////////////////
// SPI Init 
//////////////////////////////////
	//uint32_t TempLo,TempHi;
	reset_SPI();
// configure SPI
	SPI0_CFG = CFG_NoSS;
	SPI0_EN = 1;
//
	WRITE_SPI(0x10,0xA0);	
	WRITE_SPI(0x20,0xA0);
	WRITE_SPI(0x23,0x10);
///////
// IIC iitialization
/////////////////
	reset_iic(); //need icc reset here to communicate with I2c
	iic_init();
 

while (1){ // in a loop so I can watch it through debugger
//////////
//SPI
//////////////
	SPI_LO = READ_SPI(LSM9DS1_Temp_G_low);//Low
	SPI_HI = READ_SPI(LSM9DS1_Temp_G_high);//High-

	temp16 = (SPI_HI << 8) | SPI_LO;
	SPI_Temp = (temp16 / 16) + 25;

	SPI_LO = READ_SPI(0x1C);//Low
	SPI_HI = READ_SPI(0x1D);//High-

	temp16 = (SPI_HI << 8) | SPI_LO;
	SPI_Zaxis = temp16;

///////////////
// IIC	
/////////////////
	//data transfer:
	//master drives SCL clock, drives SDA low, and transmits 7+1 bit address 
	//periferal responds ACK (by driving SDA low)
	//master will begin transmitting/receiving bytes to/from the slave
	//during a read, perifieral sends data in 8 bits before stoping to wait for a ACK
	// number of requested bytes must be written to transfer size rwgister
	//during a write, all data placed in the 16 byte FIFO will be sent untill the FIFO is empty
	// new data can be placed in the write fifo and a new write transfer can be initiated
    IIC_TSIZE = 2; //transfering 2 bytes
	IIC_ADDR = LM75B_Addr;
	while ((IIC_ISR & 1) != 1){}	//wait here till transfer complete
				//read first two bytes, will be the temp
	HiByte = IIC_DATA;
	LowByte = IIC_DATA;

	temp16 = (HiByte << 8) | LowByte;
	tempc = (temp16 >> 5) * .125;

	if(Button_Data & 0x01){
		DispBytes = SPI_Temp;
	}else{
		DispBytes = tempc;
	}
	
	Disp_BCD(DispBytes);
	sprintf(SendString, "Z axis = %i \n", SPI_Zaxis);
	uart1_putstr(SendString);
	sprintf(SendString, "SPI temp = %i \n", SPI_Temp);
	uart1_putstr(SendString);
	sprintf(SendString, "IIC temp = %i \n", tempc);
	uart1_putstr(SendString);

	usleep(500000);

	strcpy(SendString, "\033c");
	uart1_putstr(SendString);

}
}
