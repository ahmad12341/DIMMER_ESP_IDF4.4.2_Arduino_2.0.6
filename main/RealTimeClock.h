#pragma once
#ifndef RTC_H_
#define RTC_H_
#include <Arduino.h>
#include <HTTPClient.h>
#include "Globals.h"
#include "FilesManager.h"
#include "RelayController.h"
#include "Logging.h"

void rtc_init(void* parameter);
void rtc_init_task();
void rtc_init(String epoch_str);
void rtc_init(uint32_t epoch);
void rtc_loop();
void rtc_new_day(uint32_t new_day, uint32_t epoch);
void rtc_new_hour(uint32_t new_hour, uint32_t epoch);
uint32_t rtc_get_epoch();
String rtc_get_epoch_str();
void rtc_millis_process();
uint32_t rtc_millis();
int32_t rtc_get_day_num(uint32_t epoch);
uint32_t rtc_get_hour_num(uint32_t epoch);
int8_t rtc_get_weekday(uint32_t epoch);
int8_t rtc_check_token_timeout(uint32_t epoch_time);
DateTime rtc_get_date(uint32_t epoch);
void rtc_process_days_minutes_seconds(uint32_t epoch, DateTime* dt);
#endif