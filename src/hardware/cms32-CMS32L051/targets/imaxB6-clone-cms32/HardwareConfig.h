
#ifndef HARDWARE_CONFIG_H_
#define HARDWARE_CONFIG_H_

#include "GlobalConfig.h"
#include "HardwareConfigGeneric.h"
#include "imaxB6-pins.h"

#define MAX_CHARGE_V            ANALOG_VOLT(27.000)
#define MAX_CHARGE_I            ANALOG_AMP(5.000)
#define MAX_CHARGE_P            ANALOG_WATT(50.000)

#define MAX_DISCHARGE_P         ANALOG_WATT(5.000)
#define MAX_DISCHARGE_I         ANALOG_AMP(1.000)

#define SMPS_UPPERBOUND_VALUE               (60000)
#define DISCHARGER_UPPERBOUND_VALUE         32760

#endif /* HARDWARE_CONFIG_H_ */
