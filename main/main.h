#pragma once
#define USE_ESP_IDF_LOG 1

#include <Arduino.h>
#include <stdint.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include "SparkFun_SHTC3.h" 


#include <HTTPClient.h>

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_flash_partitions.h"
#include "esp_partition.h"
#include "BluetoothSerial.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "mqtt_client.h"

#include <WiFiUdp.h>

#include "SPIFFS.h"

#include "Globals.h"
#include "PubSubClient.h"
#include "WifiWebPages.h"
#include "HLW8012.h"

#include "BroadcastManager.h"
#include "Buzzer.h"
#include "CurrentSensor.h"
#include "FilesManager.h"
#include "MqttHandler.h"
#include "Ota.h"
#include "RealTimeClock.h"
#include "RelayController.h"
#include "ServerComm.h"
#include "Scheduler.h"
#include "SwitchInput.h"
#include "SystemConfiguration.h"
#include "UserHandler.h"
#include "Watchdog.h"
#include "WifiManager.h"
#include "Dimmer.h"

#include "DebugSocket.h"
#include "Logging.h"
void dimmer_task(void * pvParameters );
void main_task(void * pvParameters );