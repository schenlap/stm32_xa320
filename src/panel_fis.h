// Flight Instrument System Panel
#ifndef PANEL_FIS_H
#define PANEL_FIS_H

void task_panel_fis(void);
void panel_fis_setup(void);
void panel_fis_setup_datarefs(void);

void panel_fis_cb(uint8_t id, uint32_t data);

#endif
