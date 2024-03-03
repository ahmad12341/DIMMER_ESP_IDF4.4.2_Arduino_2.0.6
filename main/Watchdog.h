#pragma once
#ifndef WATCHDOG_H_
#define WATCHDOG_H_
#include <Arduino.h>
#include "PubSubClient.h"
#include "Globals.h"
#include "FilesManager.h"
#include "Logging.h"

void IRAM_ATTR watchdog_woof();
void watchdog_init();
void watchdog_init_mqtt();
void watchdog_feed();
#endif