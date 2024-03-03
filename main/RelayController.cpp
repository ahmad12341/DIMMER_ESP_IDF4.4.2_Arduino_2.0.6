#include "RelayController.h"

/** 
 * NOTE: An interface to define the output functions would be very helpful here so we can define different output methods.
**/

extern ServerComms server_comms;



void Relays::init(){	
	for(int i=0; i < NUM_CHANNELS; i++){
		_relays_arr[i]->name = _relay_names[i];
		_relays_arr[i]->pin = _relay_pins[i];
		set_brightness(_relays_arr[i], SYSTEM_CONFIGURATION.startup_brightness);
		// _relays_arr[i]->brightness = SYSTEM_CONFIGURATION.startup_brightness; // Set brightness to 50% on restart as we aren't saving state every time brightness changed.
		pinMode(_relays_arr[i]->pin, OUTPUT);
		log_d("Relay Status %c : %i", _relays_arr[i]->name, _relays_arr[i]->status);
		// digitalWrite(_relays_arr[i]->pin, _relays_arr[i]->status);
	}
}


void Relays::loop(){
	if(temperature_c > 85.0){
		log_w("Temperature Limit Reached - Turning OFF all channels");
		all_unlock(); // Make sure everything is unlocked so we can toggle it off.
		all_off(); // Toggle everything OFF.
		String msg = "WARN,temperatureLimit";
		broadcast_send(msg);
		mqtt_publish_msg(msg);
	}

	if(activepower > 900){
		log_w("Active Power Limit Reached - Turning OFF all channels.");
		all_unlock(); // Make sure everything is unlocked so we can toggle it off.
		all_off(); // Toggle everything OFF.
		String msg = "WARN,powerLimit";
		broadcast_send(msg);
		mqtt_publish_msg(msg);
	}
    static uint32_t time = 0;
    if (millis() - time >= 1000){
		for(int i=0; i< NUM_CHANNELS; i++){
			process_ontime(_relays_arr[i]);
		};
		time = millis();
    }
}

uint8_t Relays::encode_status_byte(){
	/**
	 * @brief Encodes the relay status into a single byte:
	 * [7]: Relay D Lock (0 - Unlocked | 1 - Locked) 
	 * [6]: Relay C Lock (0 - Unlocked | 1 - Locked) 
	 * [5]: Relay D status (0 - OFF | 1 - ON) 
	 * [4]: Relay C status (0 - OFF | 1 - ON) 
	 * [3]: Relay B Lock (0 - Unlocked | 1 - Locked) 
	 * [2]: Relay A Lock (0 - Unlocked | 1 - Locked) 
	 * [1]: Relay B status (0 - OFF | 1 - ON) 
	 * [0]: Relay A status (0 - OFF | 1 - ON) 
	 * 
	 */
	uint8_t byte_to_send = 0 | _relay_d.lock << 7| _relay_c.lock << 6 |
							_relay_d.status << 5| _relay_c.status << 4 |
							_relay_b.lock << 3 | _relay_a.lock << 2 | 
							_relay_b.status << 1 | _relay_a.status;

	return byte_to_send;
}

void Relays::process_ontime(struct relay *r){
    if (r->status){
		uint32_t epoch = rtc_get_epoch();
		r->on_time += (epoch - r->start_time);
		r->start_time = epoch;
	}
}

void Relays::force_all_off(){
	/**
	 * @brief Helper function when testing Dimmer - Allows off without state change.
	 * 
	 */
	for(int i=0; i < NUM_CHANNELS; i++){
		digitalWrite(_relays_arr[i]->pin, 0);
	}
}

void Relays::force_all_on(){
	/**
	 * @brief Helper function when testing Dimmer - Allows off without state change.
	 * 
	 */
	for(int i=0; i < NUM_CHANNELS; i++){
		digitalWrite(_relays_arr[i]->pin, 1);
	}
}

void Relays::set_off(struct relay *r, uint8_t r_mode){
    r->status = 0;
    state_change_updater(r, r_mode);
}

void Relays::set_on(struct relay *r, uint8_t r_mode){
    r->status = 1;
    state_change_updater(r, r_mode);

}

void Relays::set_toggle(struct relay *r, uint8_t r_mode){
    if (r->status){
        log_d("Relay %c TOGGLE [ON->OFF]", r->name);
        set_off(r, r_mode);
    }else{
		log_d("Relay %c TOGGLE [OFF->ON]", r->name);
        set_on(r, r_mode);
    }
}

