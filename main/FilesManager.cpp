#include "FilesManager.h"

extern ServerComms server_comms;
extern Relays relays;

void files_manager_init() {
    if (!SPIFFS.begin(true)) {
        log_w("Error While Mounting SPIFFS");
        return;
    }
    files_manager_read_config();
    files_manager_read_program_status();
    system_config_check_config_request();
    files_manager_read_rtc();
    uint32_t val = esp_random();
    val = val % 999;
    SYSTEM_CONFIGURATION.apSSID = "dm_infinite_" + String(val);
}

int32_t files_manager_get_free_space() {
    uint32_t total = SPIFFS.totalBytes();
    uint32_t usedB = SPIFFS.usedBytes();
    int32_t freeSpace = total - usedB;
    
    return freeSpace;
}

void files_manager_reset_config() {
    SPIFFS.remove(FILES_CONFIG);
    files_manager_set_default_config_gang();
    SPIFFS.remove(FILES_UUID);
    SPIFFS.remove(FILES_PROGRAM);
}

void files_manager_set_default_config_gang() {
    SYSTEM_CONFIGURATION.serialNo = "default";
    SYSTEM_CONFIGURATION.deviceNo = "default";
    SYSTEM_CONFIGURATION.deviceUser = "default";
    SYSTEM_CONFIGURATION.devicePwd = "default";
    SYSTEM_CONFIGURATION.deviceLockConfig = "default";
    SYSTEM_CONFIGURATION.deviceLockControl = "default";
    SYSTEM_CONFIGURATION.deviceLockTotal = "default";
    SYSTEM_CONFIGURATION.deviceReset = "default";
    SYSTEM_CONFIGURATION.deviceHostName = "default";

    uint32_t val = esp_random();
    val = val % 999;
    SYSTEM_CONFIGURATION.apSSID = "dm_infinite_" + String(val);
    SYSTEM_CONFIGURATION.apPWD = "automation";
    SYSTEM_CONFIGURATION.wifiSSID = "default";
    SYSTEM_CONFIGURATION.wifiPWD = "default";

    SYSTEM_CONFIGURATION.mqttHost = "inet.infiniteautomation.net";
    SYSTEM_CONFIGURATION.mqttPort = "1883";
    SYSTEM_CONFIGURATION.mqttTopicSub = "default/device";
    SYSTEM_CONFIGURATION.mqttTopicPub = "default/mobile";
    SYSTEM_CONFIGURATION.mqttTopicConfig = "default/config";
    SYSTEM_CONFIGURATION.mqttUser = "default";
    SYSTEM_CONFIGURATION.mqttPWD = "default";

    SYSTEM_CONFIGURATION.rtc_url = "https://api.infiniteautomation.net/api/time";
    SYSTEM_CONFIGURATION.switchType = "TOGGLE";
    SYSTEM_CONFIGURATION.device_configured = 0;
    SYSTEM_CONFIGURATION.mqtt_cert_url = "http://firmware.infiniteautomation.net/certs/infy_mqtt_client.crt";
    SYSTEM_CONFIGURATION.wifi_configured = 1;
    SYSTEM_CONFIGURATION.broadcastIntervalWifi = 10;
    SYSTEM_CONFIGURATION.broadcastIntervalMqtt = 10;
    SYSTEM_CONFIGURATION.owner = "infinite";
    SYSTEM_CONFIGURATION.trailing_edge = 0;
    SYSTEM_CONFIGURATION.startup_brightness = 100;
    files_manager_write_config();
}

