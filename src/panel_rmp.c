#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "systime.h"
#include "gpio.h"
#include "encoder.h"
#include "teensy.h"
#include "usb.h"
#include "led.h"
#include "max7219.h"
#include "xa320.h"
#include "panel_rmp.h"

rmp_act_t rmp_act = RMP_VOR;

uint32_t nav1_freq = 11000;
uint32_t nav1_stdby_freq = 11100;
int32_t nav1_crs = 90;

uint32_t ndb_freq = 255;
uint32_t ndb_stdby_freq = 255;

uint32_t nav2_freq = 11000;
uint32_t nav2_stdby_freq = 11100;
int32_t nav2_crs = 90;

uint32_t com1_freq10 = 121700; // * 10 um 0.02, 0.05, 0.07, 0.10 darzustellen
uint32_t com1_stdby_freq10 = 121700;

uint32_t com2_freq10 = 121700; // * 10
uint32_t com2_stdby_freq10 = 121700;

uint32_t avionics_power = 99; // must be != 0|1 to be sent on startup

int32_t autop_heading = 90;
int32_t autop_alt = 2000;

int32_t airspeed = 999;
int32_t variometer = -999;
uint32_t course = 360;

uint32_t adf_dme = 0;
uint32_t nav1_dme = 0;
uint32_t nav2_dme = 0;

int32_t nav1_hdef_dots10 = 25;
int32_t nav1_vdef_dots10 = 25;
int32_t nav2_hdef_dots10 = 25;

void panel_rmp_cb(uint8_t id, uint32_t data);
void panel_rmp_ndb(void);
uint8_t panel_rmp_switch(void);
void panel_rmp_general(uint32_t *stdby, uint32_t *act, uint16_t comma_value, uint32_t high_step, uint32_t low_step, uint32_t max, uint32_t min, uint32_t id_stdby, uint32_t id_act);
void panel_rmp_general_single_float(int32_t *act, int32_t high_step, int32_t low_step, int32_t max, int32_t min, uint32_t id_act);
void panel_send_dial_commands(uint32_t large_up, uint32_t large_down, uint32_t small_up, uint32_t small_down);
void panel_set_led(void);
uint8_t disp_in_newdata(uint32_t freq, uint32_t freq_stdby, rmp_act_t act);

void str_add_dots(char *str, int dot10);
int strinsert(char **dest, char *ins, size_t location);

uint8_t panel_get_associated_led(uint8_t page);
int panel_rmp_setup_datarefs_connect(void);

rmp_act_t panel_rmp_get_active(void) {
	return rmp_act;
}


void panel_set_led(void) {
		switch(rmp_act) {
		case RMP_VOR:
		case RMP_VOR_CRS:
				led_set(LED_VOR, 1);
				break;
		case RMP_VOR2:
		case RMP_VOR2_CRS:
		case RMP_ILS:
				led_set(LED_ILS, 1);
				break;
		case RMP_ADF:
				led_set(LED_ADF, 1);
				break;
		case RMP_BFO:
		case RMP_BFO_ALT:
				led_set(LED_BFO, 1);
				break;
		case RMP_COM1:
				led_set(LED_HF1, 1);
				break;
		case RMP_COM2:
				led_set(LED_HF2, 1);
				break;
		case RMP_VHF1:
				led_set(LED_VHF1, 1);
				break;
		default:
				;
		}
}


uint8_t panel_get_associated_led(uint8_t page) {
	switch(page) {
			case RMP_COM1:
					return LED_HF1;
			case RMP_COM2:
					return LED_HF2;
			case RMP_VOR:
			case RMP_VOR_CRS:
					return LED_VOR;
			case RMP_ILS:
			case RMP_VOR2: /* MLS */
			case RMP_VOR2_CRS: /* MLS */
					return LED_ILS;
			case RMP_ADF:
					return LED_ADF;
			case RMP_BFO: /* Autopilot heading */
			case RMP_BFO_ALT: /* Autopilot height */
					return LED_BFO;
			case RMP_VHF1: /* Airplane speed, course and varia */
					return LED_VHF1;
			default:
					return -1;
	}
}


