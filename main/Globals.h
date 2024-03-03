/*
 * Globals.h
 *
 * Created: 12/2/2018 12:33:44 PM
 *  Author: Sherif
 */ 
#pragma once
#ifndef GLOBALS_H_
#define GLOBALS_H_
#include "HLW8012.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include "PubSubClient.h"
#include "DateFormat.h"
#include "Logging.h"

#define NUM_CHANNELS 4
#define MAX_BRIGHTNESS 100ULL

#define SERIAL_PRINT // <-------------------------- COMMENT
#define GANG_DEFAULT_CONFIG			"4_DM"

// MAKE SURE TO COMMENT SERIAL PRINT 07
#define FIRMWARE_VERSION_NUMBER			(String(BUILD_DATE_YEAR_INT) + "." + String(BUILD_DATE_MONTH_INT) + "." + String(BUILD_DATE_DAY_INT))
#define FIRMWARE_VERSION_DESCRIPTION	String("Usage issue fix and Prefix parameter added to device configuration process")
extern String ota_date_time;

#define DEVICENO
// -----------------------------------------------------------------------------
// -------------------------- Current sensor Globals ---------------------------
// -----------------------------------------------------------------------------
extern double temperature_c;

extern double current_usage;
extern double current_draw;
extern uint32_t activepower;

extern uint32_t hlw8012_update_time;

extern HLW8012 hlw8012;
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --------------------------- Switches Globals --------------------------------
// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// ----------------------------- WIFI Globals ----------------------------------
// -----------------------------------------------------------------------------
//const char* ssid = "infinite";
//const char* pass = "automation";

//const char* ssid = "Gujju";
//const char* pass = "122FerntreeGullyRd";
//const char* pass = "2BGrantStOakleigh";
#define WIFI_AP_SWAP_TIMEOUT	15000

extern char* wifi_ssid;
extern char* wifi_pass;
extern char* wifi_host_name;
extern WiFiClient wifi_client;
extern WiFiClientSecure wifi_secure;

extern uint8_t wifi_internet_up;

extern uint8_t wifi_connected_flag;
extern uint32_t wifi_sta_start_time;
extern uint8_t wifi_started_hotspot;
extern wifi_sta_list_t wifi_ap_station_list;

extern uint8_t wifi_variables_flag;

enum wifi_status{WIFI_DEFAULT, WIFI_CONNECTING_STA, WIFI_STATUS_AP, WIFI_CONNECTED};
extern uint8_t wifi_program_status;
extern uint32_t wifi_ap_start_time;
extern uint8_t wifi_ap_connected;

extern WiFiUDP broadcast_udp;
extern IPAddress broadcast_ip;
extern IPAddress ba;
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// ----------------------------- MQTT Globals ----------------------------------
// -----------------------------------------------------------------------------
#define MQTT_CHANNEL_CONFIG		10
#define MQTT_CHANNEL_SUB		20

#define MQTT_TOPIC_MAX_SIZE 100
#define MQTT_RECONNECT_TIMER_DEF 60000
extern uint32_t MQTT_RECONNECT_TIMER;
//const char* mqtt_broker_user = "yhiqwmwn";
//const char* mqtt_broker_pass = "Xu674AEGyWZy";
extern char* mqtt_broker_user;
extern char* mqtt_broker_pass;

extern char* mqtt_broker_id;

//const char* mqtt_server_domain = "m15.cloudmqtt.com";
//uint16_t mqtt_port = 10146;
extern char* mqtt_server_domain;
extern uint16_t mqtt_port;

//const char* mqtt_outtopic = "iot_updates/";
extern char* mqtt_publish_topic;
extern char* mqtt_config_topic;
//const char* mqtt_intopic = "iot_commands/";
extern char* mqtt_subscribe_topic;

extern uint8_t mqtt_connected;
extern volatile uint8_t mqtt_server_set;
extern PubSubClient mqtt_client;

extern uint32_t mqtt_timer;
#define MQTT_SEND_TIMER 2000 // 2 seconds

extern TimerHandle_t mqttReconnectTimer;
extern uint8_t mqtt_client_connected;
extern int32_t mqtt_time;