void files_manager_set_default_config() {
    SYSTEM_CONFIGURATION.serialNo = "default";
    SYSTEM_CONFIGURATION.deviceNo = "default";
    SYSTEM_CONFIGURATION.deviceUser = "default";
    SYSTEM_CONFIGURATION.devicePwd = "default";
    SYSTEM_CONFIGURATION.deviceLockConfig = "default";
    SYSTEM_CONFIGURATION.deviceLockControl = "default";
    SYSTEM_CONFIGURATION.deviceLockTotal = "default";
    SYSTEM_CONFIGURATION.deviceReset = "default";
    SYSTEM_CONFIGURATION.deviceHostName = "default";

    uint32_t val = esp_random();
    val = val % 999;
    SYSTEM_CONFIGURATION.apSSID = "dm_infinite_" + String(val);
    SYSTEM_CONFIGURATION.apPWD = "automation";
    SYSTEM_CONFIGURATION.wifiSSID = "default";
    SYSTEM_CONFIGURATION.wifiPWD = "default";

    SYSTEM_CONFIGURATION.mqttHost = "inet.infiniteautomation.net";
    SYSTEM_CONFIGURATION.mqttPort = "1883";
    SYSTEM_CONFIGURATION.mqttTopicSub = "default/device";
    SYSTEM_CONFIGURATION.mqttTopicPub = "default/mobile";
    SYSTEM_CONFIGURATION.mqttTopicConfig = "default/config";
    SYSTEM_CONFIGURATION.mqttUser = "default";
    SYSTEM_CONFIGURATION.mqttPWD = "default";

    SYSTEM_CONFIGURATION.rtc_url = "https://api.infiniteautomation.net/api/time";
    SYSTEM_CONFIGURATION.switchType = "TOGGLE";
    SYSTEM_CONFIGURATION.device_configured = 0;
    SYSTEM_CONFIGURATION.mqtt_cert_url = "http://firmware.infiniteautomation.net/certs/infy_mqtt_client.crt";
    SYSTEM_CONFIGURATION.wifi_configured = 1;
    SYSTEM_CONFIGURATION.broadcastIntervalWifi = 10;
    SYSTEM_CONFIGURATION.broadcastIntervalMqtt = 10;
    SYSTEM_CONFIGURATION.owner = "infinite";
    SYSTEM_CONFIGURATION.gangType = GANG_DEFAULT_CONFIG;
    SYSTEM_CONFIGURATION.trailing_edge = 0;
    SYSTEM_CONFIGURATION.startup_brightness = 100;

    files_manager_write_config();
}

