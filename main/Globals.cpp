#include "Globals.h"

String ota_date_time = "03 - 05 - 2021, 11:35AM IST";

double temperature_c = 0;

double current_usage = 0;
double current_draw = 0;
uint32_t activepower = 0;

uint32_t hlw8012_update_time = 0;

HLW8012 hlw8012;

char* wifi_ssid;
char* wifi_pass;
char* wifi_host_name;
WiFiClient wifi_client;
WiFiClientSecure wifi_secure;

uint8_t wifi_internet_up = 0;

uint8_t wifi_connected_flag = 0;
uint32_t wifi_sta_start_time = 0;
uint8_t wifi_started_hotspot = 0;
wifi_sta_list_t wifi_ap_station_list;

uint8_t wifi_variables_flag = 0;

uint8_t wifi_program_status = WIFI_DEFAULT;
uint32_t wifi_ap_start_time = 0;
uint8_t wifi_ap_connected = 0;

WiFiUDP broadcast_udp;
IPAddress broadcast_ip(255, 255, 255, 255);
IPAddress ba;

uint32_t MQTT_RECONNECT_TIMER = MQTT_RECONNECT_TIMER_DEF;
//const char* mqtt_broker_user = "yhiqwmwn";
//const char* mqtt_broker_pass = "Xu674AEGyWZy";
char* mqtt_broker_user;
char* mqtt_broker_pass;

char* mqtt_broker_id;

//const char* mqtt_server_domain = "m15.cloudmqtt.com";
//uint16_t mqtt_port = 10146;
char* mqtt_server_domain;
uint16_t mqtt_port = 1883;

//const char* mqtt_outtopic = "iot_updates/";
char* mqtt_publish_topic;
char* mqtt_config_topic;
//const char* mqtt_intopic = "iot_commands/";
char* mqtt_subscribe_topic;

uint8_t mqtt_connected = 0;
volatile uint8_t mqtt_server_set = 0;
PubSubClient mqtt_client(wifi_client);

uint32_t mqtt_timer = 0;


TimerHandle_t mqttReconnectTimer;
uint8_t mqtt_client_connected = 0;
int32_t mqtt_time = 0;

uint32_t mqtt_last_time = 0;

uint8_t mqtt_cert_set = 0;
String mqtt_cert = "";