extern uint32_t mqtt_last_time;

extern uint8_t mqtt_cert_set;
extern String mqtt_cert;

extern const char* mqtt_ca_cert;
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// ------------------------------ OTA Globals ----------------------------------
// -----------------------------------------------------------------------------
extern uint8_t ota_started;
#define OTA_BUFFSIZE 1024
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// ---------------------- Server Communication Globals -------------------------
// -----------------------------------------------------------------------------
#define MIN_BYTES_LENGTH_GENERAL			2
#define MIN_BYTES_LENGTH_TIMER_DELETE		MIN_BYTES_LENGTH_GENERAL + 3 // comma and id
#define MIN_BYTES_LENGTH_TIMER				MIN_BYTES_LENGTH_GENERAL + 9 // comma and epoch

enum server_comm_reponse{SCR_MQTT, SCR_HTTP};
enum server_comm_commands{CMD_SWITCH, CMD_TIMER, CMD_SCHEDULER, CMD_CONFIG};

enum server_comm_switches{SWITCH_A, SWITCH_B, SWITCH_C, SWITCH_D};

enum server_comm_lockstatus{LOCK_STATUS, LOCK_UNLOCK, LOCK_LOCK, LOCK_NO_OP};
enum server_comm_status{STATUS_STATUS, STATUS_OFF, STATUS_ON, STATUS_TOGGLE};

struct Command
{
	uint8_t cmd : 2;
	uint8_t Switch : 2; // has to be called that because of switch state
	uint8_t lock_status : 2;
	uint8_t status : 2; // on/off
	uint8_t cmd_i_value;
};

struct Response_package
{
	String req_id = "";
	int8_t resp_type;
	uint8_t cmd_i_val;
};
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// ---------------------------- Relays Globals ---------------------------------
// -----------------------------------------------------------------------------
#define RELAY_SWITCH_A_CHANNEL	10
#define RELAY_SWITCH_B_CHANNEL	11

#define TIMERS_MAX_COUNT	16
struct relay
{
	uint8_t status = 0;
	uint8_t lock = 0;
	double current_usage = 0;
	int32_t clicks_usage = 0;
	int8_t pin;
	char name;
	uint32_t on_time = 0;
	uint32_t start_time = 0;
	uint8_t brightness = 100;
	uint8_t current_brightness = 100;
	uint64_t timeout_us = 0;
};

struct relay_timer
{
	int8_t timer_id = 0;
	uint32_t epoch_time = 0;
	int16_t repeat_count = 0;
	uint32_t duration_repeat = 0;
	uint8_t relay_switch = SWITCH_A;
	uint8_t timer_status = 0;
	uint32_t timer_duration = 0;
	uint32_t timer_start_ms = 0;
	uint8_t timer_started = 0;
	uint8_t timer_state = 0;
	int8_t weekdays = 0;
};
extern relay_timer timers[TIMERS_MAX_COUNT], schedules[TIMERS_MAX_COUNT];


extern uint8_t timers_count;
extern uint8_t schedules_count;
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// ------------------------------ Buzzer Globals -------------------------------
// -----------------------------------------------------------------------------
#define BUZZER_MAX_COUNT			10
#define BUZZER_MAX_DURATION			5000
#define BUZZER_PIN					19
#define BUZZER_FREQUENCY			2000
#define BUZZER_CHANNEL				0
#define BUZZER_RESOLUTION			8
#define BUZZER_DUTY_CYCLE			125
#define BUZZER_DEFAULT_DURATION		500

extern uint8_t buzzer_init_flag;
extern uint32_t buzzer_timer_start;
extern uint16_t buzzer_max_count, buzzer_count;
extern uint8_t buzzer_started;
extern uint16_t buzzer_duration;
extern uint16_t buzzer_frequncy;
extern uint8_t buzzer_reboot;
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// ----------------------- System Configuration Globals ------------------------
// -----------------------------------------------------------------------------
#define FOLDER_LOGS		String("/logs/")

#define FILES_CONFIG	F("/config.byn")
#define FILES_LOGS		F("/logs/logs.json")
#define FILES_CURRENT	F("/current.bin")
#define FILES_UUID		F("/uuid.bin")
#define FILES_PROGRAM	F("/program.bin")
#define FILES_RTC		F("/rtc.bin")

