#include "Scheduler.h"


void Scheduler::init(Relays* relays, ServerComms* server_comms){
    _relays = relays;
	_server_comms = server_comms;
}

void Scheduler::loop(){
    // NOTE: This variable shadows global...
    long rtc_epoch_time = rtc_get_epoch();

    for (int i = 0; i < schedules_count; i++){
		schedules_handler(schedules[i], i, rtc_epoch_time);
	}

    for (int i = 0; i < timers_count; i++){
		timers_handler(timers[i], i, rtc_epoch_time);
	}
    // Clear any expired Schedules.
    expired_schedules(_relays->get_relay(SWITCH_A), _relays->get_relay_hard_schedule(SWITCH_A));
    expired_schedules(_relays->get_relay(SWITCH_B),_relays->get_relay_hard_schedule(SWITCH_B));
    expired_schedules(_relays->get_relay(SWITCH_C),_relays->get_relay_hard_schedule(SWITCH_C));
    expired_schedules(_relays->get_relay(SWITCH_D),_relays->get_relay_hard_schedule(SWITCH_D));
}

void Scheduler::schedules_handler(struct relay_timer &schedule, int index, long rtc_epoch_time){
    String return_string = "";

    if (schedule.epoch_time > rtc_epoch_time){
        return;
    }

    log_d("Executing schedule..");
	log_d("Schedule Epoch Time: %lu | RTC Epoch Time: %lu", schedule.epoch_time, rtc_epoch_time);
    timer_execute(&schedule);
    char* str = (char*)calloc(150, 1);
    String days = _server_comms->weekdays_string(schedule.weekdays);
    uint16_t str_len = days.length() + 1;
    char* char_array = (char*)calloc(str_len, 1);
    days.toCharArray(char_array, str_len);

    sprintf(str, "%d:%d:%s:%d:%d:%d:%d", index, schedule.epoch_time, char_array, schedule.repeat_count, schedule.duration_repeat, schedule.relay_switch, schedule.timer_status);
    return_string += str;
    struct Response_package resp_pack;
    resp_pack.cmd_i_val = 0xBC;
    resp_pack.req_id = "00000";
    resp_pack.resp_type = SCR_MQTT;
    _server_comms->send_custom(return_string, resp_pack);

    if (schedule.weekdays != 0){
        uint32_t value_to_add = get_next_day(schedule.weekdays, schedule.epoch_time);
        schedule.epoch_time += value_to_add;
	}
	else if (schedule.repeat_count < 0){
			_relays->remove_timer(schedules, index, &schedules_count);
    }else{
        schedule.epoch_time += schedule.duration_repeat;
    }
		
    free(str);
    free(char_array);
    program_status_change_flag = 1;
    //files_manager_write_program_status();

}
void Scheduler::start_timer(struct relay_timer &timer, int index, long rtc_epoch_time){
	String return_string = "";
    log_d("Executing timer..");
    log_d("Timer Epoch Time: %lu | RTC Epoch Time: %lu", timer.epoch_time, rtc_epoch_time);
    timer_execute(&timer);
    char* str = (char*)calloc(150, 1);

    String days = _server_comms->weekdays_string(timer.weekdays);
    uint16_t str_len = days.length() + 1;
    char* char_array = (char*)calloc(str_len, 1);
    days.toCharArray(char_array, str_len);
    
    sprintf(str, "%d:%d:%s:%d:%d:%d:%d:%d", index,timer.epoch_time, char_array, timer.repeat_count, timer.duration_repeat, timer.relay_switch, timer.timer_status, timer.timer_duration);
    return_string += str;
    struct Response_package resp_pack;
    resp_pack.cmd_i_val = 0x7C;
    resp_pack.req_id = "00000";
    resp_pack.resp_type = SCR_MQTT;
    _server_comms->send_custom(return_string, resp_pack);
    
    if (timer.weekdays != 0)
    {
        uint32_t value_to_add = get_next_day(timer.weekdays, timer.epoch_time);
        timer.epoch_time += value_to_add;
    }
    else if (timer.repeat_count > 0)
    {
        timer.epoch_time += timer.duration_repeat;
    }
    
    timer.timer_started = 1;
    timer.timer_start_ms = rtc_epoch_time;
    free(str);
    free(char_array);
    program_status_change_flag = 1;
    //files_manager_write_program_status();
}