void Relays::state_change_updater(struct relay *r, uint8_t r_mode){
    /***
     *  Perform updates required after state change. 
     ***/
	log_d("State Change Updater");
    r->clicks_usage++;
	broadcast_send_switch_status();
	Response_package resp;
	resp.cmd_i_val = 0x3C;
	resp.req_id = "0000";
	resp.resp_type = SCR_MQTT;
	server_comms.send_switch_status(resp);
    if (r_mode == STATUS_OFF){
        uint32_t epoch = rtc_get_epoch();
	    r->on_time += (epoch - r->start_time);
    }else{
        r->start_time = rtc_get_epoch();
        hard_timer_handler(r, r_mode);
    }
}

void Relays::set_schedules(struct relay_timer *rt, uint16_t hard_timer, server_comm_switches sw){
	rt->relay_switch = sw;
	rt->repeat_count = 0;
	rt->epoch_time = rtc_get_epoch() + hard_timer;
	rt->timer_status = 0;
	log_d("Setting Hard Timer %u | Epoch %lu | Timer: %u ", sw, rt->epoch_time, hard_timer);
}

void Relays::hard_timer_handler(struct relay *r, uint8_t r_mode){
	log_d("Hard Timer Handler");
    if(SYSTEM_CONFIGURATION.hard_timer_a > 10 && r->name == 'a'){
		struct relay_timer *rt = get_relay_hard_schedule(SWITCH_A);
		set_schedules(rt, SYSTEM_CONFIGURATION.hard_timer_a, SWITCH_A);

	}else if(SYSTEM_CONFIGURATION.hard_timer_b > 10 && r->name == 'b'){
		struct relay_timer *rt = get_relay_hard_schedule(SWITCH_B);
		set_schedules(rt, SYSTEM_CONFIGURATION.hard_timer_b, SWITCH_B);

	}else if(SYSTEM_CONFIGURATION.hard_timer_c > 10 && r->name == 'c'){
		struct relay_timer *rt = get_relay_hard_schedule(SWITCH_C);
		set_schedules(rt, SYSTEM_CONFIGURATION.hard_timer_c, SWITCH_C);

	}else if(SYSTEM_CONFIGURATION.hard_timer_d > 10 && r->name == 'd'){
		struct relay_timer *rt = get_relay_hard_schedule(SWITCH_D);
		set_schedules(rt, SYSTEM_CONFIGURATION.hard_timer_d, SWITCH_D);
	}
}
void  Relays::set_all_brightness(uint8_t brightness){
	/**
	 * @brief Sets the brightness for all channels.
	 * 
	 */
	for(int i=0; i < NUM_CHANNELS; i++){
		set_brightness(_relays_arr[i], brightness);
	}
}
void Relays::all_off(){
	for(int i=0; i < NUM_CHANNELS; i++){
		log_d("Relay A: %p | Relay B: %p | Relay C: %p | Relay D: %p", &_relay_a, &_relay_b, &_relay_c, &_relay_d);
		log_d("Relay %c | Lock Status: %d | addr: %p", _relays_arr[i]->name, _relays_arr[i]->lock, _relays_arr[i]);
		uint8_t lock = _relays_arr[i]->lock ? LOCK_LOCK : LOCK_UNLOCK;
		update_relay(_relays_arr[i], lock, STATUS_OFF, _relays_arr[i]->brightness);
		// set_off(_relays_arr[i], STATUS_OFF);
	}
}

void Relays::all_on(){
	for(int i=0; i < NUM_CHANNELS; i++){
		log_d("Relay %c | Lock Status: %d", _relays_arr[i]->name, _relays_arr[i]->lock);
		uint8_t lock = _relays_arr[i]->lock ? LOCK_LOCK : LOCK_UNLOCK;
		update_relay(_relays_arr[i], lock, STATUS_ON, _relays_arr[i]->brightness);
	}
}

void Relays::all_lock(){
	for(int i=0; i < NUM_CHANNELS; i++){
		update_relay(_relays_arr[i], LOCK_LOCK, STATUS_STATUS, _relays_arr[i]->brightness);
	}
}

void Relays::all_unlock(){
	for(int i=0; i < NUM_CHANNELS; i++){
		update_relay(_relays_arr[i], LOCK_UNLOCK, STATUS_STATUS, _relays_arr[i]->brightness);
	}
}

uint8_t Relays::set_state(struct relay *r, uint8_t r_mode){
	uint8_t num_changes = 0;
	if (r->lock){
		log_d("Relay %c Locked! - New State Not Applied.", r->name);
		return num_changes;
	}
	if((r_mode == STATUS_OFF && r->status !=0) || (r_mode == STATUS_ON && r->status != 1) || (r_mode == STATUS_TOGGLE)){
		// Only log if somethong os actually going to change.
		log_d("Setting Relay %c | pin %d | status: %d | lock: %d | new state: %d", r->name, r->pin, r->status, r->lock, r_mode);
		num_changes++;
	}
    if (r_mode == STATUS_OFF && r->status != 0){
		set_off(r, r_mode);
		log_d("Turned OFF relay %c", r->name);
	}
	else if (r_mode == STATUS_ON && r->status != 1){
		set_on(r, r_mode);
		log_d("Turned ON relay %c", r->name);

	}else if (r_mode == STATUS_TOGGLE){
        set_toggle(r, r_mode);
	}
	return num_changes;
}
uint8_t Relays::update_relay(struct relay *r, uint8_t lock, uint8_t status){
	/**
	 * @brief Update Relay with default brightness of 100%
	 * 
	 */
	// Check if lock is different 
	return update_relay(r, lock, status, 100);
}

