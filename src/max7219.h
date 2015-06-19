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
void max7219_setup (uint8_t daisy_cnt);
void max7219_ShutdownStart (void);
void max7219_DisplayTestStart (void);
void max7219_DisplayTestStop (void);
void max7219_SetBrightness (uint8_t nr, char brightness);
void max7219_SetBrightnessAll (uint8_t brightness);
void max7219_Clear (uint8_t daisy_nr);
void max7219_ClearAll (void);
void max7219_DisplayChar (uint8_t digit, char character);
void max7219_display_string(uint8_t offset, char *str);
void max7219_display_string_fixpoint(uint8_t offset, char *str, uint8_t fp);
void max7219_DisplayCharDp (uint8_t digit, char character, uint8_t dp);