#define SYS_CONFIG_USE_TOKEN				String(("useToken"))
#define SYS_CONFIG_SERIALNO					String(("serialNo"))
#define SYS_CONFIG_DEVICENO					String(("deviceNo"))
#define SYS_CONFIG_DEVICE_USER				String(("deviceUser"))
#define SYS_CONFIG_DEVICE_PWD				String(("devicePwd"))
#define SYS_CONFIG_OWNER					String("owner")

#define SYS_CONFIG_DEVICE_IP				String(("deviceIP"))
#define SYS_CONFIG_DEVICE_MAC				String(("deviceMac"))

#define SYS_CONFIG_DEVICELOCK_CONFIG		String(("deviceLockConfig"))
#define SYS_CONFIG_DEVICELOCK_CONTROL		String(("deviceLockControl"))
#define SYS_CONFIG_DEVICELOCK_TOTAL			String(("deviceLockTotal"))

#define SYS_CONFIG_DEVICE_RESET				String(("deviceReset"))
#define SYS_CONFIG_DEVICE_HOSTNAME			String(("deviceHostName"))

#define SYS_CONFIG_AP_SSID					String(("apSSID"))
#define SYS_CONFIG_AP_PWD					String(("apPWD"))
#define SYS_CONFIG_WIFI_SSID				String(("wifiSSID"))
#define SYS_CONFIG_WIFI_PWD					String(("wifiPWD"))

#define SYS_CONFIG_MQTT_HOST				String(("mqttHost"))
#define SYS_CONFIG_MQTT_PORT				String(("mqttPort"))
#define SYS_CONFIG_MQTT_USER				String(("mqttUser"))
#define SYS_CONFIG_MQTT_PASSWORD			String(("mqttPwd"))
#define SYS_CONFIG_MQTT_TOPIC_PUB			String(("mqttTopicPub"))
#define SYS_CONFIG_MQTT_TOPIC_SUB			String(("mqttTopicSub"))
#define SYS_CONFIG_MQTT_TOPIC_CONFIG		String(("mqttTopicConfig"))
#define SYS_CONFIG_MQTT_TOPIC_SERVER		String(("mqttTopicServer"))
#define SYS_CONFIG_MQTT_cert_URL			String(("mqttCertUrl"))

#define SYS_CONFIG_RTC_URL					String(("rtcUrl"))
#define SYS_CONFIG_RTC						String(("rtc"))
#define SYS_CONFIG_RTC_TIMEZONE				String (("timeZone"))

#define SYS_CONFIG_SWITCH_TYPE				String(("switchType"))
#define SYS_CONFIG_BROADCAST_MQTT_INTERVAL	String(("broadcastMQTTInterval"))
#define SYS_CONFIG_BROADCAST_WIFI_INTERVAL	String(("broadcastWiFiInterval"))
#define SYS_CONFIG_GANG_TYPE				String(("gangType"))

#define SYS_CONFIG_HARD_TIMER_A				String(("hardTimerA"))
#define SYS_CONFIG_HARD_TIMER_B				String(("hardTimerB"))
#define SYS_CONFIG_HARD_TIMER_C				String(("hardTimerC"))
#define SYS_CONFIG_HARD_TIMER_D				String(("hardTimerD"))

#define SYS_CONFIG_MIN_TRIAC_PULSE_US				String(("triacMinPulseMicroSeconds"))
#define SYS_CONFIG_TRIAC_PULSE_PERCENTAGE				String(("triacPulsePercentage"))
#define SYS_CONFIG_TRAILING_EDGE				String(("trailingEdge"))
#define SYS_CONFIG_STARTUP_BRIGHTNESS				String(("startupBrightness"))

#define SYS_CONFIG_BRIGHTNESS_CEILING_PERCENTAGE		String(("brightnessCeilingPercentage"))
#define SYS_CONFIG_BRIGHTNESS_FLOOR_PERCENTAGE		String(("brightnessFloorPercentage"))