void files_manager_read_config() {
    if (!SPIFFS.exists(FILES_CONFIG)) {
        files_manager_set_default_config();
        return;
    }

    File file = SPIFFS.open(FILES_CONFIG, FILE_READ);

    SYSTEM_CONFIGURATION.serialNo = file.readStringUntil(',');
    SYSTEM_CONFIGURATION.deviceNo = file.readStringUntil(',');
    SYSTEM_CONFIGURATION.deviceUser = file.readStringUntil(',');
    SYSTEM_CONFIGURATION.devicePwd = file.readStringUntil(',');

    SYSTEM_CONFIGURATION.deviceIP = file.readStringUntil(',');
    SYSTEM_CONFIGURATION.deviceMac = file.readStringUntil(',');

    SYSTEM_CONFIGURATION.deviceLockConfig = file.readStringUntil(',');
    SYSTEM_CONFIGURATION.deviceLockControl = file.readStringUntil(',');
    SYSTEM_CONFIGURATION.deviceLockTotal = file.readStringUntil(',');

    SYSTEM_CONFIGURATION.deviceReset = file.readStringUntil(',');
    SYSTEM_CONFIGURATION.deviceHostName = file.readStringUntil(',');

    SYSTEM_CONFIGURATION.apSSID = file.readStringUntil(',');
    SYSTEM_CONFIGURATION.apPWD = file.readStringUntil(',');

    SYSTEM_CONFIGURATION.wifiSSID = file.readStringUntil(',');
    SYSTEM_CONFIGURATION.wifiPWD = file.readStringUntil(',');

    SYSTEM_CONFIGURATION.mqttHost = file.readStringUntil(',');
    SYSTEM_CONFIGURATION.mqttPort = file.readStringUntil(',');
    SYSTEM_CONFIGURATION.mqttUser = file.readStringUntil(',');
    SYSTEM_CONFIGURATION.mqttPWD = file.readStringUntil(',');
    SYSTEM_CONFIGURATION.mqttTopicPub = file.readStringUntil(',');
    SYSTEM_CONFIGURATION.mqttTopicSub = file.readStringUntil(',');
    SYSTEM_CONFIGURATION.mqttTopicConfig = file.readStringUntil(',');

    file.read((uint8_t*)&SYSTEM_CONFIGURATION.uuid_counter, 2);
    file.read((uint8_t*)&SYSTEM_CONFIGURATION.uuid_max_count, 2);
    file.read((uint8_t*)&SYSTEM_CONFIGURATION.use_token, 1);

    SYSTEM_CONFIGURATION.rtc_url = file.readStringUntil(',');
    SYSTEM_CONFIGURATION.switchType = file.readStringUntil(',');

    if (SYSTEM_CONFIGURATION.rtc_url == "" || SYSTEM_CONFIGURATION.rtc_url == "http://dev.infiniteautomation.com.au:9000/api/time")
        SYSTEM_CONFIGURATION.rtc_url = "https://api.infiniteautomation.net/api/time";

    if (SYSTEM_CONFIGURATION.switchType == "")
        SYSTEM_CONFIGURATION.switchType = "TOGGLE";

    file.read((uint8_t*)&SYSTEM_CONFIGURATION.ota_broadcast, 1);
    file.read((uint8_t*)&SYSTEM_CONFIGURATION.timeZone, 1);
    file.read((uint8_t*)&SYSTEM_CONFIGURATION.restart_broadcast, 1);
    file.read((uint8_t*)&SYSTEM_CONFIGURATION.mqtt_connection_trial, 1);
    file.read((uint8_t*)&SYSTEM_CONFIGURATION.device_configured, 1);

    SYSTEM_CONFIGURATION.mqtt_cert_url = file.readStringUntil(',');
    if (SYSTEM_CONFIGURATION.mqtt_cert_url == "")
        SYSTEM_CONFIGURATION.mqtt_cert_url = "http://firmware.infiniteautomation.net/certs/infy_mqtt_client.crt";

    file.read((uint8_t*)&SYSTEM_CONFIGURATION.wifi_configured, 1);
    file.read((uint8_t*)&SYSTEM_CONFIGURATION.broadcastIntervalWifi, 2);
    file.read((uint8_t*)&SYSTEM_CONFIGURATION.broadcastIntervalMqtt, 2);

    if (SYSTEM_CONFIGURATION.broadcastIntervalMqtt == 0) {
        SYSTEM_CONFIGURATION.broadcastIntervalMqtt = 10;
    }

    if (SYSTEM_CONFIGURATION.broadcastIntervalWifi == 0) {
        SYSTEM_CONFIGURATION.broadcastIntervalWifi = 10;
    }

    SYSTEM_CONFIGURATION.owner = file.readStringUntil(',');
    file.read((uint8_t*)&SYSTEM_CONFIGURATION.config_owner_changed, 1);
    SYSTEM_CONFIGURATION.gangType = file.readStringUntil(',');
    if (SYSTEM_CONFIGURATION.gangType == "") {
        SYSTEM_CONFIGURATION.gangType = GANG_DEFAULT_CONFIG;
    }
    file.read((uint8_t*)&SYSTEM_CONFIGURATION.hard_timer_a, 2);
    file.read((uint8_t*)&SYSTEM_CONFIGURATION.hard_timer_b, 2);
    file.read((uint8_t*)&SYSTEM_CONFIGURATION.hard_timer_c, 2);
    file.read((uint8_t*)&SYSTEM_CONFIGURATION.hard_timer_d, 2);

    file.read((uint8_t*)&SYSTEM_CONFIGURATION.triac_min_pulse_us, 2);
    file.read((uint8_t*)&SYSTEM_CONFIGURATION.triac_pulse_percentage, 1);

    file.read((uint8_t*)&SYSTEM_CONFIGURATION.brightness_ceiling_percentage, 1);
    file.read((uint8_t*)&SYSTEM_CONFIGURATION.brightness_floor_percentage, 1);
    file.read((uint8_t*)&SYSTEM_CONFIGURATION.trailing_edge, 1);
    file.read((uint8_t*)&SYSTEM_CONFIGURATION.startup_brightness, 1);
    
    file.close();
}

