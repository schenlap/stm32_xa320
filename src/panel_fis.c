// Flight Instrument System Panel
#include <stdlib.h>
#include <stdio.h>

#include "task.h"
#include "systime.h"
#include "gpio.h"
#include "encoder.h"
#include "teensy.h"
#include "usb.h"
#include "servo.h"
#include "panel_fis.h"


void task_panel_fis(void) {
}


void panel_fis_cb(uint8_t id, uint32_t data) {
	switch(id) {
	}
}


void panel_fis_setup(void) {
}
