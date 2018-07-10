#ifndef XA320_H
#define XA320_H


#define ID_AIRCRAFT_TYPE   0
#define ID_STROBE_LIGHT    1
#define ID_NAV_LIGHT       2
#define ID_NAV1_FREQ       3
#define ID_NAV1_STDBY_FREQ 4
#define ID_NAV1_CRS        5
#define ID_NDB_FREQ        6
#define ID_NDB_STDBY_FREQ  7
#define ID_NAV2_FREQ       8
#define ID_NAV2_STDBY_FREQ 9
#define ID_NAV2_CRS        10
#define ID_COM1_FREQ       11
#define ID_COM1_STDBY_FREQ 12
#define ID_COM2_FREQ       13
#define ID_COM2_STDBY_FREQ 14
#define ID_AVIONICS_POWER  15
#define ID_AUTOP_HEADING   16
#define ID_AUTOP_ALT       17
#define ID_COM1_LARGE_UP   18
#define ID_COM1_LARGE_DOWN 19
#define ID_COM1_SMALL_UP   20
#define ID_COM1_SMALL_DOWN 21
#define ID_AIRCRAFT_AIRSPEED 22
#define ID_AIRCRAFT_VARIO 23
#define ID_AIRCRAFT_COURSE 24
#define ID_ADF_DME         25
#define ID_NAV1_DME        26
#define ID_NAV2_DME        27
#define ID_NAV1_HDEF_DOTS10 28
#define ID_NAV1_VDEF_DOTS10 29
#define ID_NAV2_HDEF_DOTS10 30

uint8_t xa320_datarefs_ready(void);
uint8_t xa320_xplane_ready(void);

#endif

