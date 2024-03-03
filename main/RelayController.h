#pragma once
#ifndef RELAYS_H_
#define RELAYS_H_
#include <Arduino.h>
#include "Globals.h"
#include <functional>
#include "Logging.h"

#define RELAY_SWITCH_A      23 //Channel 2 - Switch 3
#define RELAY_SWITCH_B      25 //
#define RELAY_SWITCH_C		26
#define RELAY_SWITCH_D		27

/**
 * Switch 1 - Pin 34 - Channel 1 Pin 27 - Out Label 2
 * Switch 2 - Pin 35 - Channel 2 Pin 23 - Out Label 4
 * Switch 3 - Pin 32 - Channel 3 Pin 25 - Out Label 3
 * Switch 4 - Pin 33 - Channel 4 Pin 26 - Out Label 1

 * 
 */
class Relays{
    private:
        char _relay_names[NUM_CHANNELS] = {'a', 'b', 'c', 'd'};
        int _relay_pins[NUM_CHANNELS] = {RELAY_SWITCH_A, RELAY_SWITCH_B, RELAY_SWITCH_C, RELAY_SWITCH_D};
        struct relay _relay_a, _relay_b, _relay_c, _relay_d;
        struct relay* _relays_arr[NUM_CHANNELS] = {&_relay_a, &_relay_b, &_relay_c, &_relay_d};

        struct relay_timer _hard_schedule_a, _hard_schedule_b, _hard_schedule_c, _hard_schedule_d;
        struct relay_timer* _relay_schedules_arr[NUM_CHANNELS] = {&_hard_schedule_a, &_hard_schedule_b, &_hard_schedule_c, &_hard_schedule_d};

        void process_ontime(struct relay *r);
        uint8_t set_brightness(struct relay *r, uint8_t brightness);
        void set_off(struct relay *r, uint8_t r_mode);
        void set_on(struct relay *r, uint8_t r_mode);
        void set_toggle(struct relay *r, uint8_t r_mode);
        void state_change_updater(struct relay *r, uint8_t r_mode);
        void hard_timer_handler(struct relay *r, uint8_t r_mode);
        void set_schedules(struct relay_timer *rt, uint16_t hard_timer, server_comm_switches sw);
        uint8_t timer_checks(struct relay_timer *rt, struct relay_timer *temp_timer, uint8_t* count);
        uint8_t encode_status_byte();
        int32_t* get_clicks_ptr(struct relay *r);
        uint32_t* get_on_time_ptr(struct relay *r);
        void add_timer(struct relay_timer *rt, uint32_t epoch_time, uint8_t relay_switch, uint8_t timer_status, uint8_t* count);

        
    public:
        // Needed for ISR which is why they are stored in IRAM.
        struct relay* IRAM_ATTR get_relay(server_comm_switches number);
        struct relay* IRAM_ATTR get_relay(uint8_t number);
        struct relay_timer* get_relay_hard_schedule(uint8_t number);
        struct relay_timer* get_relay_hard_schedule(server_comm_switches number);
        void all_lock();
        void all_unlock();
        void all_off();
        void set_all_brightness(uint8_t brightness);
        void force_all_off();
        void force_all_on();
        void all_on();
        int32_t* get_clicks_ptr(server_comm_switches number);
        uint32_t* get_on_time_ptr(server_comm_switches number);
        void reset_relay_clicks(server_comm_switches number);
        uint8_t update_relay(struct relay *r, uint8_t lock, uint8_t status);
        uint8_t update_relay(struct relay *r, uint8_t lock, uint8_t status, uint8_t brightness);
        uint8_t set_state(struct relay *r, uint8_t r_mode);
        void add_timer(struct relay_timer *rt, struct relay_timer *temp_timer, uint8_t* count);
        uint16_t build_config_response_data(char* char_arr);
        uint16_t build_response_data(char* char_arr);
        uint16_t build_response_data(char* prefix, char* char_arr);
        uint16_t build_response_data(String prefix_str, char* char_arr);
        uint8_t update_timer(struct relay_timer *rt, struct relay_timer *temp_timer, uint8_t count);
        void remove_timer(struct relay_timer *rt, uint8_t timer_index, uint8_t* count);
        int any_relays_on();
        void reset_all_usage();
        void reset_on_time();
        void reset_clicks_usage();
        void reset_current_usage();
        void current_usage_all_day(double* current_sum);
        void clicks_usage_all_day(double* clicks_sum);
        Relays(){};
        void init();
        void loop();
};

#include "RealTimeClock.h"
#include "FilesManager.h"
#include "BroadcastManager.h"
#include "esp_log.h"
#endif