uint8_t disp_in_newdata(uint32_t freq, uint32_t freq_stdby, rmp_act_t act) {
	static uint32_t freq_last;
	static uint32_t freq_stdby_last;
	static rmp_act_t act_last;
	uint8_t new = 0;

	if (freq != freq_last)
		new = 1;
	else if (freq_stdby != freq_stdby_last)
		new = 1;
	else if (act != act_last)
		new = 1;

	freq_last = freq;
	freq_stdby_last = freq_stdby;
	act_last = act;

	return new;
}


int strinsert(char **dest, char *ins, size_t location)
{
	size_t origsize = 0;
	size_t resize = 0;
	size_t inssize = 0;

	if (!dest || !ins)
		return -1;  // invalid parameter

	if (strlen(ins) == 0)
		return -1; // invalid parameter

	location ++;
	origsize = strlen(*dest);
	inssize = strlen(ins);
	resize = strlen(*dest) + inssize + 1; // 1 for the null terminator

	if (location > origsize)
		return -1; // invalid location, out of original string

	// move string to make room for insertion
	memmove(&(*dest)[location+inssize], &(*dest)[location], origsize - location);
	(*dest)[origsize + inssize] = '\0'; // null terminate string

	// insert string
	memcpy(&(*dest)[location], ins, inssize);

	return resize; // return buffer size
}


void str_add_dots(char *str, int dot10) {
	// 2 points in middle as ok sign
	if (abs(dot10) < 5) {
		strinsert(&str, ".", 3);
		strinsert(&str, ".", 4 + 1);
	} else {
		if (abs(dot10) < 10) {
			strinsert(&str, ".", dot10 > 0 ? 4 : 3);
		} else if (abs(dot10) < 15) {
			strinsert(&str, ".", dot10 > 0 ? 4 : 3);
			strinsert(&str, ".", dot10 > 0 ? 5 + 1 : 2);
		} else if (abs(dot10) < 20) {
			strinsert(&str, ".", dot10 > 0 ? 4 : 3);
			strinsert(&str, ".", dot10 > 0 ? 5 + 1: 2);
			strinsert(&str, ".", dot10 > 0 ? 6 + 2: 1);
		} else  {
			strinsert(&str, ".", dot10 > 0 ? 4 : 3);
			strinsert(&str, ".", dot10 > 0 ? 5 + 1: 2);
			strinsert(&str, ".", dot10 > 0 ? 6 + 2: 1);
			strinsert(&str, ".", dot10 > 0 ? 7 + 3: 0);
		}
	}
}


