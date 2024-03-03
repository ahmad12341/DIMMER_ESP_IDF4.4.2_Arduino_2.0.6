#pragma once
#ifndef MQTT_HANDLER_H_
#define MQTT_HANDLER_H_
#include <Arduino.h>
#include "Globals.h"
#include "RealTimeClock.h"
#include "ServerComm.h"
#include "RelayController.h"
#include "Logging.h"

void mqtt_init();
void mqtt_set_server();
void mqtt_loop();
void mqtt_connect_task();
void mqtt_send_periodic_status();
void mqtt_callback(char* topic, byte* payload, unsigned int length);
uint8_t mqtt_msg_channel(char* topic);
void mqtt_config_callback(char* topic, byte* payload, unsigned int length);
void mqtt_connect_toserver(void* parameter);
uint8_t mqtt_publish_msg(char* str, uint16_t data_len);
uint8_t mqtt_publish_msg(String str);
void mqtt_send_config_request();
void mqtt_publish_msg(int32_t* val);
#endif