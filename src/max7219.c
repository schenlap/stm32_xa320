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
#define REG_NOP           0x00                        // Nop for daisy chaining
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
#define CLK_PORT      GPIOA                           // assume "CLK" is on PA5
#define CLK_PIN       GPIO5
#define LOAD_PORT     GPIOA                           // assume "LOAD (nCS)" is on PA6
#define LOAD_PIN      GPIO6


/*
*********************************************************************************************************
* Private Data
*********************************************************************************************************
*/
uint8_t max7219_cnt = 0;


/*
*********************************************************************************************************
* Private Function Prototypes
*********************************************************************************************************
*/
static void max7219_Write (uint8_t daisy_nr, unsigned char reg_number, unsigned char dataout);
static void max7219_SendByte (unsigned char data);
static unsigned char max7219_LookupCode (char character);


// ...................................... Public Functions ..............................................


/*
*********************************************************************************************************
* MAX7219_Init()
*
* Description: Initialize MAX7219 module; must be called before any other MAX7219 functions.
* Arguments  : daisy_cnt: nr of max7219 in daisy chain (1 .. 6)
* Returns    : none
*********************************************************************************************************
*/
void max7219_setup (uint8_t daisy_cnt)
{
	uint8_t i;
	rcc_periph_clock_enable(RCC_GPIOA); // TODO: RCC_xxxx aus PORT erzeugen

	gpio_mode_setup(DATA_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, DATA_PIN);
	gpio_mode_setup(CLK_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, CLK_PIN);
	gpio_mode_setup(LOAD_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LOAD_PIN);

	max7219_cnt = daisy_cnt;
	for (i = 1; i <= daisy_cnt; i++) {
		max7219_Write(i, REG_SCAN_LIMIT, 7);                   // set up to scan all eight digits
		max7219_Write(i, REG_DECODE, 0x00);                    // set to "no decode" for all digits
		max7219_Write(i, REG_SHUTDOWN, 1);                     // put MAX7219 into "normal" mode
		max7219_Write(i, REG_DISPLAY_TEST, 0);                 // put MAX7219 into "normal" mode
	}
	max7219_ClearAll();                                    // clear all digits
	max7219_SetBrightnessAll(0);
}


void max7219_SetBrightnessAll (uint8_t brightness)
{
  char i;
  for (i=1; i <= max7219_cnt ; i++)
    max7219_SetBrightness(i, brightness);
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
void max7219_SetBrightness (uint8_t nr, char brightness)
{
	uint8_t i;

	for (i = 1; i <= max7219_cnt; i++) {
		brightness &= 0x0f;                                 // mask off extra bits
		max7219_Write(nr, REG_INTENSITY, brightness);           // set brightness
	}
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
void max7219_ClearAll (void)
{
  char i;
  for (i=1; i <= max7219_cnt ; i++)
    max7219_Clear(i);                           // turn all segments off
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
void max7219_Clear(uint8_t daisy_nr)
{
  char i;
  for (i=0; i < 8; i++)
    max7219_DisplayChar(i + 8 * (daisy_nr - 1), ' ');                           // turn all segments off
}


/*
*********************************************************************************************************
* MAX7219_DisplayChar()
*
* Description: Display a character on the specified digit.
* Arguments  : digit = digit number (0-7*n) left to right
*              character = character to display (0-9, A-Z)
* Returns    : none
*********************************************************************************************************
*/
void max7219_DisplayChar (uint8_t digit, char character)
{
	uint8_t nr = 1;

	if (digit >= 8) {
		digit -= 8;
		nr ++;
	}

	max7219_Write(nr, 8-digit, max7219_LookupCode(character));
}

// Display char with data point
void max7219_DisplayCharDp (uint8_t digit, char character, uint8_t dp)
{
	uint8_t nr = 1;

	if (digit >= 8) {
		digit -= 8;
		nr ++;
	}

	if (dp)
		max7219_Write(nr, 8-digit, max7219_LookupCode(character) | 0x80);
	else
		max7219_Write(nr, 8-digit, max7219_LookupCode(character));
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
			str++;
		}
}

void max7219_display_string_fixpoint(uint8_t offset, char *str, uint8_t fp)
{
		while (*str != 0) {
			if (fp-- == 1) {
				max7219_DisplayCharDp (offset++,  *str, 1);
			} else {
				max7219_DisplayCharDp (offset++,  *str, 0);
			}
			str++;
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
  {'G', 0x5f},
  {'H', 0x37},
  {'I', 0x30},
  {'J', 0x4e},
  {'L', 0x0e},
  {'n', 0x15},
  {'O', 0x7e},
  {'P', 0x67},
  {'r', 0x05},
  {'S', 0x5b},
  {'T', 0x46},
  {'U', 0x3e},
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
static void max7219_Write (uint8_t daisy_nr, unsigned char reg_number, unsigned char dataout)
{
	uint8_t i;
	gpio_clear(LOAD_PORT, LOAD_PIN);
	for (i = daisy_nr; i < max7219_cnt; i++) {
		max7219_SendByte(REG_NOP);                      // write register number to MAX7219
		max7219_SendByte(0);                            // write data to MAX7219
	}
	max7219_SendByte(reg_number);                       // write register number to MAX7219
	max7219_SendByte(dataout);                          // write data to MAX7219
	for (i = 1; i < daisy_nr; i++) {
		max7219_SendByte(REG_NOP);                      // write register number to MAX7219
		max7219_SendByte(0);                            // write data to MAX7219
	}
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