void task_display(void) {
	char str[8 * 2 + 1]; // str must hold digit * 2 (for points) + null byte
	rmp_act_t act =  panel_rmp_get_active();
	static rmp_act_t act_last = RMP_OFF;
	uint32_t freq, freq_stdby;

	if (gpio_get_state(SWITCH_RMP_OFF)) {
		disp_in_newdata(-1, -1, RMP_OFF);
		max7219_ClearAll();
		return;	
	}

	if (act != act_last) {
		act_last = act;
		max7219_ClearAll();
	}

	switch(act) {
		case RMP_VOR:
			freq = panel_rmp_get_nav1_freq();
			freq_stdby = panel_rmp_get_nav1_stdby_freq();
			if (!disp_in_newdata(freq, freq_stdby, act))
				break;
			snprintf(str, 8, "%5d", (int)freq);
			max7219_display_string_fixpoint(3, str, 3);
			snprintf(str, 8, "%5d", (int)freq_stdby);
			max7219_display_string_fixpoint(8, str, 3);
			max7219_display_string(0, "V1");
		break;
		case RMP_VOR_CRS:
			freq = panel_rmp_get_nav1_freq();
			freq_stdby = panel_rmp_get_nav1_crs();
			snprintf(str, 9, "C-%03d %02d", (int)freq_stdby, (int)panel_rmp_get_nav1_dme());
			str_add_dots(str, (int)panel_rmp_get_nav1_hdef_dots10());
			max7219_display_string(8, str);
			//max7219_display_string(14, str);
			//if (!disp_in_newdata(freq_stdby, 0, act))
			//	break;
			//max7219_display_string_fixpoint(3, "     ", 99);
			snprintf(str, 8, "%5d", (int)freq);
			max7219_display_string_fixpoint(3, str, 3);
			max7219_display_string(0, "V1");
		break;
		case RMP_ADF:
			freq = panel_rmp_get_ndb_freq();
			freq_stdby = panel_rmp_get_ndb_stdby_freq();
			snprintf(str, 3, "%02d", (int)panel_rmp_get_adf_dme());
			max7219_display_string(14, str);
			if (!disp_in_newdata(freq, freq_stdby, act))
				break;
			snprintf(str, 8, "%5d", (int)freq);
			max7219_display_string_fixpoint(3, str, 99);
			snprintf(str, 8, "%5d", (int)freq_stdby);
			max7219_display_string_fixpoint(8, str, 99);
			max7219_display_string(0, "AD");
		break;
		case RMP_VOR2:
			freq = panel_rmp_get_nav2_freq();
			freq_stdby = panel_rmp_get_nav2_stdby_freq();
			if (!disp_in_newdata(freq, freq_stdby, act))
				break;
			snprintf(str, 8, "%5d", (int)freq);
			max7219_display_string_fixpoint(3, str, 3);
			snprintf(str, 8, "%5d", (int)freq_stdby);
			max7219_display_string_fixpoint(8, str, 3);
			max7219_display_string(0, "V2");
		break;
		case RMP_VOR2_CRS:
			freq = panel_rmp_get_nav2_freq();
			freq_stdby = panel_rmp_get_nav2_crs();
			snprintf(str, 9, "C-%03d %02d", (int)freq_stdby, (int)panel_rmp_get_nav2_dme());
			str_add_dots(str, (int)panel_rmp_get_nav2_hdef_dots10());
			max7219_display_string(8, str);
			max7219_display_string_fixpoint(3, "     ", 99);
			snprintf(str, 8, "%5d", (int)freq);
			max7219_display_string_fixpoint(3, str, 3);
			max7219_display_string(0, "V2");
		break;
		case RMP_COM1:
			freq = panel_rmp_get_com1_freq() / 10;
			freq_stdby = panel_rmp_get_com1_stdby_freq() / 10;
			if (!disp_in_newdata(freq, freq_stdby, act))
				break;
			snprintf(str, 8, "%5d", (int)freq);
			max7219_display_string_fixpoint(3, str, 3);
			snprintf(str, 8, "%5d", (int)freq_stdby);
			max7219_display_string_fixpoint(8, str, 3);
			max7219_display_string(0, "C1");
		break;
		case RMP_COM2:
			freq = panel_rmp_get_com2_freq() / 10;
			freq_stdby = panel_rmp_get_com2_stdby_freq() / 10;
			if (!disp_in_newdata(freq, freq_stdby, act))
				break;
			snprintf(str, 8, "%5d", (int)freq);
			max7219_display_string_fixpoint(3, str, 3);
			snprintf(str, 8, "%5d", (int)freq_stdby);
			max7219_display_string_fixpoint(8, str, 3);
			max7219_display_string(0, "C2");
		break;
		case RMP_BFO:
			freq_stdby = panel_rmp_get_autop_heading();
			freq = panel_rmp_get_autop_alt();
			if (!disp_in_newdata(freq_stdby, 0, act))
				break;
			max7219_display_string_fixpoint(3, "     ", 99);
			snprintf(str, 7, " C-%03d", (int)freq_stdby);
			max7219_display_string_fixpoint(7, str, 99);
			snprintf(str, 8, "A%5d", (int)freq);
			max7219_display_string_fixpoint(2, str, 99);
			max7219_display_string(0, "AH");
		break;
		case RMP_BFO_ALT:
			freq_stdby = panel_rmp_get_autop_alt();
			freq = panel_rmp_get_autop_heading();
			if (!disp_in_newdata(freq_stdby, 0, act))
				break;
			max7219_display_string_fixpoint(3, "     ", 99);
			snprintf(str, 8, "%5d", (int)freq_stdby);
			max7219_display_string_fixpoint(8, str, 99);
			snprintf(str, 7, " C-%03d", (int)freq);
			max7219_display_string_fixpoint(2, str, 99);
			max7219_display_string(0, "AA");
		break;
		case RMP_VHF1:
			snprintf(str, 7, "S-%4d", (int)panel_rmp_get_aircraft_speed());
			max7219_display_string(0, str);
			snprintf(str, 5, "C%3d", (int)panel_rmp_get_aircraft_course());
			max7219_display_string(7, str);
			int vario = panel_rmp_get_aircraft_variometer();
			if (vario > 0)
				snprintf(str, 6, "+%04d", vario);
			else
				snprintf(str, 6, "-%04d", abs(vario));
			max7219_display_string(11, str);
		break;
		default:
			if (!disp_in_newdata(0, 0, act))
				break;
			snprintf(str, 8, "%5d", (int)0);
			max7219_display_string_fixpoint(3, str, 3);
			snprintf(str, 8, "%5d", (int)0);
			max7219_display_string_fixpoint(8, str, 3);
			max7219_display_string(0, "--");
		break;
	}
}