struct system_configuration
{
	String serialNo;
	String deviceNo;
	String deviceUser;
	String devicePwd;
	
	String deviceIP;
	String deviceMac;
	
	String deviceLockConfig;
	String deviceLockControl;
	String deviceLockTotal;
	
	String deviceReset;
	String deviceHostName;
	
	String apSSID;
	String apPWD;
	String wifiSSID;
	String wifiPWD;
	
	String mqttHost;
	String mqttPort;
	String mqttUser;
	String mqttPWD;
	String mqttTopicPub;
	String mqttTopicSub;
	String mqttTopicConfig;
	String mqtt_cert_url;
	
	uint16_t uuid_counter = 0;
	uint16_t uuid_max_count = 10;
	
	uint8_t use_token = 0;
	
	String rtc_url;
	
	String switchType;
	
	uint8_t ota_broadcast = 0;
	int8_t timeZone = 0;
	uint8_t restart_broadcast = 0;
	uint8_t mqtt_connection_trial = 0;
	uint8_t device_configured = 0;
	uint8_t wifi_configured = 0;
	uint16_t broadcastIntervalMqtt = 0;
	uint16_t broadcastIntervalWifi = 0;
	String owner;
	uint8_t config_server_request;
	uint8_t config_owner_changed = 0;
	String gangType;
	uint16_t hard_timer_a = 0;
	uint16_t hard_timer_b = 0;
	uint16_t hard_timer_c = 0;
	uint16_t hard_timer_d = 0;

	uint16_t triac_min_pulse_us = 100; // 
	uint8_t triac_pulse_percentage = 95; // Percentage of half cycle triac is enabled

	uint8_t brightness_ceiling_percentage = 95; // Brightness ceiling at which point triac will remain ON
	uint8_t brightness_floor_percentage = 5; // Brightness ceiling at which point triac will remain OFF

	uint8_t trailing_edge = 0;
	uint8_t startup_brightness = 100;
};
extern system_configuration SYSTEM_CONFIGURATION;
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --------------------------- User Manager Globals ----------------------------
// -----------------------------------------------------------------------------

struct uuid_tokens
{
	String uuid;
	uint32_t token = 0;
	uint32_t token_start_time = 0;
};
extern struct uuid_tokens *UUID_STORAGE;
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// ------------------------------- RTC Globals ---------------------------------
// -----------------------------------------------------------------------------
extern int32_t one_minute;
extern int32_t one_hour;
extern int32_t one_day;
extern int32_t one_year;
extern int32_t four_years;

enum rtc_days{SUNDAY, MONDAY, TUESDAY, WEDNESDAY, THURSDAY, FRIDAY, SATURDAY};
extern uint8_t rtc_init_flag;
extern uint32_t _rtc_epoch_time;
extern int32_t rtc_time_diff;
extern uint32_t rtc_today;
extern uint32_t rtc_hour;
struct DateTime
{
	uint8_t month;
	uint8_t day;
	uint16_t year;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
};

extern uint32_t rtc_start_time;

extern uint32_t rtc_millis_last_time;
extern uint32_t rtc_millis_value;

extern uint8_t rtc_url_try_flag;
extern uint32_t rtc_url_connect_try_time;
// -----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// ----------------------------- Watchdog Globals ------------------------------
// -----------------------------------------------------------------------------
extern const int64_t WATCHDOG_TIMEOUT; // 45 seconds in us
extern hw_timer_t *watchdog_timer;
// -----------------------------------------------------------------------------

#define BYTE_TO_BINARY(byte)  \
(byte & 0x80 ? '1' : '0'), \
(byte & 0x40 ? '1' : '0'), \
(byte & 0x20 ? '1' : '0'), \
(byte & 0x10 ? '1' : '0'), \
(byte & 0x08 ? '1' : '0'), \
(byte & 0x04 ? '1' : '0'), \
(byte & 0x02 ? '1' : '0'), \
(byte & 0x01 ? '1' : '0')


// template <typename T>
extern uint32_t largest_delay_start;
extern uint32_t largest_delay_end;
extern uint32_t largest_zero_cross_delay;

extern uint8_t program_status_change_flag;
#endif /* GLOBALS_H_ */