uint8_t Relays::set_brightness(struct relay *r, uint8_t brightness){
	/**
	 * @brief Verifiys the brightness is in the correct range then sets it.
	 * 
	 */
	if(brightness > SYSTEM_CONFIGURATION.brightness_ceiling_percentage){
		brightness = SYSTEM_CONFIGURATION.brightness_ceiling_percentage;
	}else if(brightness < SYSTEM_CONFIGURATION.brightness_floor_percentage){
		brightness = SYSTEM_CONFIGURATION.brightness_floor_percentage;
	}
	if(0 <= brightness  && brightness <= 100 && r->brightness != brightness){
	
		r->brightness = brightness;
		return 1;
	}else{
		// log_d("Failed to set Brightness - Out of range: Relay %c | Brightness: %d", r->name, r->brightness);
		return 0;
	}
}

uint8_t Relays::update_relay(struct relay *r, uint8_t lock, uint8_t status, uint8_t brightness){
	uint8_t num_changes = 0;
    if (lock == LOCK_UNLOCK && r->lock != 0){
		r->lock = 0;
		log_d("Setting Relay %c | Unlocked: %d", r->name, r->lock);
		num_changes++;
	}else if (lock == LOCK_LOCK && r->lock != 1){
		r->lock = 1;
		log_d("Setting Relay %c | Locked: %d", r->name, r->lock);
		num_changes++;
	}
	set_brightness(r, brightness); // Set brightness now doesn't count as program change.
	// num_changes += set_brightness(r, brightness);
	num_changes += set_state(r, status);
	return num_changes;
}

struct relay* IRAM_ATTR Relays::get_relay(server_comm_switches number){
	/**
	 * @brief Simple Getter - Stored in IRAM as this is called in dimmer ISR.
	 */
	return _relays_arr[number];
}

struct relay* IRAM_ATTR Relays::get_relay(uint8_t number){
	return _relays_arr[number];
}

struct relay_timer* Relays::get_relay_hard_schedule(server_comm_switches number){
	return _relay_schedules_arr[number];
}

struct relay_timer* Relays::get_relay_hard_schedule(uint8_t number){
	return _relay_schedules_arr[number];
}

uint8_t Relays::timer_checks(struct relay_timer *rt, struct relay_timer *temp_timer, uint8_t* count){
    if (*count >= 16){
		return 0; // What to do when we have 16 timers? nothing for now
	}
    // Check if timer already exists.
	for (int i = 0; i < *count; i++){
		if(rt[i].epoch_time == temp_timer->epoch_time && rt[i].timer_status == temp_timer->timer_status && rt[i].relay_switch == temp_timer->relay_switch){
			return 0;
		}
	}
    return 1;
}

void Relays::add_timer(struct relay_timer *rt, uint32_t epoch_time, uint8_t relay_switch, uint8_t timer_status, uint8_t* count){
    struct relay_timer temp_timer;
    temp_timer.epoch_time = epoch_time;
    temp_timer.relay_switch = relay_switch;
    temp_timer.timer_status = timer_status;

    add_timer(rt, &temp_timer, count);
}

void Relays::add_timer(struct relay_timer *rt, struct relay_timer *temp_timer, uint8_t* count){
    if(!timer_checks(rt, temp_timer, count)){
        return;
    }
	uint32_t rtc_epoch_time = rtc_get_epoch();
    log_d("Schedule Epoch Time: %lu | RTC Epoch Time: %lu",  temp_timer->epoch_time, rtc_epoch_time);
    rt[*count].epoch_time = temp_timer->epoch_time;
	rt[*count].repeat_count = temp_timer->repeat_count;// + 1;
	rt[*count].duration_repeat = temp_timer->duration_repeat;
	rt[*count].relay_switch = temp_timer->relay_switch;
	rt[*count].timer_status = temp_timer->timer_status;
	rt[*count].timer_duration = temp_timer->timer_duration;
	rt[*count].weekdays = temp_timer->weekdays;

	*count = *count + 1;
	files_manager_write_config();
}