uint8_t panel_rmp_switch(void) {
	rmp_act_t new = RMP_OFF;
	rmp_act_t last = rmp_act;
	uint8_t lednr;

	if (gpio_get_pos_event(SWITCH_VOR)) {
		if (last == RMP_VOR)
			new = RMP_VOR_CRS;
		else
			new = RMP_VOR;
	/*} else if (gpio_get_pos_event(SWITCH_ILS)) {
		new = RMP_ILS; */
	} else if (/*gpio_get_pos_event(SWITCH_VOR2) ||*/ gpio_get_pos_event(SWITCH_ILS)) { // VOR2 = MLS is short circuit to ADF
		if (last == RMP_VOR2)
			new = RMP_VOR2_CRS;
		else
			new = RMP_VOR2;
	} else if (gpio_get_pos_event(SWITCH_ADF)) {
		new = RMP_ADF;
	} else if (gpio_get_pos_event(SWITCH_BFO) ||
			  ((last == RMP_BFO || last == RMP_BFO_ALT) && gpio_get_pos_event(SWITCH_SW_STBY))) {
		if (last == RMP_BFO)
			new = RMP_BFO_ALT;
		else
			new = RMP_BFO;
	} else if (gpio_get_pos_event(SWITCH_COM1)) {
		new = RMP_COM1;
	} else if (gpio_get_pos_event(SWITCH_COM2)) {
		new = RMP_COM2;
	} else if (gpio_get_pos_event(SWITCH_NAV)) {
			led_set(LED_SEL, 1);
			return 0; // nothing to do here
	} else if (gpio_get_pos_event(SWITCH_VHF1)) { 
		new = RMP_VHF1;
	} else {
		return 0; // No switch pressed
	}

	rmp_act = new;
	lednr = panel_get_associated_led(last);

	if (new != last) {
		if (lednr > 0)
				led_clear(lednr);
		panel_set_led();
	}

	return new != last;
}

