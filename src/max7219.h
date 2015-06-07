/*
*********************************************************************************************************
* Module     : MAX7219.H
* Author     : Randy Rasa
* Description: Header file for MAX7219.C (LED Display Driver Routines)
*********************************************************************************************************
*/


/*
*********************************************************************************************************
* Public Function Prototypes
*********************************************************************************************************
*/
void max7219_setup (void);
void max7219_ShutdownStart (void);
void max7219_ShutdownStop (void);
void max7219_DisplayTestStart (void);
void max7219_DisplayTestStop (void);
void max7219_SetBrightness (char brightness);
void max7219_Clear (void);
void max7219_DisplayChar (char digit, char character);
void max7219_display_string(uint8_t offset, char *str);
void max7219_DisplayCharDp (char digit, char character, uint8_t dp);
