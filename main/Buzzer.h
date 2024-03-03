#pragma once
#ifndef BUZZER_H_
#define BUZZER_H_
#include <Arduino.h>
#include "Globals.h"
#include "Logging.h"

void buzzer_init();
void buzzer_loop();
void buzzer_beep_duration(int16_t beep_t);
void buzzer_beep_n_times(int16_t beep_count);

#endif