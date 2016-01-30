#ifndef PANEL_RMP_H
#define PANEL_RMP_H

void panel_rmp_setup_datarefs(void);
void task_panel_rmp(void);

typedef enum {
	RMP_OFF = 0,
	RMP_VOR,
	RMP_VOR_CRS,
	RMP_ILS,
	RMP_VOR2, /* MLS */
	RMP_VOR2_CRS, /* MLS */
	RMP_ADF,
	RMP_BFO,
	RMP_COM1,
	RMP_COM2
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
#endif /* PANEL_RMP_H */