void task_panel_rmp(void) {
	panel_rmp_switch();

	switch (rmp_act) {
			case RMP_OFF:
			break;
			case RMP_VOR:
				panel_rmp_general(&nav1_stdby_freq,
				                  &nav1_freq,
				                  100,
				                  100,
				                  5,
				                  11800,
				                  10800,
				                  ID_NAV1_STDBY_FREQ,
				                  ID_NAV1_FREQ);
			break;
			case RMP_VOR_CRS:
				panel_rmp_general_single_float(&nav1_crs,
				                  30,
				                  1,
				                  359,
				                  0,
				                  ID_NAV1_CRS);
			break;
			case RMP_ADF:
				panel_rmp_ndb();
			break;
			case RMP_VOR2:
				//panel_rmp_nav2();
				panel_rmp_general(&nav2_stdby_freq,
				                  &nav2_freq,
				                  100,
				                  100,
				                  5,
				                  11800,
				                  10800,
				                  ID_NAV2_STDBY_FREQ,
				                  ID_NAV2_FREQ);
			break;
			case RMP_VOR2_CRS:
				panel_rmp_general_single_float(&nav2_crs,
				                  30,
				                  1,
				                  359,
				                  0,
				                  ID_NAV2_CRS);
			break;
			case RMP_COM1:
#ifdef QPAC_A320
				panel_send_dial_commands(ID_COM1_LARGE_UP, ID_COM1_LARGE_DOWN, ID_COM1_SMALL_UP, ID_COM1_SMALL_DOWN);
#else
				panel_rmp_general(&com1_stdby_freq10,
				                  &com1_freq10,
				                  1000,
				                  1000,
				                  25,
				                  136000,
				                  118000,
				                  ID_COM1_STDBY_FREQ,
				                  ID_COM1_FREQ);
#endif
			break;
			case RMP_COM2:
				panel_rmp_general(&com2_stdby_freq10,
				                  &com2_freq10,
				                  1000,
				                  1000,
				                  25,
				                  136000,
				                  118000,
				                  ID_COM2_STDBY_FREQ,
				                  ID_COM2_FREQ);
			break;
			case RMP_BFO:
				panel_rmp_general_single_float(&autop_heading,
				                  30,
				                  1,
				                  359,
				                  0,
				                  ID_AUTOP_HEADING);
			break;
			case RMP_BFO_ALT:
				panel_rmp_general_single_float(&autop_alt,
				                  500,
				                  10,
				                  90000,
				                  0,
				                  ID_AUTOP_ALT);
			break;
			case RMP_VHF1:
			break;
			default:
			break;
	}

}

void panel_rmp_general_single_float(int32_t *act, int32_t high_step, int32_t low_step, int32_t max, int32_t min, uint32_t id_act) {
	int16_t enc_high = encoder_read(ENC_B, 0);
	int16_t enc_low = encoder_read(ENC_A, 0);
	int32_t last = *act;
	int32_t tmp = *act;
	float f;

	if (enc_low) {
		tmp += enc_low * low_step;	// 5kHz
		if (tmp < min) tmp = max;
		if (tmp > max) tmp = min;
		*act = tmp;
	} else if (enc_high) {
		tmp += enc_high * high_step;	// 1MHz
		if (tmp < min) {
			while (tmp + high_step < max) tmp += high_step;
		}
		*act = tmp;
		if (*act > max) {
			tmp = *act % high_step;
			*act = min + tmp;
		}
	}

	f = *act;
	if (*act != last)
		teensy_send_float(id_act, f);
}

void panel_rmp_general(uint32_t *stdby, uint32_t *act, uint16_t comma_value, uint32_t high_step, uint32_t low_step, uint32_t max, uint32_t min, uint32_t id_stdby, uint32_t id_act) {
	int16_t enc_high = encoder_read(ENC_B, 0);
	int16_t enc_low = encoder_read(ENC_A, 0);
	int16_t hv;
	int16_t tmp;

	if (enc_low) {
		hv = *stdby / comma_value;
		tmp = *stdby % comma_value;

		tmp += enc_low * low_step;	// 5kHz

		// lap to 0-995kHz range
		if (tmp < 0) tmp += comma_value - low_step;
		if (tmp >= comma_value) tmp = 0;

		*stdby = hv * comma_value + tmp;
		if (*stdby > max) *stdby = max;

		teensy_send_int(id_stdby, *stdby);
	}
	if (enc_high) {
		*stdby += enc_high * high_step;	// 1MHz

		// lap to 108-118MHz range
		while (*stdby < min) *stdby += high_step;
		while (*stdby >= max) *stdby -= high_step;

		teensy_send_int(id_stdby, *stdby);
	}

	if (gpio_get_pos_event(SWITCH_SW_STBY)) {
		uint32_t t = *act;
		*act = *stdby;
		*stdby = t;
		teensy_send_int(id_act, *act);
		teensy_send_int(id_stdby, *stdby);
	}
}

void panel_send_dial_commands(uint32_t large_up, uint32_t large_down, uint32_t small_up, uint32_t small_down) {
	int16_t enc_high = encoder_read(ENC_B, 0);
	int16_t enc_low = encoder_read(ENC_A, 0);

	if (enc_high)
		teensy_send_command_once(enc_high > 0 ? large_up : large_down);
	
	if (enc_low)
		teensy_send_command_once(enc_low > 0 ? small_up : small_down);
}

