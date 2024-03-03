#pragma once
#ifndef SERVER_COMMS_H_
#define SERVER_COMMS_H_
#include <Arduino.h>
#include "Globals.h"
#include "FilesManager.h"
#include "UserHandler.h"
#include "MqttHandler.h"
#include "Buzzer.h"
#include "SystemConfiguration.h"
#include "WifiManager.h"
#include "Ota.h"
#include "RelayController.h"
#include "Logging.h"

class ServerComms{
    private:
        Relays *_relays;
        void set_timer(uint8_t cmd_i_val, struct relay_timer* temp_timer, struct Response_package response);
        void update_timer(uint8_t cmd_i_val, struct relay_timer* temp_timer, struct Response_package response);
        void delete_timer(uint8_t cmd_i_val, struct relay_timer* temp_timer, struct Response_package response);
        void read_timer(uint8_t cmd_i_val, struct relay_timer* temp_timer, struct Response_package response);
        void process_timer(uint8_t cmd_i_val, struct relay_timer* temp_timer, struct Response_package response);

        void set_schedule(uint8_t cmd_i_val, struct relay_timer* temp_timer, struct Response_package response);
        void update_schedule(uint8_t cmd_i_val, struct relay_timer* temp_timer, struct Response_package response);
        void delete_schedule(uint8_t cmd_i_val, struct relay_timer* temp_timer, struct Response_package response);
        void read_schedule(uint8_t cmd_i_val, struct relay_timer* temp_timer, struct Response_package response);
        void process_schedule(uint8_t cmd_i_val, struct relay_timer* temp_timer, struct Response_package response);
       void process_switch(struct Command cmd, uint8_t brightness, struct Response_package response);
        void process_switch(struct Command cmd, struct Response_package response);
        void process_config(String params[], int16_t params_count, struct Command cmd, struct Response_package response);
        void process_config_c(String params[], int16_t params_count, struct Command cmd, struct Response_package response);
        void process_config_d(String params[], int16_t params_count, struct Command cmd, struct Response_package response);
        void process_config_e(String params[], int16_t params_count, struct Command cmd, struct Response_package response);
        void process_bulk_commands(String params[], int16_t params_count, struct Command cmd, struct Response_package response);
        void owner_changed(String params[], int16_t params_count);
        uint8_t config_wifi(String params[], int16_t params_count);

        void process_file_config(String params[], int16_t params_count, struct Command cmd, struct Response_package response);
        void copy_command_int(struct Command* c, uint8_t i_val);
        
        uint8_t parse_one_byte(uint8_t* c, int16_t* i);
        uint16_t parse_two_bytes(uint8_t* c, int16_t* i);
        uint32_t parse_four_bytes(uint8_t* c, int16_t* i);

        String read_till_comma(uint8_t* c, int16_t* i, uint16_t count);
        String read_till_colon(uint8_t* c, int16_t* i, uint16_t count);
        String read_till_end(uint8_t* c, int16_t* i, uint16_t count);

        uint32_t get_till_comma(uint8_t* c, int16_t* i, int8_t* num_bytes);
        uint8_t get_number(uint8_t n);

        int8_t parse_weekdays(String weekdays);
        void parse_json(String parameters[], uint8_t* c, int16_t count, int16_t* params_count, struct Response_package response);
        void send_timer_status(struct Response_package response, uint8_t cmd_type);
        void send_true(struct Response_package response);
        void send_false(struct Response_package response);
        void send_invalid(struct Response_package response);
    public:
        void send_response(char* str, uint16_t data_len, struct Response_package response);
        void send_response(String str, struct Response_package response);
        void send_response(int32_t* val, struct Response_package response);

        void send_switch_status(struct Response_package response); 
        void send_custom(String response_msg, struct Response_package response_pack);
        String weekdays_string(int8_t weekdays);
        void factory(uint8_t* msg, int16_t count, int8_t resp_type);
        void factory_config(uint8_t* msg, int16_t count, int8_t resp_type);
        ServerComms(){};
        void init(Relays* relays);
        void loop();
};

#endif