void files_manager_write_config() {
    File file = SPIFFS.open(FILES_CONFIG, FILE_WRITE);

    file.print(SYSTEM_CONFIGURATION.serialNo);
    file.print(",");
    file.print(SYSTEM_CONFIGURATION.deviceNo);
    file.print(",");
    file.print(SYSTEM_CONFIGURATION.deviceUser);
    file.print(",");
    file.print(SYSTEM_CONFIGURATION.devicePwd);
    file.print(",");

    file.print(SYSTEM_CONFIGURATION.deviceIP);
    file.print(",");
    file.print(SYSTEM_CONFIGURATION.deviceMac);
    file.print(",");

    file.print(SYSTEM_CONFIGURATION.deviceLockConfig);
    file.print(",");
    file.print(SYSTEM_CONFIGURATION.deviceLockControl);
    file.print(",");
    file.print(SYSTEM_CONFIGURATION.deviceLockTotal);
    file.print(",");

    file.print(SYSTEM_CONFIGURATION.deviceReset);
    file.print(",");
    file.print(SYSTEM_CONFIGURATION.deviceHostName);
    file.print(",");

    file.print(SYSTEM_CONFIGURATION.apSSID);
    file.print(",");
    file.print(SYSTEM_CONFIGURATION.apPWD);
    file.print(",");
    file.print(SYSTEM_CONFIGURATION.wifiSSID);
    file.print(",");
    file.print(SYSTEM_CONFIGURATION.wifiPWD);
    file.print(",");

    file.print(SYSTEM_CONFIGURATION.mqttHost);
    file.print(",");
    file.print(SYSTEM_CONFIGURATION.mqttPort);
    file.print(",");
    file.print(SYSTEM_CONFIGURATION.mqttUser);
    file.print(",");
    file.print(SYSTEM_CONFIGURATION.mqttPWD);
    file.print(",");
    file.print(SYSTEM_CONFIGURATION.mqttTopicPub);
    file.print(",");
    file.print(SYSTEM_CONFIGURATION.mqttTopicSub);
    file.print(",");
    file.print(SYSTEM_CONFIGURATION.mqttTopicConfig);
    file.print(",");

    // file.printf("%d,", SYSTEM_CONFIGURATION.uuid_counter);
    // file.printf("%d,", SYSTEM_CONFIGURATION.uuid_max_count);
    file.write((uint8_t*)&SYSTEM_CONFIGURATION.uuid_counter, 2);
    file.write((uint8_t*)&SYSTEM_CONFIGURATION.uuid_max_count, 2);
    file.write((uint8_t*)&SYSTEM_CONFIGURATION.use_token, 1);

    file.print(SYSTEM_CONFIGURATION.rtc_url);
    file.print(",");
    file.print(SYSTEM_CONFIGURATION.switchType);
    file.print(",");
    file.write((uint8_t*)&SYSTEM_CONFIGURATION.ota_broadcast, 1);
    file.write((uint8_t*)&SYSTEM_CONFIGURATION.timeZone, 1);
    file.write((uint8_t*)&SYSTEM_CONFIGURATION.restart_broadcast, 1);
    file.write((uint8_t*)&SYSTEM_CONFIGURATION.mqtt_connection_trial, 1);
    file.write((uint8_t*)&SYSTEM_CONFIGURATION.device_configured, 1);
    file.print(SYSTEM_CONFIGURATION.mqtt_cert_url);
    file.print(",");
    file.write((uint8_t*)&SYSTEM_CONFIGURATION.wifi_configured, 1);
    file.write((uint8_t*)&SYSTEM_CONFIGURATION.broadcastIntervalWifi, 2);
    file.write((uint8_t*)&SYSTEM_CONFIGURATION.broadcastIntervalMqtt, 2);

    file.print(SYSTEM_CONFIGURATION.owner);
    file.print(",");
    file.write((uint8_t*)&SYSTEM_CONFIGURATION.config_owner_changed, 1);
    file.print(SYSTEM_CONFIGURATION.gangType);
    file.print(",");
    file.write((uint8_t*)&SYSTEM_CONFIGURATION.hard_timer_a, 2);
    file.write((uint8_t*)&SYSTEM_CONFIGURATION.hard_timer_b, 2);
    file.write((uint8_t*)&SYSTEM_CONFIGURATION.hard_timer_c, 2);
    file.write((uint8_t*)&SYSTEM_CONFIGURATION.hard_timer_d, 2);

    file.write((uint8_t*)&SYSTEM_CONFIGURATION.triac_min_pulse_us, 2);
    file.write((uint8_t*)&SYSTEM_CONFIGURATION.triac_pulse_percentage, 1);

    file.write((uint8_t*)&SYSTEM_CONFIGURATION.brightness_ceiling_percentage, 1);
    file.write((uint8_t*)&SYSTEM_CONFIGURATION.brightness_floor_percentage, 1);
    file.write((uint8_t*)&SYSTEM_CONFIGURATION.trailing_edge, 1);
    file.write((uint8_t*)&SYSTEM_CONFIGURATION.startup_brightness, 1);


    file.close();
}