const char* mqtt_ca_cert = \
"-----BEGIN CERTIFICATE-----\r\n" \
"MIIDwTCCAqmgAwIBAgIUDjGXqIDthxhvgDdgMwechKq2NicwDQYJKoZIhvcNAQEL\r\n" \
"BQAwcDELMAkGA1UEBhMCRUcxDjAMBgNVBAgMBUNhaXJvMQ4wDAYDVQQHDAVDYWly\r\n" \
"bzEbMBkGA1UECgwSU2hlcmlmIEZSRUVMQU5DSU5HMQ8wDQYDVQQLDAZTaGVyaWYx\r\n" \
"EzARBgNVBAMMCm1xdHQubG9jYWwwHhcNMjAwNTI4MTEwMDU1WhcNMjUwNTI4MTEw\r\n" \
"MDU1WjBwMQswCQYDVQQGEwJFRzEOMAwGA1UECAwFQ2Fpcm8xDjAMBgNVBAcMBUNh\r\n" \
"aXJvMRswGQYDVQQKDBJTaGVyaWYgRlJFRUxBTkNJTkcxDzANBgNVBAsMBlNoZXJp\r\n" \
"ZjETMBEGA1UEAwwKbXF0dC5sb2NhbDCCASIwDQYJKoZIhvcNAQEBBQADggEPADCC\r\n" \
"AQoCggEBALni1VMth6Z0WR8TEC4NjAoE9FiyZXNVc0CY23bKehRGUnC+Pf1A9Vdr\r\n" \
"AxWxMzSeSAZ4yRrhr+lSC/bGou0G7GGxYAIYo0xC/lTuvwcYaJjoJqW5oqPbb3mp\r\n" \
"WRbyZexenTT9d/aPaF2yL/RvEZ53s/5stC7mM1hiQOshLAK/MRvLGVhdRv8dwu3c\r\n" \
"DFSdPja3mnneSwkBafpbx6bfaHWCaYNOncb1kBZfK9aHlDuCnGq36vFGy6wSQo65\r\n" \
"pf9Q6lf5q6Y51Jt+PVZybbJaPf4W3Um7iJbej/Cdgn9WE6mZOGyXJaFf3Jzn3pmo\r\n" \
"TlQX7mhal6cPLJR7c9VMUPhGySayr2UCAwEAAaNTMFEwHQYDVR0OBBYEFPeix2gB\r\n" \
"0MZ5oPpAGY+pwnzPsepuMB8GA1UdIwQYMBaAFPeix2gB0MZ5oPpAGY+pwnzPsepu\r\n" \
"MA8GA1UdEwEB/wQFMAMBAf8wDQYJKoZIhvcNAQELBQADggEBAKZSEaDPNnYBa9SM\r\n" \
"DqJlH4O9Ly2bsphjdC+W8GHM71WAlrIzhm/GZ9IPPpvPDigi2Mh4ZHk9niseKWJg\r\n" \
"9TKJQa1JYQRWN8zJaYKp5ezOqxxxpL/L327eiU28pmDlcfQ2YzWKD3Tj3n1gNFaX\r\n" \
"CoJH3VTOXXY99VkYCEpMUM6y0cEQCboevpZQqP6qr39N9FGGDynwyd0xajT3k9+7\r\n" \
"K+29MZ0VQgary77NHgduwSPiho+7WKiSLIFTt2R9DXpUyB2oyKsjxxUaFv3rB6hx\r\n" \
"kMVMYzNH3pvQfGLzKz0hS4dZr+XK5N3a0I2ZoEps4wJssVjQh/VUZszn1WVvX2sS\r\n" \
"8un1JqE=\r\n" \
"-----END CERTIFICATE-----\r\n";
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// ------------------------------ OTA Globals ----------------------------------
// -----------------------------------------------------------------------------
uint8_t ota_started = 0;

system_configuration SYSTEM_CONFIGURATION;
relay_timer timers[TIMERS_MAX_COUNT], schedules[TIMERS_MAX_COUNT];

uint8_t timers_count = 0;
uint8_t schedules_count = 0;


uint8_t buzzer_init_flag = 0;
uint32_t buzzer_timer_start = 0;
uint16_t buzzer_max_count = 0, buzzer_count = 0;
uint8_t buzzer_started = 0;
uint16_t buzzer_duration = 0;
uint16_t buzzer_frequncy = BUZZER_FREQUENCY;
uint8_t buzzer_reboot = 0;



struct uuid_tokens *UUID_STORAGE;

int32_t one_minute = 60;
int32_t one_hour = 60 * one_minute;
int32_t one_day = 24 * one_hour;
int32_t one_year = 365 * one_day;
int32_t four_years = (4 * one_year) + one_day;

uint8_t rtc_init_flag = 0;
uint32_t _rtc_epoch_time = 0;
int32_t rtc_time_diff = 0;
uint32_t rtc_today = 0;
uint32_t rtc_hour = 0;

uint32_t rtc_start_time = 0;

uint32_t rtc_millis_last_time = 0;
uint32_t rtc_millis_value = 0;

uint8_t rtc_url_try_flag = 1;
uint32_t rtc_url_connect_try_time = 0;
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// ----------------------------- Watchdog Globals ------------------------------
// -----------------------------------------------------------------------------
const int64_t WATCHDOG_TIMEOUT  = 20 * 1000 * 1000; // 45 seconds in us
hw_timer_t *watchdog_timer = NULL;
// ----------------

uint32_t largest_delay_start = 0;
uint32_t largest_delay_end = 0;
uint32_t largest_zero_cross_delay = 0;

uint8_t program_status_change_flag = 0; // Flag which indicates that the program state has changed and the file should be updated.