void Scheduler::end_timer(struct relay_timer &timer, int index, long rtc_epoch_time){
    int32_t time_diff = rtc_epoch_time - timer.timer_start_ms;
    if (time_diff >= timer.timer_duration && time_diff > 0){
        log_d("epoch: %d | t_start: %d | diff: %d | duration: %d", rtc_epoch_time, timer.timer_start_ms, time_diff, timer.timer_duration);
        timer.timer_started = 0;
        timer.timer_start_ms = 0;
        revert_prev_state(&timer);
        if (timer.weekdays == 0 && timer.repeat_count < 0){
            _relays->remove_timer(timers, index, &timers_count);
        }
        program_status_change_flag = 1;
        //files_manager_write_program_status();
    }
}



void Scheduler::timers_handler(struct relay_timer &timer, int index, long rtc_epoch_time){
    if (timer.epoch_time <= rtc_epoch_time && !timer.timer_started){
        start_timer(timer, index, rtc_epoch_time);
    }
    if (timer.timer_started){
        end_timer(timer, index, rtc_epoch_time);
    }
}

void Scheduler::expired_schedules(struct relay *r, struct relay_timer *rt){
    uint32_t rtc_epoch_time = rtc_get_epoch();
    if (rt->epoch_time <= rtc_epoch_time){
        log_d("Hard Timer Triggered for Relay %c - Timer: %lu | Current: %lu", r->name, rt->epoch_time, rtc_epoch_time);
		_relays->set_state(r, STATUS_OFF);
		rt->epoch_time = 0xFFFFFFFF;
	}
}

void Scheduler::schedule_execute(struct relay *r, uint8_t r_mode, struct relay_timer *rt_execute){
    log_d("Schedule Execute Switch %c Mode: %d", r->name, r_mode);
    rt_execute->timer_state = r->status == 1 ? STATUS_ON : STATUS_OFF;
    _relays->set_state(r, r_mode);
}

void Scheduler::timer_execute(struct relay_timer *rt_execute){
    uint8_t r_mode = rt_execute->timer_status == 1 ? STATUS_ON : STATUS_OFF;
    rt_execute->repeat_count--;
    schedule_execute(_relays->get_relay(rt_execute->relay_switch), r_mode, rt_execute);
}

int32_t Scheduler::get_next_day(int8_t weekdays, uint32_t epoch){
    const int32_t one_day = 24 * 60 * 60;
    if (weekdays == 0){
		return 0;
	}

    int today = rtc_get_weekday(epoch);

    log_d("Weekdays: 0b %c%c%c%c %c%c%c%c | Today: %d", BYTE_TO_BINARY(weekdays), today);

    int8_t count = 0;
	int num_days = 0;
	for (int i = today + 1; i < 7; i++){
		int8_t shifted_value = (weekdays >> i) & 0b00000001;
		if (shifted_value == 1){
			num_days  = i - today;
			log_d("Number of Days Forward %d", num_days);
			break;
		}
	}
    
	if (num_days == 0){
		for (int i = 0; i <= today; i++){
			int8_t shifted_value = (weekdays >> i) & 0b00000001;
			if (shifted_value == 1){
				num_days = 6 - today + i + 1;
				log_d("Number of Days Around %d", num_days);
				return (num_days * one_day);
			}
		}
	}
	return (num_days * one_day);
}

void Scheduler::revert_prev_state(struct relay_timer *rt_execute){
    log_d("Reverting to state before timer/schedule.");
    struct relay* r = _relays->get_relay(rt_execute->relay_switch);
    if (rt_execute->timer_status == r->status){
            _relays->set_state(r, rt_execute->timer_state);
    }
}