uint8_t Relays::update_timer(struct relay_timer *rt, struct relay_timer *temp_timer, uint8_t count){
	int8_t id = temp_timer->timer_id;
    if (id < 0 || id >= count){
		return 0;
	}

    rt[id].epoch_time = temp_timer->epoch_time;
	rt[id].repeat_count = temp_timer->repeat_count;
	rt[id].duration_repeat = temp_timer->duration_repeat;
	rt[id].relay_switch = temp_timer->relay_switch;
	rt[id].timer_status = temp_timer->timer_status;
	rt[id].timer_duration = temp_timer->timer_duration;
	rt[id].weekdays = temp_timer->weekdays;
	files_manager_write_config();
	return 1;
}

void Relays::remove_timer(struct relay_timer *rt, uint8_t timer_index, uint8_t* count){
    if(*count == 0 || timer_index >= *count)
		return;
	
	if ((timer_index + 1) == *count){ // If we are removing the last timer
		*count = *count - 1;
		return;
	}
    // Shift Timers to Left in Array.
    for (int i = timer_index; i < (*count - 1); i++){
		rt[i] = rt[i + 1];
	}
    files_manager_write_config();
	*count = *count - 1;
	return;
}

void Relays::reset_all_usage(){
	for(int i=0; i < NUM_CHANNELS; i++){
		_relays_arr[i]->clicks_usage = 0;
		_relays_arr[i]->current_usage =0;
	};
	current_usage = 0;
}

void Relays::reset_current_usage(){
	for(int i=0; i < NUM_CHANNELS; i++){
		_relays_arr[i]->current_usage =0;
	};
    current_usage = 0;
}

void Relays::reset_clicks_usage(){
	for(int i=0; i < NUM_CHANNELS; i++){
		_relays_arr[i]->clicks_usage = 0;
	};
}

void Relays::reset_on_time(){
	for(int i=0; i < NUM_CHANNELS; i++){
		_relays_arr[i]->on_time = 0;
	};
}

void Relays::reset_relay_clicks(server_comm_switches number){
	struct relay* r = get_relay(number);
	r->clicks_usage = 0;
}

void Relays::current_usage_all_day(double* current_sum){
    *current_sum = current_usage;
}

void Relays::clicks_usage_all_day(double* clicks_sum){
    *clicks_sum = current_usage;
}

uint32_t* Relays::get_on_time_ptr(server_comm_switches number){
	return get_on_time_ptr(get_relay(number));
}

uint32_t* Relays::get_on_time_ptr(struct relay *r){
	return &r->on_time;
}

int32_t* Relays::get_clicks_ptr(server_comm_switches number){
	return get_clicks_ptr(get_relay(number));
}

int32_t* Relays::get_clicks_ptr(struct relay *r){
	return &r->clicks_usage;
}
int Relays::any_relays_on(){
	return (_relay_a.status == 1 || _relay_b.status == 1 || _relay_c.status == 1 || _relay_d.status == 1);
}

uint16_t  Relays::build_config_response_data(char* char_arr){
	uint16_t data_len = sprintf(char_arr, "%f,%d,%d,%d,%d", current_usage, _relay_a.clicks_usage, _relay_b.clicks_usage, _relay_c.clicks_usage, _relay_d.clicks_usage);
	return data_len;
}
uint16_t Relays::build_response_data(char* char_arr){
	/**
	 * @brief Put basic switch state data into char array.
	 * 
	 */
	uint8_t byte_to_send = encode_status_byte();
	uint16_t data_len = sprintf(char_arr, "%02X,%f,%d,%d,%d,%d,%d,%d,%d,%d,%d", byte_to_send, current_usage, _relay_a.clicks_usage, _relay_b.clicks_usage,  _relay_c.clicks_usage, _relay_d.clicks_usage, _relay_a.brightness, _relay_b.brightness, _relay_c.brightness, _relay_d.brightness, activepower);
	return data_len;
}

uint16_t Relays::build_response_data(char* prefix, char* char_arr){
	/**
	 * @brief Merge provided prefix and basic switch state data into char array.
	 * 
	 */
	uint8_t byte_to_send = encode_status_byte();
	uint16_t data_len = sprintf(char_arr, "%s,%02X,%f,%d,%d,%d,%d,%d,%d,%d,%d,%d", prefix, byte_to_send, current_usage, _relay_a.clicks_usage, _relay_b.clicks_usage,  _relay_c.clicks_usage, _relay_d.clicks_usage, _relay_a.brightness, _relay_b.brightness, _relay_c.brightness, _relay_d.brightness, activepower);
	return data_len;

}

uint16_t Relays::build_response_data(String prefix_str, char* char_arr){
	/**
	 * @brief Merge provided prefix and basic switch state data into char array.
	 * 
	 */
	int prefix_len = prefix_str.length() + 1;
	char* prefix = (char*)calloc(prefix_len + 2, 1);
	prefix_str.toCharArray(prefix, prefix_len);
	uint16_t data_len = build_response_data(prefix, char_arr);
	free(prefix);
	return data_len;
}