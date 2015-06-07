/*
*********************************************************************************************************
* Module     : MAX7219.C
* Author     : Randy Rasa
* Description: MAX7219 LED Display Driver Routines
*
*  The Maxim MAX7219 is an LED display driver thant can control up to 64 individual LEDs, or
*  eight 7-segment LED digits, or any combination of individual LEDs and digits.  It frees the
*  host from the chore of constantly multiplexing the 8 rows and 8 columns.  In addition, it
*  takes care of brightness control (16 steps), and implements display test and display blank
*  (shutdown) features.
*
*  The host communicates with the MAX7219 using three signals: DATA, CLK, and LOAD.  This
*  modules bit-bangs them, but Motorola's SPI interface (or similar interface from other
*  manufacturers) may also be used to simplify and speed up the data transfer.
*                   ___ ___ ___ ___ ___ ___ ___ ___ ___ ___ ___ ___ ___ ___ ___ ___
*    DATA _________|D15|D14|D13|D12|D11|D10|D09|D08|D07|D06|D05|D04|D03|D02|D01|D00|______
*         ________    __    __    __    __    __    __    __    __    __    __    ________
*    CLK          |__|  |__|  |__|  |__|  |__|  |__|  |__|  |__|  |__|  |__|  |__|
*                __________________________________________________________________    ___
*    LOAD ______|                                                                  |__|
*
*
*  Implementation Notes:
*
*  1. This module was written and tested using an Atmel AT89C2051 microcontroller, with the
*     MAX7219 connected to I/O pins P3.3 (LOAD), P3.4 (CLK), and P3.3 (DATA).
*
*  2. Macros are provided to simplify control of the DATA, CLK, and LOAD signals.  You may also use
*     memory-mapped output ports such as a 74HC374, but you'll need to replace the macros with
*     functions, and use a shadow register to store the state of all the output bits.
*
*  3. This module was tested with the evaluation version of Hi-Tech C-51, using the small memory model.
*     It should be portable to most other compilers, with minor modifications.
*
*  4. The MAX7219 is configured for "no decode" mode, rather than "code B" decoding.  This
*     allows the program to display more than the 0-9,H,E,L,P that code B provides.  However,
*     the "no decode" method requires that each character to be displayed have a corresponding
*     entry in a lookup table, to convert the ascii character to the proper 7-segment code.
*     MAX7219_LookupCode() provides this function, using the MAX7219_Font[] array.  If you need
*     to display more than the characters I have provided, simply add them to the table ...
*
*********************************************************************************************************
*/


/*
*********************************************************************************************************
* Include Header Files
*********************************************************************************************************
*/
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include "max7219.h"                                  // MAX7219 header file


/*
*********************************************************************************************************
* Constants
*********************************************************************************************************
*/
#define REG_DECODE        0x09                        // "decode mode" register
#define REG_INTENSITY     0x0a                        // "intensity" register
#define REG_SCAN_LIMIT    0x0b                        // "scan limit" register
#define REG_SHUTDOWN      0x0c                        // "shutdown" register
#define REG_DISPLAY_TEST  0x0f                        // "display test" register

#define INTENSITY_MIN     0x00                        // minimum display intensity
#define INTENSITY_MAX     0x0f                        // maximum display intensity


/*
*********************************************************************************************************
* Macros
*********************************************************************************************************
*/
#define DATA_PORT     GPIOA                           // assume "DATA" is on PA7
#define DATA_PIN      GPIO7
#define CLK_PORT      GPIOA                           // assume "CLK" is on PA4
#define CLK_PIN       GPIO4
#define LOAD_PORT     GPIOA                           // assume "LOAD (nCS)" is on PA6
#define LOAD_PIN      GPIO6


/*
*********************************************************************************************************
* Private Data
*********************************************************************************************************
*/
// ... none ...


/*
*********************************************************************************************************
* Private Function Prototypes
*********************************************************************************************************
*/
static void max7219_Write (unsigned char reg_number, unsigned char data);
static void max7219_SendByte (unsigned char data);
static unsigned char max7219_LookupCode (char character);


// ...................................... Public Functions ..............................................


/*
*********************************************************************************************************
* MAX7219_Init()
*
* Description: Initialize MAX7219 module; must be called before any other MAX7219 functions.
* Arguments  : none
* Returns    : none
*********************************************************************************************************
*/
void max7219_setup (void)
{
	rcc_periph_clock_enable(RCC_GPIOA); // TODO: RCC_xxxx aus PORT erzeugen

	gpio_mode_setup(DATA_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, DATA_PIN);
	gpio_mode_setup(CLK_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, CLK_PIN);
	gpio_mode_setup(LOAD_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LOAD_PIN);

	max7219_Write(REG_SCAN_LIMIT, 7);                   // set up to scan all eight digits
	max7219_Write(REG_DECODE, 0x00);                    // set to "no decode" for all digits
	max7219_ShutdownStop();                             // select normal operation (i.e. not shutdown)
	max7219_DisplayTestStop();                          // select normal operation (i.e. not test mode)
	max7219_Clear();                                    // clear all digits
	max7219_SetBrightness(3);
}


/*
*********************************************************************************************************
* MAX7219_ShutdownStart()
*
* Description: Shut down the display.
* Arguments  : none
* Returns    : none
*********************************************************************************************************
*/
void max7219_ShutdownStart (void)
{
  max7219_Write(REG_SHUTDOWN, 0);                     // put MAX7219 into "shutdown" mode
}


/*
*********************************************************************************************************
* MAX7219_ShutdownStop()
*
* Description: Take the display out of shutdown mode.
* Arguments  : none
* Returns    : none
*********************************************************************************************************
*/
void max7219_ShutdownStop (void)
{
  max7219_Write(REG_SHUTDOWN, 1);                     // put MAX7219 into "normal" mode
}


