#ifndef PANEL_RMP_H
#define PANEL_RMP_H

void panel_rmp_setup_datarefs(void);
void task_panel_rmp(void);

uint32_t panel_rmp_get_nav1_freq(void);
uint32_t panel_rmp_get_nav1_stdby_freq(void);

#endif /* PANEL_RMP_H */