void panel_rmp_ndb(void) {
	uint8_t coarse = 0;
	int16_t enc_high = encoder_read(ENC_B, 0);
	int16_t enc_low = encoder_read(ENC_A, &coarse);
	if (enc_low || enc_high) {
		if (enc_low) {
			if (coarse)
				ndb_stdby_freq += enc_low > 0 ? 10 : -10; // 10kHz
			else
				ndb_stdby_freq += enc_low;	// 1kHz

			if (ndb_stdby_freq > 525) ndb_stdby_freq = 525;
			if (ndb_stdby_freq < 200) ndb_stdby_freq = 200;
		}
		if (enc_high)
			ndb_stdby_freq += enc_high * 100;

		while (ndb_stdby_freq < 200) ndb_stdby_freq += 100;
		while (ndb_stdby_freq > 525) ndb_stdby_freq -= 100;

		teensy_send_int(ID_NDB_STDBY_FREQ, ndb_stdby_freq);
	}

	if (gpio_get_pos_event(SWITCH_SW_STBY)) {
		uint32_t t = ndb_freq;
		ndb_freq = ndb_stdby_freq;
		ndb_stdby_freq = t;
		teensy_send_int(ID_NDB_STDBY_FREQ, ndb_stdby_freq);
		teensy_send_int(ID_NDB_FREQ, ndb_freq);
	}
}

uint32_t panel_rmp_get_nav1_freq(void) {
	return nav1_freq;
}

uint32_t panel_rmp_get_nav1_stdby_freq(void) {
	return nav1_stdby_freq;
}

uint32_t panel_rmp_get_nav1_crs(void) {
	return nav1_crs;
}

uint32_t panel_rmp_get_ndb_freq(void) {
	return ndb_freq;
}

uint32_t panel_rmp_get_ndb_stdby_freq(void) {
	return ndb_stdby_freq;
}

uint32_t panel_rmp_get_nav2_freq(void) {
	return nav2_freq;
}

uint32_t panel_rmp_get_nav2_stdby_freq(void) {
	return nav2_stdby_freq;
}

uint32_t panel_rmp_get_nav2_crs(void) {
	return nav2_crs;
}

uint32_t panel_rmp_get_com1_freq(void) {
	return com1_freq10;
}

uint32_t panel_rmp_get_com1_stdby_freq(void) {
	return com1_stdby_freq10;
}

uint32_t panel_rmp_get_com2_freq(void) {
	return com2_freq10;
}

uint32_t panel_rmp_get_com2_stdby_freq(void) {
	return com2_stdby_freq10;
}

void panel_rmp_set_avionics_power(uint32_t on) {
	if (!xa320_datarefs_ready())
		return;

	if (on != avionics_power) {
		avionics_power = on;
		teensy_send_int(ID_AVIONICS_POWER, avionics_power);
	}
}

uint32_t panel_rmp_get_autop_heading(void) {
	return autop_heading;
}

uint32_t panel_rmp_get_autop_alt(void) {
	return autop_alt;
}

int32_t panel_rmp_get_aircraft_speed(void) {
	return airspeed;
}

uint32_t panel_rmp_get_aircraft_course(void) {
	return course;
}

int32_t panel_rmp_get_aircraft_variometer(void) {
	return variometer;
}

uint32_t panel_rmp_get_adf_dme(void) {
	return adf_dme > 99 ? 99 : adf_dme;
}

uint32_t panel_rmp_get_nav1_dme(void) {
	return nav1_dme > 99 ? 99 : nav1_dme;
}

uint32_t panel_rmp_get_nav2_dme(void) {
	return nav2_dme > 99 ? 99 : nav2_dme;
}

int32_t panel_rmp_get_nav1_hdef_dots10(void) {
	return nav1_hdef_dots10;
}

int32_t panel_rmp_get_nav1_vdef_dots10(void) {
	return nav1_vdef_dots10;
}

int32_t panel_rmp_get_nav2_hdef_dots10(void) {
	return nav2_hdef_dots10;
}