void files_manager_write_current_log(uint32_t epoch) {
    if (files_manager_get_free_space() < 200) {
        // don't write, what to do?
        return;
    }

    int32_t current_ceiling = ceil(current_usage);
    File file = SPIFFS.open(FILES_CURRENT, FILE_APPEND);
    file.write((uint8_t*)&epoch, 4);
    file.write((uint8_t*)&current_ceiling, 4);
    file.write((uint8_t*)relays.get_clicks_ptr(SWITCH_A), 4);
    file.write((uint8_t*)relays.get_clicks_ptr(SWITCH_B), 4);
    file.write((uint8_t*)relays.get_clicks_ptr(SWITCH_C), 4);
    file.write((uint8_t*)relays.get_clicks_ptr(SWITCH_D), 4);

    file.write((uint8_t*)relays.get_on_time_ptr(SWITCH_A), 4);
    file.write((uint8_t*)relays.get_on_time_ptr(SWITCH_B), 4);
    file.write((uint8_t*)relays.get_on_time_ptr(SWITCH_C), 4);
    file.write((uint8_t*)relays.get_on_time_ptr(SWITCH_D), 4);

    file.close();
}

void files_manager_read_current_log(struct Response_package response) {
    File file = SPIFFS.open(FILES_CURRENT, FILE_READ);
    char* arr = (char*)calloc(100, 1);
    sprintf(arr, "%02X", response.cmd_i_val);
    String resp = response.req_id + String(",") + SYSTEM_CONFIGURATION.deviceNo + String(",") + String(arr) + String(",");
    uint32_t epoch, relay_a_clicks, relay_b_clicks, relay_c_clicks, relay_d_clicks, currentusage, relay_a_ontime, relay_b_ontime, relay_c_ontime, relay_d_ontime;
    uint8_t fibyte = 0;
    while (file.available()) {
        if (fibyte) {
            resp += ",";
        } else {
            fibyte = 1;
        }

        file.read((uint8_t*)&epoch, 4);
        file.read((uint8_t*)&currentusage, 4);
        file.read((uint8_t*)&relay_a_clicks, 4);
        file.read((uint8_t*)&relay_b_clicks, 4);
        file.read((uint8_t*)&relay_c_clicks, 4);
        file.read((uint8_t*)&relay_d_clicks, 4);

        file.read((uint8_t*)&relay_a_ontime, 4);
        file.read((uint8_t*)&relay_b_ontime, 4);
        file.read((uint8_t*)&relay_c_ontime, 4);
        file.read((uint8_t*)&relay_d_ontime, 4);

        sprintf(arr, "%d:%d:%d:%d:%d:%d:%d:%d:%d:%d", epoch, currentusage, relay_a_clicks, relay_b_clicks,relay_c_clicks,relay_d_clicks, relay_a_ontime, relay_b_ontime, relay_c_ontime, relay_d_ontime);
        resp += String(arr);
    }

    // resp += file.readString();
    free(arr);
	log_d("Current Response String %s", resp.c_str());

    if (response.resp_type == SCR_MQTT) {
        mqtt_publish_msg(resp);
    } else if (response.resp_type == SCR_HTTP) {
        webserver_send_jsresponse(resp);
    }

    file.close();
}

void files_manager_clear_current_log() {
    File file = SPIFFS.open(FILES_CURRENT, FILE_WRITE);
    file.print("");
    file.close();
}

void files_manager_write_clear_log_file(struct Response_package response) {
    String mqtt_to_write = "[";
    File file = SPIFFS.open(FILES_LOGS, FILE_READ);
    int16_t counter = 0;
    int8_t comma_flag = 0;

    while (file.available()) {
        if (comma_flag)
            mqtt_to_write += ",";

        mqtt_to_write += "{";
        mqtt_to_write += file.readStringUntil('\n');
        mqtt_to_write += "}";
        counter++;
        comma_flag = 1;

        if (counter == 50) {
            server_comms.send_response(mqtt_to_write, response);
            // server_comm_send_response(mqtt_to_write, response);
            mqtt_to_write = "";
            counter = 0;
        }
    }

    mqtt_to_write += "]";
    server_comms.send_response(mqtt_to_write, response);
    file.close();

    file = SPIFFS.open(FILES_LOGS, FILE_WRITE);
    file.println();
    file.close();
}

uint8_t files_manager_delete_log_file() {
    uint8_t ret = SPIFFS.remove(FILES_LOGS);
    return ret;
}

void files_manager_read_uuid_tokens() {
    if (!SPIFFS.exists(FILES_UUID)) {
        return;
    }

    if (SYSTEM_CONFIGURATION.uuid_counter == 0) {
        return;
    }

    File file = SPIFFS.open(FILES_UUID, FILE_READ);

    for (int i = 0; i < SYSTEM_CONFIGURATION.uuid_counter; i++) {
        UUID_STORAGE[i].uuid = file.readStringUntil(',');
        file.read((uint8_t*)&UUID_STORAGE[i].token, 4);
        file.read((uint8_t*)&UUID_STORAGE[i].token_start_time, 4);
    }

    file.close();
}

