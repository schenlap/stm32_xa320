#ifndef __SERVO_DISPLAY_H_
#define __SERVO_DISPLAY_H_

#define SERVO_LINEAR ((servo_display_nonlinear_t*)0)
typedef struct servo_display_nonlinear_s {
	int32_t pwm;
	int32_t sim;
} servo_display_nonlinear_t;


typedef struct servo_display_t {
	uint8_t servo_id;
	uint16_t servo_min;
	uint16_t servo_max;
	int16_t servo_offset;
	servo_display_nonlinear_t *nl;
	int32_t simval_min;
	int32_t simval_max;
	int32_t sim_multi;
	uint8_t simval_type;
	uint16_t ref_id;
	char * ref;
	void (*cb)(uint8_t, uint32_t);
} servo_display_defs;

void servo_display_value(uint16_t id, int32_t data, servo_display_defs *s); // drive servo to sim value.
void servo_display_setup(void);

#endif