void panel_rmp_setup_datarefs(void) {
		teensy_register_dataref(ID_AIRCRAFT_TYPE, "sim/aircraft/view/acf_ICAO", 1, &panel_rmp_connect_cb);
		teensy_register_dataref(ID_STROBE_LIGHT, "sim/cockpit/electrical/strobe_lights_on", 1, &panel_rmp_cb);
		teensy_register_dataref(ID_NAV_LIGHT, "sim/cockpit/electrical/nav_lights_on", 1, &panel_rmp_cb);
		teensy_register_dataref(ID_NAV1_FREQ, "sim/cockpit2/radios/actuators/nav1_frequency_hz", 1, &panel_rmp_cb);
		teensy_register_dataref(ID_NAV1_STDBY_FREQ, "sim/cockpit2/radios/actuators/nav1_standby_frequency_hz", 1, &panel_rmp_cb);
		teensy_register_dataref(ID_NAV1_CRS, "sim/cockpit/radios/nav1_obs_degm", 2, &panel_rmp_cb);
		teensy_register_dataref(ID_NDB_FREQ, "sim/cockpit2/radios/actuators/adf1_frequency_hz", 1, &panel_rmp_cb);
		teensy_register_dataref(ID_NDB_STDBY_FREQ, "sim/cockpit2/radios/actuators/adf1_standby_frequency_hz", 1, &panel_rmp_cb);
		teensy_register_dataref(ID_NAV2_FREQ, "sim/cockpit2/radios/actuators/nav2_frequency_hz", 1, &panel_rmp_cb);
		teensy_register_dataref(ID_NAV2_STDBY_FREQ, "sim/cockpit2/radios/actuators/nav2_standby_frequency_hz", 1, &panel_rmp_cb);
		teensy_register_dataref(ID_NAV2_CRS, "sim/cockpit/radios/nav2_obs_degm", 2, &panel_rmp_cb);
		//teensy_register_dataref(ID_COM1_FREQ, "sim/cockpit2/radios/actuators/com1_frequency_hz_833", 1, &panel_rmp_cb);
		teensy_register_dataref(ID_COM1_FREQ, "*2/radios/actuators/com1_frequency_hz_833", 1, &panel_rmp_cb);
		teensy_register_dataref(ID_COM1_STDBY_FREQ, "*2/radios/actuators/com1_standby_frequency_hz_833", 1, &panel_rmp_cb);
		teensy_register_dataref(ID_COM2_FREQ, "*2/radios/actuators/com2_frequency_hz_833", 1, &panel_rmp_cb);
		teensy_register_dataref(ID_COM2_STDBY_FREQ, "*2/radios/actuators/com2_standby_frequency_hz_833", 1, &panel_rmp_cb);
		teensy_register_dataref(ID_AVIONICS_POWER, "sim/cockpit2/switches/avionics_power_on", 1, &panel_rmp_cb);
		teensy_register_dataref(ID_AUTOP_HEADING, "sim/cockpit2/autopilot/heading_dial_deg_mag_pilot", 2, &panel_rmp_cb);
		teensy_register_dataref(ID_AUTOP_ALT, "sim/cockpit/autopilot/altitude", 2, &panel_rmp_cb);
#ifdef QPAC_A320
		teensy_register_dataref(ID_COM1_STDBY_FREQ,   "AirbusFBW/RMP1StbyFreq", 1, &panel_rmp_cb);
		teensy_register_dataref(ID_COM1_LARGE_UP, "AirbusFBW/RMP1FreqUpLrg", 0, &panel_rmp_cb);
		teensy_register_dataref(ID_COM1_LARGE_DOWN, "AirbusFBW/RMP1FreqDownLrg", 0, &panel_rmp_cb);
		teensy_register_dataref(ID_COM1_SMALL_UP, "AirbusFBW/RMP1FreqUpSml", 0, &panel_rmp_cb);
		teensy_register_dataref(ID_COM1_SMALL_DOWN, "AirbusFBW/RMP1FreqDownSml", 0, &panel_rmp_cb);
#else
#endif
		teensy_register_dataref(ID_AIRCRAFT_AIRSPEED, "sim/cockpit2/gauges/indicators/airspeed_kts_pilot", 2, &panel_rmp_cb);
		teensy_register_dataref(ID_AIRCRAFT_VARIO, "sim/cockpit2/gauges/indicators/vvi_fpm_pilot", 2, &panel_rmp_cb);
//		teensy_register_dataref(ID_AIRCRAFT_COURSE, "sim/cockpit2/gauges/indicators/compass_heading_deg_mag", 2, &panel_rmp_cb);

		teensy_register_dataref(ID_ADF_DME, "sim/cockpit2/radios/indicators/adf1_dme_distance_nm", 2, &panel_rmp_cb);
		teensy_register_dataref(ID_NAV1_DME, "sim/cockpit2/radios/indicators/nav1_dme_distance_nm", 2, &panel_rmp_cb);
		teensy_register_dataref(ID_NAV2_DME, "sim/cockpit2/radios/indicators/nav2_dme_distance_nm", 2, &panel_rmp_cb);
		
		teensy_register_dataref(ID_NAV1_HDEF_DOTS10, "sim/cockpit2/radios/indicators/nav1_hdef_dots_pilot", 2, &panel_rmp_cb);
		teensy_register_dataref(ID_NAV1_VDEF_DOTS10, "sim/cockpit2/radios/indicators/nav1_vdef_dots_pilot", 2, &panel_rmp_cb);
		teensy_register_dataref(ID_NAV2_HDEF_DOTS10, "sim/cockpit2/radios/indicators/nav2_hdef_dots_pilot", 2, &panel_rmp_cb);
}


