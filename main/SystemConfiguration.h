#pragma once
#ifndef SYSTEM_CONFIGURATION_H_
#define SYSTEM_CONFIGURATION_H_
#include <Arduino.h>
#include "Globals.h"
#include "FilesManager.h"
#include "RealTimeClock.h"
#include "WifiManager.h"
#include "MqttHandler.h"
#include "Logging.h"

void system_config_check_config_request();
String system_config_get_all_config();
String system_config_get_by_params(String parameters[], int16_t params_length);
void system_config_set_by_params(String parameters[], int16_t params_length);
String system_config_get_value_by_name(String param_name, uint8_t* flag);
void system_config_set_value_by_name(String param_name, String param_value);
#endif