#ifndef PANEL_RMP_H
#define PANEL_RMP_H

void panel_rmp_setup_datarefs(void);
void task_panel_rmp(void);
void panel_rmp_setup(void);

typedef enum {
	RMP_OFF = 0,
	RMP_VOR,
	RMP_VOR_CRS,
	RMP_ILS,
	RMP_VOR2, /* MLS */
	RMP_VOR2_CRS, /* MLS */
	RMP_ADF,
	RMP_BFO, /* Autopilot heading */
	RMP_BFO_ALT, /* Autopilot height */
	RMP_COM1,
	RMP_COM2,
	RMP_VHF1 /* aircraft speed, course, vario */
} rmp_act_t;

rmp_act_t panel_rmp_get_active(void);

uint32_t panel_rmp_get_nav1_freq(void);
uint32_t panel_rmp_get_nav1_stdby_freq(void);
uint32_t panel_rmp_get_nav1_crs(void);

uint32_t panel_rmp_get_ndb_freq(void);
uint32_t panel_rmp_get_ndb_stdby_freq(void);

uint32_t panel_rmp_get_nav2_freq(void);
uint32_t panel_rmp_get_nav2_stdby_freq(void);
uint32_t panel_rmp_get_nav2_crs(void);

uint32_t panel_rmp_get_com1_freq(void);
uint32_t panel_rmp_get_com1_stdby_freq(void);

uint32_t panel_rmp_get_com2_freq(void);
uint32_t panel_rmp_get_com2_stdby_freq(void);

uint32_t panel_rmp_get_autop_heading(void);
uint32_t panel_rmp_get_autop_alt(void);

int32_t panel_rmp_get_aircraft_speed(void);
uint32_t panel_rmp_get_aircraft_course(void);
int32_t panel_rmp_get_aircraft_variometer(void);

uint32_t panel_rmp_get_adf_dme(void);
uint32_t panel_rmp_get_nav1_dme(void);
uint32_t panel_rmp_get_nav2_dme(void);

int32_t panel_rmp_get_nav1_hdef_dots10(void);
int32_t panel_rmp_get_nav1_vdef_dots10(void);
int32_t panel_rmp_get_nav2_hdef_dots10(void);

// Setter
void panel_rmp_set_avionics_power(uint32_t on);
#endif /* PANEL_RMP_H */