void files_manager_write_uuid_tokens() {
    File file = SPIFFS.open(FILES_UUID, FILE_WRITE);

    for (int i = 0; i < SYSTEM_CONFIGURATION.uuid_counter; i++) {
        file.print(UUID_STORAGE[i].uuid);
        file.print(",");
        file.write((uint8_t*)&UUID_STORAGE[i].token, 4);
        file.write((uint8_t*)&UUID_STORAGE[i].token_start_time, 4);
    }

    file.close();
}

void files_manager_read_program_status() {
    File file = SPIFFS.open(FILES_PROGRAM, FILE_READ);
    
    file.read((uint8_t*)&timers, TIMERS_MAX_COUNT * sizeof(struct relay_timer));
    file.read((uint8_t*)&schedules, TIMERS_MAX_COUNT * sizeof(struct relay_timer));
    file.read(&timers_count, sizeof(uint8_t));
    file.read(&schedules_count, sizeof(uint8_t));
    file.read((uint8_t*)relays.get_relay(SWITCH_A), sizeof(struct relay));
    file.read((uint8_t*)relays.get_relay(SWITCH_B), sizeof(struct relay));
	file.read((uint8_t*)relays.get_relay(SWITCH_C), sizeof(struct relay));
    file.read((uint8_t*)relays.get_relay(SWITCH_D), sizeof(struct relay));

    file.read((uint8_t*)&current_usage, sizeof(double));
    file.read((uint8_t*)&rtc_today, sizeof(uint32_t));
    file.read((uint8_t*)relays.get_relay_hard_schedule(SWITCH_A), sizeof(struct relay_timer));
    file.read((uint8_t*)relays.get_relay_hard_schedule(SWITCH_B), sizeof(struct relay_timer));
	file.read((uint8_t*)relays.get_relay_hard_schedule(SWITCH_C), sizeof(struct relay_timer));
	file.read((uint8_t*)relays.get_relay_hard_schedule(SWITCH_D), sizeof(struct relay_timer));
    file.close();
}

void files_manager_write_program_status() {
    /**
     * NOTE: Potentially a struct should be used instead which stores the current state and checks that against any changes.
     * 
     */
    File file = SPIFFS.open(FILES_PROGRAM, FILE_WRITE);

    file.write((uint8_t*)&timers, TIMERS_MAX_COUNT * sizeof(struct relay_timer));
    file.write((uint8_t*)&schedules, TIMERS_MAX_COUNT * sizeof(struct relay_timer));
    file.write(&timers_count, sizeof(uint8_t));
    file.write(&schedules_count, sizeof(uint8_t));
    file.write((uint8_t*)relays.get_relay(SWITCH_A), sizeof(struct relay));
    file.write((uint8_t*)relays.get_relay(SWITCH_B), sizeof(struct relay));
	file.write((uint8_t*)relays.get_relay(SWITCH_C), sizeof(struct relay));
    file.write((uint8_t*)relays.get_relay(SWITCH_D), sizeof(struct relay));

    file.write((uint8_t*)&current_usage, sizeof(double));
    file.write((uint8_t*)&rtc_today, sizeof(uint32_t));
    file.write((uint8_t*)relays.get_relay_hard_schedule(SWITCH_A), sizeof(struct relay_timer));
    file.write((uint8_t*)relays.get_relay_hard_schedule(SWITCH_B), sizeof(struct relay_timer));
	file.write((uint8_t*)relays.get_relay_hard_schedule(SWITCH_C), sizeof(struct relay_timer));
	file.write((uint8_t*)relays.get_relay_hard_schedule(SWITCH_D), sizeof(struct relay_timer));
    file.close();
}

void files_manager_write_rtc() {
    File file = SPIFFS.open(FILES_RTC, FILE_WRITE);
    uint32_t epoch = rtc_get_epoch();
    file.write((uint8_t*)&epoch, sizeof(uint32_t));
    file.close();
}

void files_manager_read_rtc() {
    if (!SPIFFS.exists(FILES_RTC)) {
        return;
    }

    File file = SPIFFS.open(FILES_RTC, FILE_READ);
    uint32_t epoch = 0;
    file.read((uint8_t*)&epoch, sizeof(uint32_t));
    file.close();
    SPIFFS.remove(FILES_RTC);

    rtc_init(epoch);
}