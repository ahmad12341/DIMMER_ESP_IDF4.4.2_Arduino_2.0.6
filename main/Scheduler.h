#pragma once
#ifndef SCHEDULER_H_
#define SCHEDULER_H_
#include <Arduino.h>
#include "Globals.h"
#include "RelayController.h"
#include "RealTimeClock.h"
#include "ServerComm.h"
#include "Logging.h"


class Scheduler{

    private:
        Relays *_relays;
        ServerComms *_server_comms;

        void expired_schedules(struct relay *r, struct relay_timer *rt);
        void schedules_handler(struct relay_timer &schedule, int index, long rtc_epoch_time);
        void start_timer(struct relay_timer &timer, int index, long rtc_epoch_time);
        void end_timer(struct relay_timer &timer, int index, long rtc_epoch_time);

        void timers_handler(struct relay_timer &timer, int index, long rtc_epoch_time);
        void timer_execute(struct relay_timer *rt_execute);
        void schedule_execute(struct relay *r, uint8_t r_mode, struct relay_timer *rt_execute);
        int32_t get_next_day(int8_t weekdays, uint32_t epoch);
        void revert_prev_state(struct relay_timer *rt_execute);
    public:
        Scheduler(){};
        void init(Relays* relays,  ServerComms* server_comms);
        void loop();
};

#endif