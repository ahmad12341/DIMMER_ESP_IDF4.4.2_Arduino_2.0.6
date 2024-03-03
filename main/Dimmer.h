/**
 * Zero Crossing detection for Syncronysing the Triac.
 * 
 */
#pragma once
#ifndef ZERO_CROSSING_H_
#define ZERO_CROSSING_H_
#include <Arduino.h>
#include "esp_timer.h"
#include "Globals.h"
#include "RelayController.h"
#include "driver/gpio.h"
#include "Logging.h"

#define ZERO_CROSSING_PIN   GPIO_NUM_17
#define ZERO_CROSSING_PIN_MASK (1ULL<<ZERO_CROSSING_PIN)
#define MAINS_HALF_CYCLE_US 10000ULL
#define ESP_INTR_FLAG_DEFAULT 0

// TODO: End triac timer can be simplified and this struct split in two.
struct triac_timer_t{
    esp_timer_handle_t timer;
    esp_timer_create_args_t config;
    uint8_t channel_num;
    struct relay* relay;
    int64_t set_time;
    uint64_t timeout_us;
    bool timer_active = false;
    void *end_timer; // Cast to instance of self.
};

class Dimmer{
    private:
        const char _a_name = 'a',  _b_name = 'b', _c_name = 'c', _d_name = 'd'; 
        const char* _channel_names[NUM_CHANNELS] = {&_a_name, &_b_name, &_c_name, &_d_name};
        

        String _db_timer_name = "db_t";

        // Timers for handling triggering the triac after some delay.
        struct triac_timer_t _triac_timer_a,_triac_timer_b,_triac_timer_c, _triac_timer_d;
        struct triac_timer_t* _triac_timers_arr[NUM_CHANNELS] = {&_triac_timer_a, &_triac_timer_b, &_triac_timer_c, &_triac_timer_d};

        // Timers for handling turning OFF the triac pulse - This is so they are non-blocking.
        struct triac_timer_t _triac_end_timer_a,_triac_end_timer_b,_triac_end_timer_c,_triac_end_timer_d;
        struct triac_timer_t* _triac_end_timers_arr[NUM_CHANNELS] = {&_triac_end_timer_a, &_triac_end_timer_b, &_triac_end_timer_c, &_triac_end_timer_d};

        void init_zero_crossing();
        struct triac_timer_t*  init_triac_timer(struct triac_timer_t* timer, uint8_t channel_num, const char* name, void (*callback)(void *), bool main_timer);
        void start_triac_timer(uint8_t channel_num);
        void init_debounce_timer();
        void init_fallback_timer();
        void testing_signal_generator();
        void deinit_triac_timer(struct triac_timer_t* timer, uint8_t channel_num, bool main_timer);
    public:
        uint8_t currently_trailing_edge = 0;

        Dimmer(){};
        void init();
        void reinit();
        void loop();
        esp_timer_handle_t debounce_timer;
        esp_timer_create_args_t debounce_timer_config;

        esp_timer_handle_t fallback_timer;
        esp_timer_create_args_t fallback_timer_config;
        triac_timer_t* IRAM_ATTR get_triac_timer(uint8_t channel_num); // This needs to be in IRAM as it is called in the ISR.
};
#endif