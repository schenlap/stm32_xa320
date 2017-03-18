#ifndef LED_H
#define LED_H

#include <stdlib.h>
#include <stdint.h>

#define LED_NAV      0
#define LED_VOR      1
#define LED_ILS      2
#define LED_VOR2     3
#define LED_MLS      3
#define LED_ADF      4
#define LED_BFO      5
#define LED_HF1      6 
#define LED_HF2      7
#define LED_AM       8
#define LED_VHF1     9
#define LED_VHF2     10
#define LED_VHF3     11
#define LED_SEL      12


void led_set(uint32_t pin, uint32_t state);
void led_clear(uint32_t pin);

#endif // LED_H