void panel_rmp_connect_cb(uint8_t id, uint32_t data) {
		id = id;
		data = data;
}


void panel_rmp_cb(uint8_t id, uint32_t data) {
	switch(id) {
		case ID_STROBE_LIGHT:
				//gpio_set_led(LED5, !!data);
				break;
		case ID_NAV_LIGHT:
				//gpio_set_led(LED4, !!data);
				break;
		case ID_NAV1_FREQ:
				nav1_freq = data;
				break;
		case ID_NAV1_STDBY_FREQ:
				nav1_stdby_freq = data;
				break;
		case ID_NAV1_CRS:
				nav1_crs = teensy_from_float(data);
				break;
		case ID_NDB_FREQ:
				ndb_freq = data;
				break;
		case ID_NDB_STDBY_FREQ:
				ndb_stdby_freq = data;
				break;
		case ID_NAV2_FREQ:
				nav2_freq = data;
				break;
		case ID_NAV2_STDBY_FREQ:
				nav2_stdby_freq = data;
				break;
		case ID_NAV2_CRS:
				nav2_crs = data;
				break;
		case ID_COM1_FREQ:
				com1_freq10 = data;
				break;
		case ID_COM1_STDBY_FREQ:
				com1_stdby_freq10 = data;
				break;
		case ID_COM2_FREQ:
				com2_freq10 = data;
				break;
		case ID_COM2_STDBY_FREQ:
				com2_stdby_freq10 = data;
				break;
		case ID_AVIONICS_POWER:
				//avionics_power_on = data;
				break;
		case ID_AIRCRAFT_AIRSPEED:
				airspeed = teensy_from_float(data);
				break;
		case ID_AIRCRAFT_VARIO:
				variometer = teensy_from_float(data);
				break;
		case ID_AIRCRAFT_COURSE:
				course = teensy_from_float(data);
				break;
		case ID_ADF_DME:
				adf_dme = teensy_from_float(data);
				break;
		case ID_NAV1_DME:
				nav1_dme = teensy_from_float(data);
				break;
		case ID_NAV2_DME:
				nav2_dme = teensy_from_float(data);
				break;
		case ID_NAV1_HDEF_DOTS10:
				nav1_hdef_dots10 = 10 * teensy_from_float(data);
				break;
		case ID_NAV1_VDEF_DOTS10:
				nav1_vdef_dots10 = 10 * teensy_from_float(data);
				break;
		case ID_NAV2_HDEF_DOTS10:
				nav2_hdef_dots10 = 10 * teensy_from_float(data);
				break;
	}
}


void panel_rmp_setup(void)
{
	rmp_act = RMP_VOR;
	led_set(LED_VOR, 1);
	gpio_set_led(LED5, 0);
	gpio_set_led(LED6, 1);
}