/*
*********************************************************************************************************
* MAX7219_DisplayTestStart()
*
* Description: Start a display test.
* Arguments  : none
* Returns    : none
*********************************************************************************************************
*/
void max7219_DisplayTestStart (void)
{
  max7219_Write(REG_DISPLAY_TEST, 1);                 // put MAX7219 into "display test" mode
}


/*
*********************************************************************************************************
* MAX7219_DisplayTestStop()
*
* Description: Stop a display test.
* Arguments  : none
* Returns    : none
*********************************************************************************************************
*/
void max7219_DisplayTestStop (void)
{
  max7219_Write(REG_DISPLAY_TEST, 0);                 // put MAX7219 into "normal" mode
}


/*
*********************************************************************************************************
* MAX7219_SetBrightness()
*
* Description: Set the LED display brightness
* Arguments  : brightness (0-15)
* Returns    : none
*********************************************************************************************************
*/
void max7219_SetBrightness (char brightness)
{
  brightness &= 0x0f;                                 // mask off extra bits
  max7219_Write(REG_INTENSITY, brightness);           // set brightness
}


/*
*********************************************************************************************************
* MAX7219_Clear()
*
* Description: Clear the display (all digits blank)
* Arguments  : none
* Returns    : none
*********************************************************************************************************
*/
void max7219_Clear (void)
{
  char i;
  for (i=0; i < 8; i++)
    max7219_Write(i, 0x00);                           // turn all segments off
}


/*
*********************************************************************************************************
* MAX7219_DisplayChar()
*
* Description: Display a character on the specified digit.
* Arguments  : digit = digit number (0-7)
*              character = character to display (0-9, A-Z)
* Returns    : none
*********************************************************************************************************
*/
void max7219_DisplayChar (char digit, char character)
{
  max7219_Write(digit, max7219_LookupCode(character));
}

// Display char with data point
void max7219_DisplayCharDp (char digit, char character, uint8_t dp)
{
	if (dp)
		max7219_Write(digit, max7219_LookupCode(character));
	else
		max7219_Write(digit, max7219_LookupCode(character) | 0x80);
}

void max7219_display_string(uint8_t offset, char *str)
{
		while (*str != 0) {
			if (*(str + 1) == '.') {
				max7219_DisplayCharDp (offset++,  *str, 1);
				str++;
			} else {
				max7219_DisplayCharDp (offset++,  *str, 0);
			}
		}
}


// ..................................... Private Functions ..............................................


/*
*********************************************************************************************************
* LED Segments:         a
*                     ----
*                   f|    |b
*                    |  g |
*                     ----
*                   e|    |c
*                    |    |
*                     ----  o dp
*                       d
*   Register bits:
*      bit:  7  6  5  4  3  2  1  0
*           dp  a  b  c  d  e  f  g
*********************************************************************************************************
*/
static const struct {
	char   ascii;
	char   segs;
} MAX7219_Font[] = {
  {' ', 0x00},
  {'0', 0x7e},
  {'1', 0x30},
  {'2', 0x6d},
  {'3', 0x79},
  {'4', 0x33},
  {'5', 0x5b},
  {'6', 0x5f},
  {'7', 0x70},
  {'8', 0x7f},
  {'9', 0x7b},
  {'A', 0x77},
  {'B', 0x1f},
  {'C', 0x4e},
  {'D', 0x3d},
  {'E', 0x4f},
  {'F', 0x47},
  {'-', 0x01},
  {'\0', 0x00}
};

/*
*********************************************************************************************************
* MAX7219_LookupCode()
*
* Description: Convert an alphanumeric character to the corresponding 7-segment code.
* Arguments  : character to display
* Returns    : segment code
*********************************************************************************************************
*/
static unsigned char max7219_LookupCode (char character)
{
  uint32_t i;
  for (i = 0; MAX7219_Font[i].ascii; i++)             // scan font table for ascii code
    if (character == MAX7219_Font[i].ascii)
      return MAX7219_Font[i].segs;                    // return segments code
  return 0;                                           // code not found, return null (blank)
}


/*
*********************************************************************************************************
* MAX7219_Write()
*
* Description: Write to MAX7219
* Arguments  : reg_number = register to write to
*              dataout = data to write to MAX7219
* Returns    : none
*********************************************************************************************************
*/
static void max7219_Write (unsigned char reg_number, unsigned char dataout)
{
//	gpio_set(LOAD_PORT, LOAD_BIT);
	gpio_clear(LOAD_PORT, LOAD_PIN);
	max7219_SendByte(reg_number);                       // write register number to MAX7219
	max7219_SendByte(dataout);                          // write data to MAX7219
//	uint32_t t = systime_get(void);
//	gpio_clear(LOAD_PORT, LOAD_BIT);
//	while (systime_get() < t + 2) ;                     // TODO test if necessary...
	gpio_set(LOAD_PORT, LOAD_PIN);
}


/*
*********************************************************************************************************
* MAX7219_SendByte()
*
* Description: Send one byte to the MAX7219
* Arguments  : dataout = data to send
* Returns    : none
*********************************************************************************************************
*/
static void max7219_SendByte (unsigned char dataout)
{
	char i;
	for (i=8; i>0; i--) {
		unsigned char mask = 1 << (i - 1);                // calculate bitmask
		gpio_clear(CLK_PORT, CLK_PIN);
		if (dataout & mask)                               // output one data bit
			gpio_set(DATA_PORT, DATA_PIN);
		else                                              //  or
			gpio_clear(DATA_PORT, DATA_PIN);
		gpio_set(CLK_PORT, CLK_PIN);
	}
}
