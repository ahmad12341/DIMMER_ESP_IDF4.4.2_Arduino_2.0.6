#pragma once
#ifndef OTA_H_
#define OTA_H_
#include <Arduino.h>
#include <HTTPClient.h>
#include "Globals.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "FilesManager.h"
#include "MqttHandler.h"
#include "BroadcastManager.h"
#include "WifiManager.h"
#include "esp_ota_ops.h"
#include "Watchdog.h"
#include "Logging.h"
#include "esp_https_ota.h"
#include <map>
#include "esp_task_wdt.h"

void ota_broadcast();
void restart_broadcast();

enum ota_errors{
    SUCCESS = 0,
    NVS_ERASE_FAILED,
    NVS_INIT_FAILED,
    NO_WIFI,
    PARTITIONS_CORRUPTED, 
    NULL_PARTITION,
    HTTP_INIT_FAILED,
    HTTP_GET_FAILED,
    HTTP_SIZE_ZERO,
    HTTP_INVALID_STATUS,
    ESP_OTA_BEGIN_FAILED,
    ESP_OTA_WRITE_FAILED,
    ESP_OTA_END_FAILED,
    ESP_OTA_SET_BOOT_FAILED,
    HTTP_STREAM_DISCONNECTED,
    HTTP_STREAM_RESTART_FAILED,
    OTA_TIMEOUT,
};


class OTA{
    public:
        OTA(){};
        ~OTA();
        bool begin(const char* url);
    private:
        void set_mutable_url(const char* const_url);
        int32_t http_get_with_retries();

        ota_errors wifi_status_check();
        ota_errors nvs_init_check();
        ota_errors partitions_corruption_check();
        ota_errors init_http_client();
        ota_errors get_ota_size();
        ota_errors get_update_partition();
        ota_errors check_all_data_valid();
        ota_errors check_set_boot_partition();
        ota_errors init_esp_ota();
        ota_errors ota_process();
        ota_errors restart_with_range();
        ota_errors download_chunks();
        ota_errors store_chunk();

        static uint8_t init_flag;

        WiFiClient* _ota_client = NULL;
        HTTPClient* _http = NULL;
        char* url = NULL;


        const esp_partition_t* update_partition;
        esp_ota_handle_t update_handle = 0;

        int32_t update_size = 0;
        int32_t update_size_save = 0;

        char ota_write_data[OTA_BUFFSIZE + 1] = { 0 };

        WiFiClient* stream; // This is just ota_client..
        size_t avail_size;
        int32_t total_size = 0;
        int16_t counter = 0;
        uint16_t stream_ret;
        uint32_t start_update_time = millis();
        uint32_t packet_update_time = millis();
        uint32_t timeDiff = 0;
        uint32_t download_stalled_timeout = 10 * 1000; // 5 seconds for now, placeholder
        uint32_t main_download_timeout = 5 * 60 * 1000; // 5 minute timeout..
};


#endif