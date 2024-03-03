#pragma once
#ifndef SWITCH_INPUT_H_
#define SWITCH_INPUT_H_
#include <Arduino.h>
#include "RelayController.h"

#include "Globals.h"
#include "Watchdog.h"
#include "FilesManager.h"
#include "Logging.h"


#define SWITCH_PIN_A    32
#define SWITCH_PIN_B    33
#define SWITCH_PIN_C	34
#define SWITCH_PIN_D 	35
// Switch is checked 3x at 5ms intervals which means switch must be active for 15ms total to be valid.
#define SWITCH_POLLING_COUNT 3
#define SWITCH_POLLING_TIME_MS 5

struct switch_t{
    int pin;
    uint8_t flag = 0;
    uint32_t start_time = 0;
    uint16_t counter = 0;
    struct relay* relay;
};
class Switches{
    private:
        Relays* _relays;
        int _switch_pins[NUM_CHANNELS] = {SWITCH_PIN_A, SWITCH_PIN_B, SWITCH_PIN_C, SWITCH_PIN_D};

        struct switch_t _switch_a, _switch_b, _switch_c, _switch_d;
        struct switch_t* _switch_arr[NUM_CHANNELS] = {&_switch_a, &_switch_b, &_switch_c, &_switch_d};
        
        void button_loop();
        void switch_loop();
        void reset_handler();
        bool reset_triggered(uint16_t switch_counter, uint32_t switch_start_time);
        uint8_t poll_switch(int8_t pin, uint8_t loops, uint8_t polling_delay_ms, uint8_t expected_state);
        void switch_handler(struct switch_t* sw);
        void button_handler(struct switch_t* sw);
        void init_state(uint8_t &switch_flag, struct relay* r);
    public:
        Switches(){};
        void init(Relays* relays);
        void loop();
};
#endif