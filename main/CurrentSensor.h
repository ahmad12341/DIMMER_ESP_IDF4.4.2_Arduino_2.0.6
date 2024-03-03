#pragma once
#ifndef CURRENT_SENSE_H_
#define CURRENT_SENSE_H_
#include <Arduino.h>
#include "Globals.h"
#include "RelayController.h"
#include "Logging.h"

#define HLW8012_CF_PIN				18
#define HLW8012_CF1_PIN				15
#define HLW8012_SEL_PIN				14
#define HLW8012_PULSE_TIMEOUT		2000000

#define HLW8012_CURRENT_RESISTOR	0.001f
#define HLW8012_RESISTOR_UPSTREAM	(5L * 470000L)
#define HLW8012_RESISTOR_DOWNSTREAM	1000

#define HLW8012_UPDATE_TIME			4000 // 4 seconds

// void ICACHE_RAM_ATTR hlw8012_cf_interrupt();
// void ICACHE_RAM_ATTR hlw8012_cf1_interrupt();
void current_sensor_init();
void current_sensor_loop();

#endif