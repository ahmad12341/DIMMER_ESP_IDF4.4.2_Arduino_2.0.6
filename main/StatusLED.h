#pragma once
#ifndef STATUS_LED_H_
#define STATUS_LED_H_
#include <Arduino.h>
#include "Globals.h"

#define STATUS_LED_PIN 2

void status_led_init();
void status_led_loop();

#endif