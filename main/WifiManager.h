#pragma once
#ifndef WIFI_MANAGER_H_
#define WIFI_MANAGER_H_
#include <Arduino.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include "Globals.h"
#include "MqttHandler.h"
#include "esp_wifi.h"
#include "Logging.h"

void wifi_setup();
int8_t wifi_hotspot_sta();
void wifi_load_sta_info();
void wifi_sta();
void wifi_connected_action();
String wifi_get_ip();
void webserver_loop();
void wifi_loop();
void wifi_mdns_init();
void wifi_hotspot_loop();
void wifi_connection_status_loop();
void webserver_init();
void webserver_send_jsresponse(char* msg);
void webserver_send_jsresponse(String msg);
void webserver_handle_root_get();
void webserver_handle_root_post();
void webserver_handle_get_info();
void webserver_handle_404();
#endif