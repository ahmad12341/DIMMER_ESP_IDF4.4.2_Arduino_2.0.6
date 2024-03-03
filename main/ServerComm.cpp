#include "ServerComm.h"

void ServerComms::init(Relays* relays){
    _relays = relays; 
}

void ServerComms::set_timer(uint8_t cmd_i_val, struct relay_timer* temp_timer, struct Response_package response){
    if (temp_timer->timer_id != -1){
        String msg = "Invalid timer command";
        send_custom(msg, response);
        return;
    }
    if (timers_count == TIMERS_MAX_COUNT){
        String response_msg = "T_M";
        send_custom(response_msg, response);
        return;
    }
    _relays->add_timer(timers, temp_timer, &timers_count);
    send_timer_status(response, CMD_TIMER);
    program_status_change_flag = 1;
    //files_manager_write_program_status();
}

void ServerComms::update_timer(uint8_t cmd_i_val, struct relay_timer* temp_timer, struct Response_package response){
    if (_relays->update_timer(timers, temp_timer, timers_count)){
        String response_msg = "Timer Updated";
        send_custom(response_msg, response);
    }else{
        String response_msg = "Invalid Timer ID";
        send_custom(response_msg, response);
    }
    program_status_change_flag = 1;
    //files_manager_write_program_status();
}

void ServerComms::delete_timer(uint8_t cmd_i_val, struct relay_timer* temp_timer, struct Response_package response){
    _relays->remove_timer(timers, temp_timer->timer_id, &timers_count);
    send_timer_status(response, CMD_TIMER);
    program_status_change_flag = 1;
    //files_manager_write_program_status();
}

void ServerComms::read_timer(uint8_t cmd_i_val, struct relay_timer* temp_timer, struct Response_package response){
    send_timer_status(response, CMD_TIMER);
}

void ServerComms::send_timer_status(struct Response_package response, uint8_t cmd_type){
   char* str = (char*)calloc(150, 1);
	uint8_t relay_switch;
	uint8_t timer_status;
	uint8_t timerByte;
	String return_string = "";
	struct relay_timer *rt;
	uint8_t counter;
	uint8_t static_byte;
	String non_response;
	if (cmd_type == CMD_SCHEDULER)
	{
		rt = schedules;
		counter = schedules_count;
		static_byte = 0b10000000;
		non_response = "NO_SCHEDULERS";
		
		for (int i = 0; i < counter; i++)
		{
			if (i > 0)
				return_string += ",";
	
			String days = weekdays_string(schedules[i].weekdays);
			uint16_t str_len = days.length() + 1;
			char* char_array = (char*)calloc(str_len, 1);
			days.toCharArray(char_array, str_len);
			sprintf(str, "%d:%d:%s:%d:%d:%d:%d", i, schedules[i].epoch_time, char_array, schedules[i].repeat_count, schedules[i].duration_repeat, schedules[i].relay_switch, schedules[i].timer_status);
			return_string += str;
			free(char_array);
		}
	}
	else if (cmd_type == CMD_TIMER)
	{
		rt = timers;
		counter = timers_count;
		static_byte = 0b01000000;
		non_response = "NO_TIMERS";
		
		for (int i = 0; i < counter; i++)
		{
			if (i > 0)
				return_string += ",";
			
			String days = weekdays_string(timers[i].weekdays);
			uint16_t str_len = days.length() + 1;
			char* char_array = (char*)calloc(str_len, 1);
			days.toCharArray(char_array, str_len);
			sprintf(str, "%d:%d:%s:%d:%d:%d:%d:%d", i, timers[i].epoch_time, char_array, timers[i].repeat_count, timers[i].duration_repeat, timers[i].relay_switch, timers[i].timer_status, timers[i].timer_duration);
			return_string += str;
			free(char_array);
		}
	}
	
	if (return_string.length() == 0)
		return_string = non_response;
		//return;
	
	send_custom(return_string, response);

	free(str);
}


void ServerComms::process_timer(uint8_t cmd_i_val, struct relay_timer* temp_timer, struct Response_package response){
    if (cmd_i_val == 0x40){ 
        set_timer(cmd_i_val, temp_timer, response);
    }else if (cmd_i_val == 0x41) {
        update_timer(cmd_i_val, temp_timer, response);
    }else if (cmd_i_val == 0x42){
        delete_timer(cmd_i_val, temp_timer, response);
    }else if (cmd_i_val == 0x43){
        read_timer(cmd_i_val, temp_timer, response);
    }
}
void ServerComms::set_schedule(uint8_t cmd_i_val, struct relay_timer* temp_timer, struct Response_package response){
    if (temp_timer->timer_id != -1){
        String msg = "Invalid timer command";
        log_d("Invalid Timer Command.");
        send_custom(msg, response);
        return;
	}
		
    if (schedules_count == TIMERS_MAX_COUNT){
        String response_msg = "S_M";
        log_d("Max Timers Reached.");
        send_custom(response_msg, response);
        return;
    }
    log_d("Adding Schedule...");
    _relays->add_timer(schedules, temp_timer, &schedules_count);
    send_timer_status(response, CMD_SCHEDULER);
	program_status_change_flag = 1;
    //files_manager_write_program_status();
}

void ServerComms::update_schedule(uint8_t cmd_i_val, struct relay_timer* temp_timer, struct Response_package response){
    if(_relays->update_timer(schedules, temp_timer, schedules_count)){
        String response_msg = "Schedule Updated";
        send_custom(response_msg, response);
    }else{
        String response_msg = "Invalid Schedule ID";
        send_custom(response_msg, response);
    }
    program_status_change_flag = 1;
    //files_manager_write_program_status();

}
void ServerComms::delete_schedule(uint8_t cmd_i_val, struct relay_timer* temp_timer, struct Response_package response){
   	_relays->remove_timer(schedules, temp_timer->timer_id, &schedules_count);
    send_timer_status(response, CMD_TIMER);
    program_status_change_flag = 1;
    //files_manager_write_program_status();
}

void ServerComms::read_schedule(uint8_t cmd_i_val, struct relay_timer* temp_timer, struct Response_package response){
    send_timer_status(response, CMD_TIMER);
}

void ServerComms::process_schedule(uint8_t cmd_i_val, struct relay_timer* temp_timer, struct Response_package response){
    log_d("Server Comm Scheduler Command: %i", cmd_i_val);
    if (cmd_i_val == 0x80){
        set_schedule(cmd_i_val, temp_timer, response);
    }else if (cmd_i_val == 0x81){
        update_schedule(cmd_i_val, temp_timer, response);
    }else if (cmd_i_val == 0x82){
        delete_schedule(cmd_i_val, temp_timer, response);
    }else if (cmd_i_val == 0x83){
        read_schedule(cmd_i_val, temp_timer, response);
    }
}

void ServerComms::process_switch(struct Command cmd, struct Response_package response){
	/**
	 * @brief For Dimmer the default process switch gets the relay and passes the current brightness.
	 * 
	 */
	struct relay* r = _relays->get_relay(cmd.Switch);
	process_switch(cmd, r->brightness, response);
}


void ServerComms::process_switch(struct Command cmd, uint8_t brightness, struct Response_package response){
	/**
	 * @brief Handles the swtich input with a brightness value.
	 * 
	 */
	struct relay* r = _relays->get_relay(cmd.Switch);
	uint8_t num_changes = _relays->update_relay(r, cmd.lock_status, cmd.status, brightness);
    send_switch_status(response); 
	//NOTE: Writing the program status appears to be a cause for significant delays in the dimming.
	log_d("Number of changes: %u", num_changes);
	if(num_changes !=0){
		program_status_change_flag = 1;
	}
    //files_manager_write_program_status();
}

void ServerComms::send_switch_status(struct Response_package response){
	/**
	 * @brief Send switch status to the server:
	 * 
	 *	4 Channel Response Format:
	 *	encoded_status_byte, current_usage, relay_a_clicks, relay_b_clicks,  relay_c_clicks,  relay_d_clicks, activepower. 
	 * 	
	 * 	2 Channel Response Format (Outdated):
	 * 	encoded_status_byte, current_usage, relay_a_clicks, relay_b_clicks,activepower. 
	 */
    char* char_arr = (char*)calloc(200, 1);
	uint16_t data_len = _relays->build_response_data(char_arr);
	send_response(char_arr, data_len, response);
    free(char_arr);
}

void ServerComms::send_custom(String response_msg, struct Response_package response_pack){
    send_response(response_msg, response_pack);
}

uint8_t ServerComms::config_wifi(String params[], int16_t params_count){
    uint8_t wifi = 0, pwd = 0;
	for (int i = 0; i < params_count; i++){
		if (params[i] == SYS_CONFIG_WIFI_SSID){
			wifi = 1;
		}
		if (params[i] == SYS_CONFIG_WIFI_PWD){
			pwd = 1;
		}
	}
	return wifi | pwd;
}

void ServerComms::owner_changed(String params[], int16_t params_count){
    uint8_t serial_flag = 0, device_flag = 0, user_flag = 0;
	for (int i = 0; i < params_count; i++){
		if (params[i] == SYS_CONFIG_OWNER){
			if (SYSTEM_CONFIGURATION.owner != "infinite"){
				SYSTEM_CONFIGURATION.config_owner_changed = 1;
			}
			if (SYSTEM_CONFIGURATION.owner == params[i + 1]){
				SYSTEM_CONFIGURATION.config_owner_changed = 0;
			}
			return;
		}
		
		if (params[i] == SYS_CONFIG_SERIALNO){
			serial_flag = 1;
		}else if (params[i] == SYS_CONFIG_DEVICENO){
			device_flag = 1;
		}else if (params[i] == SYS_CONFIG_DEVICE_USER){
			user_flag = 1;
		}
	}
	
	if (serial_flag && device_flag && user_flag){
		SYSTEM_CONFIGURATION.config_owner_changed = 0;
	}
}
void ServerComms::process_config_c(String params[], int16_t params_count, struct Command cmd, struct Response_package response){
    if (cmd.cmd_i_value == 0xC1){ // Config Get All
        send_response(system_config_get_all_config(), response);
    }else if (cmd.cmd_i_value == 0xC2){ // Config Get
        if (params_count > 0 && params[0] == "k"){
			String config_ret = system_config_get_by_params(params, params_count);
			send_response(config_ret, response);
		}else{
			send_false(response);
		}
    }else if (cmd.cmd_i_value == 0xC3){ // Config Set
        if ((params_count % 2) == 0 && params_count > 0){
			// we should be getting pairs
			uint8_t wifi_ret = config_wifi(params, params_count);
			owner_changed(params, params_count);
			system_config_set_by_params(params, params_count);
			send_true(response);
			system_config_check_config_request();
			
			if (wifi_ret){
                log_d("Wifi Params set");
				SYSTEM_CONFIGURATION.wifi_configured = 1;
				files_manager_write_config();
				delay(1000);
				esp_restart();
				wifi_load_sta_info();
				wifi_sta();
			}
		}else{
			send_false(response);
        }
    }else if (cmd.cmd_i_value == 0xC4){ // Config Mode
        if (SYSTEM_CONFIGURATION.use_token){
			SYSTEM_CONFIGURATION.use_token = 0;
			send_false(response);
		}else{
			SYSTEM_CONFIGURATION.use_token = 1;
			send_true(response);
		}
		files_manager_write_config();
    }else if (cmd.cmd_i_value == 0xC5){ // Hotspot mode
        if (wifi_hotspot_sta()){
            send_true(response);
        }else{
			send_false(response);
        }
    }else if (cmd.cmd_i_value == 0xC6){  // Reboot device
        send_true(response);
		SYSTEM_CONFIGURATION.restart_broadcast = 1;
		files_manager_write_config();
		program_status_change_flag = 1;
    	//files_manager_write_program_status();
		ESP.restart();
    }else if (cmd.cmd_i_value == 0xC7){ // OTA Update with url
        if (params[0] != "url" || params_count < 2){
			send_false(response);
		}
		send_true(response);
		char* buff = (char*)calloc(params[1].length() + 2, 1);
		char mybuff[] = "http://firmware.infiniteautomation.net/master/FM/DM_26_02_24/main.bin";
		
		params[1].toCharArray(buff, params[1].length() + 1);
		OTA ota_instance;
		if (ota_instance.begin(mybuff)){
			// log_d("OTA Temporarly Disabled!");
			send_true(response);
			files_manager_write_config();
			files_manager_write_uuid_tokens();
			esp_restart();
		}else{
			String ret = "OTA Failed";
			send_response(ret, response);
		}
		free(buff);
    }else if (cmd.cmd_i_value == 0xC8) { // OTA return version number
        String ret = "{\"v\":\"" + FIRMWARE_VERSION_NUMBER + "\",\"s\":\"" + FIRMWARE_VERSION_DESCRIPTION + "\",\"d\":\"" + ota_date_time + "\"}";
		send_response(ret, response);
    }else if (cmd.cmd_i_value == 0xCA){ // Config WIFI Status
        String ret = "{\"s\":\"1\",\"e\":\"\"}";
		send_response(ret, response);
    }else if (cmd.cmd_i_value == 0xCB){ // Config MQTT status
        String ret = "";
		if (mqtt_client_connected){
			ret = "{\"s\":\"1\",\"e\":\"\"}";
		}else{
			ret = "{\"s\":\"0\",\"e\":\"\"}";
		}
		send_response(ret, response);
    }else if (cmd.cmd_i_value == 0xCC){ // Beep N, we don't have a speaker
        int beep_count = params[0].toInt();
        buzzer_beep_n_times(beep_count);
		send_true(response);
    }else if (cmd.cmd_i_value == 0xCD){ // Beep L, we don't have a speaker
        int16_t beep_duration = params[0].toInt();
        buzzer_beep_duration(beep_duration);
		send_true(response);
    }
}
void ServerComms::process_config_d(String params[], int16_t params_count, struct Command cmd, struct Response_package response){
    if (cmd.cmd_i_value == 0xD0) // Register
	{
		log_d("Server Comms Process D [REGISTER]");
		if (user_handler_register(params, params_count))
			send_true(response);
		else
			send_false(response);
	}
	else if (cmd.cmd_i_value == 0xD1) // Login
	{
		log_d("Server Comms Process D [LOGIN]");
		int32_t token = user_handler_login(params, params_count);
		if (token)
			send_response(&token, response);
		else
			send_false(response);
	}
	else if (cmd.cmd_i_value == 0xD2) // De-register
	{
		log_d("Server Comms Process D [DE-REGISTER]");
		if (user_handler_deregister(params, params_count))
			send_true(response);
		else
			send_false(response);
	}
	else if (cmd.cmd_i_value == 0xD3){ // List all uuids{
		log_d("Server Comms Process D [LIST UUID]");
		String msg = user_handler_get_all_uuid();
		send_response(msg, response);
	}
}

void ServerComms::process_config_e(String params[], int16_t params_count, struct Command cmd, struct Response_package response){
    if (cmd.cmd_i_value == 0xE0) // Total usage all current
	{
		char *char_arr = (char*)calloc(120, 1);
		uint16_t data_len = _relays->build_config_response_data(char_arr);
		send_response(char_arr, data_len, response);
		free(char_arr);
	}
	else if (cmd.cmd_i_value == 0xE1) // Total usage all day
	{  
		char *char_arr = (char*)calloc(120, 1);
		uint16_t data_len = _relays->build_config_response_data(char_arr);
		send_response(char_arr, data_len, response);
		free(char_arr);
	}
	else if (cmd.cmd_i_value == 0xE2) // Total Usage All reset
	{
        _relays->reset_all_usage();
		send_true(response);
	}
	else if (cmd.cmd_i_value == 0xE3 || cmd.cmd_i_value == 0xE4) // Total Watt usage current
	{
		double cur;
		_relays->current_usage_all_day(&cur);
		send_response((int32_t*)&cur, response);
	}
	else if (cmd.cmd_i_value == 0xE4) // Total Watt usage day
	{
		double cur;
		_relays->current_usage_all_day(&cur);
		send_response((int32_t*)&cur, response);
	}
	else if (cmd.cmd_i_value == 0xE5) // Total Watt usage reset
	{
		_relays->reset_current_usage();
		send_true(response);
	}
	else if (cmd.cmd_i_value == 0xE6) // Total click usage A reset
	{
		_relays->reset_relay_clicks(SWITCH_A);
		send_true(response);
	}
	else if (cmd.cmd_i_value == 0xE7) // Total click usage B reset
	{
		_relays->reset_relay_clicks(SWITCH_B);
		send_true(response);	
	}
	else if (cmd.cmd_i_value == 0xE8) // Total click usage C reset
	{
		_relays->reset_relay_clicks(SWITCH_C);
		send_true(response);
	}
	else if (cmd.cmd_i_value == 0xE9) // Total click usage D reset
	{
		_relays->reset_relay_clicks(SWITCH_D);
		send_true(response);
	}
	else if (cmd.cmd_i_value == 0xEA) // Total click usage B Day
	{
		//Unused
	}
	else if (cmd.cmd_i_value == 0xEB) // Total click usage B reset
	{
		//Unused
	}
	else if (cmd.cmd_i_value == 0xEC || cmd.cmd_i_value == 0xED || cmd.cmd_i_value == 0xEE) // Process file usage
	{
		process_file_config(params, params_count, cmd, response);
	}
}

void ServerComms::process_file_config(String params[], int16_t params_count, struct Command cmd, struct Response_package response){
   if (cmd.cmd_i_value == 0xEC) // File usage list
	{
		String ret = "{\"f\":[\"current.txt\"]}";
		send_response(ret, response);
	}
	else if (cmd.cmd_i_value == 0xED) // Get file
	{
		files_manager_read_current_log(response);
	}
	else if (cmd.cmd_i_value == 0xEE) // Delete file
	{
		files_manager_clear_current_log();
		send_true(response);
	}
}

void ServerComms::process_bulk_commands(String params[], int16_t params_count, struct Command cmd, struct Response_package response){
	/**
	 * @brief Handles switch commands which are applied to all switches.
	 * 
	 */
	log_d("Bulk Command Recieved: %02X", cmd.cmd_i_value);
	if(cmd.cmd_i_value == 0xF0){
		_relays->all_off();
		send_switch_status(response);
	}else if (cmd.cmd_i_value == 0xF1){
		_relays->all_on();
		send_switch_status(response);
	}else if (cmd.cmd_i_value == 0xF2){
		if ((params_count % 2) == 0 && params_count > 0 && params[0] == "brightness"){
			uint8_t brightness = params[1].toInt();
			_relays->set_all_brightness(brightness);
			send_switch_status(response);
		}else{
			send_invalid(response);
		}
	}else if (cmd.cmd_i_value == 0xF3){
		_relays->all_lock();
		send_switch_status(response);
	}else if (cmd.cmd_i_value == 0xF4){
		_relays->all_unlock();
		send_switch_status(response);
	}
	program_status_change_flag = 1;
    //files_manager_write_program_status();	// Write Program Status After Special Command.
}

void ServerComms::process_config(String params[], int16_t params_count, struct Command cmd, struct Response_package response){
    uint8_t cmd_first_hex = cmd.cmd_i_value & 0xF0;
	if (cmd_first_hex == 0xC0)
	{
		process_config_c(params, params_count, cmd, response);
	}
	else if (cmd_first_hex == 0xD0)
	{
		process_config_d(params, params_count, cmd, response);
	}
	else if (cmd_first_hex == 0xE0)
	{
		process_config_e(params, params_count, cmd, response);
	}
	else if (cmd_first_hex == 0xF0){
		process_bulk_commands(params, params_count, cmd, response);
	}
}

void ServerComms::copy_command_int(struct Command* c, uint8_t i_val){
    c->cmd = (i_val & 0b11000000) >> 6;
	c->Switch = (i_val & 0b00110000) >> 4;
	c->lock_status = (i_val & 0b00001100) >> 2;
	c->status = (i_val & 0b00000011);
	c->cmd_i_value = i_val;
    log_d("%02X | Cmd: %d| Switch: %d | lock_status: %d| status: %d", i_val, c->cmd, c->Switch, c->lock_status, c->status);
}

uint8_t ServerComms::get_number(uint8_t n){
	if (n >= 48 && n <= 57) // 0 - 9
	{
		n = n - 48;
		return n;
	}
	else if (n >= 65 && n <= 70) // A - F
	{
		n = n - 55;
		return n;
	}
	else if (n >= 97 && n <= 102) // a - f
	{
		n = n - 87;
		return n;
	}
}

uint8_t ServerComms::parse_one_byte(uint8_t* c, int16_t* i){
    uint8_t ret = (get_number(c[*i]) << 4)  | get_number(c[*i + 1]);
	*i += 2;
	return ret;
}

uint16_t ServerComms::parse_two_bytes(uint8_t* c, int16_t* i){
    uint16_t ret = (get_number(c[*i]) << 12)     | (get_number(c[*i + 1]) << 8) | 
				   (get_number(c[*i + 2]) << 4)  | get_number(c[*i + 3]);
	*i += 4;
	return ret;
}

uint32_t ServerComms::parse_four_bytes(uint8_t* c, int16_t* i){

    uint32_t ret = (get_number(c[*i]) << 28)	    | (get_number(c[*i + 1]) << 24) | 
				   (get_number(c[*i + 2]) << 20) | (get_number(c[*i + 3]) << 16) | 
				   (get_number(c[*i + 4]) << 12) | (get_number(c[*i + 5]) << 8)  | 
				   (get_number(c[*i + 6]) << 4)  | get_number(c[*i + 7]);
	*i += 8;
	return ret;
}


int8_t ServerComms::parse_weekdays(String weekdays){
    if (weekdays.length() != 7){
		return -1;
	}
	
	int8_t ret = 0;
	for (int i = 0; i < 7; i++){
		if (weekdays[i] == 'Y'){
			ret = ret | (1 << (6 - i));
			//ret = ret | (1 << (i));
		}
	}
	
	return ret;
}

String ServerComms::read_till_comma(uint8_t* c, int16_t* i, uint16_t count){
    String ret_str = "";
	while((char)c[*i] != ',' && *i < count){
		ret_str += (char)c[*i];
		*i += 1;
	}
	*i += 1;
	return ret_str;
}

String ServerComms::read_till_colon(uint8_t* c, int16_t* i, uint16_t count){
    String ret_str = "";
	while((char)c[*i] != ':' && *i < count){
		ret_str += (char)c[*i];
		*i += 1;
	}
	
	*i += 1;
	return ret_str;
}

String ServerComms::read_till_end(uint8_t* c, int16_t* i, uint16_t count){
	String ret_str = "";
	while(c[*i] != 0 && *i < count)
	{
		ret_str += (char)c[*i];
		*i += 1;
	}
	*i += 1;
	return ret_str;
}

uint32_t ServerComms::get_till_comma(uint8_t* c, int16_t* i, int8_t* num_bytes){
    *num_bytes = 0;
	for (int j = 0; j < 20; j++)
	{
		if (c[*i + j] == ',' || c[*i + j] == 0)
		{
			break;
		}
		
		*num_bytes += 1;
	}
	*num_bytes = *num_bytes / 2;
	
	if (*num_bytes == 4)
	{
		uint32_t ret = parse_four_bytes(c, i);
		return ret;
	}
	else if (*num_bytes == 2)
	{
		return parse_two_bytes(c, i);
	}
	else if (*num_bytes == 1)
	{
		return parse_one_byte(c, i);
	}
}

void ServerComms::send_response(char* str, uint16_t data_len, struct Response_package response){
    uint16_t devno_str_len = SYSTEM_CONFIGURATION.deviceNo.length() + 2;
	char* devno_char_array = (char*)calloc(devno_str_len, 1);
	SYSTEM_CONFIGURATION.deviceNo.toCharArray(devno_char_array, devno_str_len);
	
	char* str_device_no;
	uint16_t str_len;
	char* req_id_char_array;
	
	if (!wifi_ap_connected)
	{
		uint16_t req_id_str_len = response.req_id.length() + 2;
		req_id_char_array = (char*)calloc(req_id_str_len, 1);
		response.req_id.toCharArray(req_id_char_array, req_id_str_len);
		
		str_device_no = (char*)calloc(data_len + devno_str_len + req_id_str_len + 12, 1);
		str_len = sprintf(str_device_no, "%s,%s,%02X,%s", req_id_char_array, devno_char_array, response.cmd_i_val, str);
	}
	else
	{
		str_device_no = (char*)calloc(data_len + devno_str_len + 12, 1);
		str_len = sprintf(str_device_no, "%s,%02X,%s", devno_char_array, response.cmd_i_val, str);
		req_id_char_array = NULL; // Here to avoid Uninitalized Error for "free(req_id_char_array)"
	}

	if(response.resp_type == SCR_MQTT)
	{
		#ifdef DEVICENO
		mqtt_publish_msg(str_device_no, str_len);
		#else
		mqtt_publish_msg(str, str_len);
		#endif		
	}
	else if (response.resp_type == SCR_HTTP)
	{
		#ifdef DEVICENO
		webserver_send_jsresponse(str_device_no);
		#else
		webserver_send_jsresponse(str);
		#endif	
	}
	
	free(devno_char_array);
	free(str_device_no);
	
	if (!wifi_ap_connected)
	{
		free(req_id_char_array);
	}
}

void ServerComms::send_response(String str, struct Response_package response){
    uint16_t str_len = str.length() + 1;
    char* char_array = (char*)calloc(str_len + 2, 1);
	str.toCharArray(char_array, str_len);
	send_response(char_array, str_len, response);
	
	free(char_array);
}

void ServerComms::send_response(int32_t* val, struct Response_package response){
    char* str = (char*)calloc(20, 1);
	uint16_t data_len = sprintf(str, "%08X", *val);
	send_response(str, data_len, response);
	
	free(str);
	return;
}

void ServerComms::send_true(struct Response_package response){
	char* str = (char*)calloc(6, 1);
	uint16_t data_len = sprintf(str, "TRUE");
	send_response(str, data_len, response);
	free(str);
}

void ServerComms::send_false(struct Response_package response){
	char* str = (char*)calloc(6, 1);
	uint16_t data_len = sprintf(str, "FALSE");
	send_response(str, data_len, response);
	free(str);
}

void ServerComms::send_invalid(struct Response_package response){
	char* str = (char*)calloc(17, 1);
	uint16_t data_len = sprintf(str, "Invalid Request");
	send_response(str, data_len, response);
	free(str);
}

void ServerComms::parse_json(String parameters[], uint8_t* c, int16_t count, int16_t* params_count, struct Response_package response){
    uint8_t dq_counter = 0;
	for (int i = 0; i < count; i++)
	{
		if (c[i] == '\"')
		{
			dq_counter++;
		}
	}
	
	uint8_t str_arr_len = dq_counter / 2;
	if (str_arr_len > 50) // We only handle 50 parameters
	{
		send_false(response);
	}
	
	uint8_t arr_i = 0;
	uint8_t loop_state = 0; // Searching for opening quotes
	for (int i = 0; i < count; i++)
	{
		if (c[i] == '\"')
		{
			if (loop_state == 1)
			{
				arr_i++;
				if (arr_i == str_arr_len)
					break;
				
				loop_state = 0; // Searching for opening quotes
				continue;
			}
			else
			{
				loop_state = 1; // Start logging parameter
				continue;
			}
		}
		
		if (loop_state == 1)
		{
			parameters[arr_i] += (char)c[i];
		}
	}
	
	*params_count = str_arr_len;
	return;
}

String ServerComms::weekdays_string(int8_t weekdays){
    String ret = "";
	for (int i = 6; i >= 0; i--){
		if (weekdays & (1 << i)){
			ret += "Y";
		}else{
			ret += "N";
		}
	}
	return ret;
}

void ServerComms::factory(uint8_t* msg, int16_t count, int8_t resp_type){
    if (count < MIN_BYTES_LENGTH_GENERAL){
		return;
	}
	
	int16_t i = 0;
	struct Response_package response;
	response.resp_type = resp_type;
	int8_t num_bytes = 0;
	uint8_t cmd_i_val;
	// DEVICE NO ENABLE HERE, UNCOMMENT BELOW LINES
	#ifdef DEVICENO
	if (!wifi_ap_connected)
	{
		response.req_id = read_till_comma(msg, &i, count);
		String device_no = read_till_comma(msg, &i, count);
		if (device_no != SYSTEM_CONFIGURATION.deviceNo){
			log_d("Device Number Expected: %s | Device ID Provided: %s", SYSTEM_CONFIGURATION.deviceNo.c_str(), device_no.c_str());
			return;
		}
	}
	#endif
	
	uint32_t ret = get_till_comma(msg, &i, &num_bytes);
	i++; // Comma
	if (num_bytes == 4)
	{
		// We got a token, do we need it?
		if (!wifi_ap_connected)
		{
			uint32_t token = ret;
			if(!user_handler_validate_token(token))
			{
				String resp = "Invalid token";
				response.cmd_i_val = 0xFF;
				send_custom(resp, response);
				return;
			}
			
			response.cmd_i_val = cmd_i_val = parse_one_byte(msg, &i);
			i++; // comma
		}
		else
		{
			// We don't need token, wrong command
			response.cmd_i_val = 0xFF;
			send_invalid(response);
			return;
		}
	}
	else if (num_bytes == 1)
	{
		response.cmd_i_val = cmd_i_val = ret;
		// This allows only D0 and D1 while connected to Wifi without a token
		if (!wifi_started_hotspot && cmd_i_val != 0xD0 && cmd_i_val != 0xD1)
		{
			String resp = "Invalid token";
			send_custom(resp, response);
			return;
		}
	}
	else
	{
		response.cmd_i_val = 0xFF;
		send_invalid(response);
		return;
	}
	
	
	struct Command c;
	copy_command_int(&c, cmd_i_val);
	
	if (c.cmd == CMD_TIMER || c.cmd == CMD_SCHEDULER)
	{
		if (cmd_i_val == 0x43)
		{
			send_timer_status(response, CMD_TIMER);
			return;
		}
		else if (cmd_i_val == 0x83)
		{
			send_timer_status(response, CMD_SCHEDULER);
			return;
		}
		
		String timer_id = read_till_colon(msg, &i, count);
		String epoch = read_till_colon(msg, &i, count);
		String weekdays = read_till_colon(msg, &i, count);
		String repeat_count = read_till_colon(msg, &i, count);
		String duration_repeat = read_till_colon(msg, &i, count);
		String switch_id = read_till_colon(msg, &i, count);
		String status = read_till_colon(msg, &i, count);
		
		#ifdef SERIAL_PRINT
		Serial.print("timer ID: "); Serial.print(timer_id);
		Serial.print("\tEpoch: "); Serial.print(epoch);
		Serial.print("\tRepeat count: "); Serial.print(repeat_count);
		Serial.print("\tduration_repeat: "); Serial.print(duration_repeat);
		Serial.print("\tSwitch_ID: "); Serial.print(switch_id);
		Serial.print("\tstatus: "); Serial.println(status);
		#endif
		
		struct relay_timer temp_timer;
		temp_timer.timer_id = timer_id.toInt();
		temp_timer.epoch_time = epoch.toInt();
		temp_timer.repeat_count = repeat_count.toInt();
		temp_timer.duration_repeat = duration_repeat.toInt();
		temp_timer.relay_switch = switch_id.toInt();
		temp_timer.timer_status = status.toInt();
		temp_timer.weekdays = parse_weekdays(weekdays);
		if (temp_timer.weekdays == -1)
		{
			String ret = "Invalid command parameters";
			send_custom(ret, response);
		}
		
		if (c.cmd == CMD_TIMER)
		{
			String timer_duration_seconds = read_till_colon(msg, &i, count);
			temp_timer.timer_duration = timer_duration_seconds.toInt();
			process_timer(c.cmd_i_value, &temp_timer, response);
		}
		else // cmd_scheduler
		{
			temp_timer.timer_duration = 0;
			process_schedule(cmd_i_val, &temp_timer, response);
		}
	}
	else if (c.cmd == CMD_SWITCH)
	{
		String brightness_str = read_till_comma(msg, &i, count);
		log_d("Switch Brightness: %s", brightness_str.c_str());
		if(brightness_str == ""){
			process_switch(c, response);
		}else{
			int brightness = brightness_str.toInt();
			process_switch(c, brightness, response);
		}
	}
	else if (c.cmd == CMD_CONFIG)
	{
		int16_t params_count = 0;
		String parameters[50];
		parse_json(parameters, (msg + i), (count - i), &params_count, response);
		log_d("------------- Factory Parameters -------------");
		for (int i = 0; i < params_count; i+=2)
		{
			log_d("%s: %s", parameters[i].c_str(), parameters[i+1].c_str());
		}
		log_d("------------- END -------------");
		
		process_config(parameters, params_count, c, response); 
	}
}

void ServerComms::factory_config(uint8_t* msg, int16_t count, int8_t resp_type){
	// TODO: Remove duplicate code that this and factory share..

    if (count < MIN_BYTES_LENGTH_GENERAL){
		return;
	}
	
	int16_t i = 0;
	struct Response_package response;
	response.resp_type = resp_type;
	int8_t num_bytes = 0;
	uint8_t cmd_i_val;
	// DEVICE NO ENABLE HERE, UNCOMMENT BELOW LINES
	if (!wifi_ap_connected)
	{
		response.req_id = "infyserver";
		String mac_addr = read_till_comma(msg, &i, count);
		if (mac_addr.length() < 17)
			return;	
		String wifi_mac_addr = WiFi.macAddress();
		for (int i = 0; i < 17; i++){
			if (mac_addr[i] != wifi_mac_addr[i])
			{
				#ifdef SERIAL_PRINT
				Serial.printf("i: %d", i);
				Serial.print("\tchar mac: ");
				Serial.print(mac_addr[i]);
				Serial.print("\twifi mac char: ");
				Serial.println(wifi_mac_addr[i]);
				#endif
				return;
			}
		}
		
	}
	
	uint32_t ret = get_till_comma(msg, &i, &num_bytes);
	i++; //Comma
	if (num_bytes != 1)
	{
		response.cmd_i_val = 0xFF;
		send_invalid(response);
		return;
	}
	
	response.cmd_i_val = cmd_i_val = ret;
	struct Command c;
	copy_command_int(&c, cmd_i_val);
	
	if (c.cmd == CMD_TIMER || c.cmd == CMD_SCHEDULER)
	{
		if (cmd_i_val == 0x43)
		{
			send_timer_status(response, CMD_TIMER);
			return;
		}
		else if (cmd_i_val == 0x83)
		{
			send_timer_status(response, CMD_SCHEDULER);
			return;
		}
		
		String timer_id = read_till_colon(msg, &i, count);
		String epoch = read_till_colon(msg, &i, count);
		String weekdays = read_till_colon(msg, &i, count);
		String repeat_count = read_till_colon(msg, &i, count);
		String duration_repeat = read_till_colon(msg, &i, count);
		String switch_id = read_till_colon(msg, &i, count);
		String status = read_till_colon(msg, &i, count);
		
		struct relay_timer temp_timer;
		temp_timer.timer_id = timer_id.toInt();
		temp_timer.epoch_time = epoch.toInt();
		temp_timer.repeat_count = repeat_count.toInt();
		temp_timer.duration_repeat = duration_repeat.toInt();
		temp_timer.relay_switch = switch_id.toInt();
		temp_timer.timer_status = status.toInt();
		temp_timer.weekdays = parse_weekdays(weekdays);
		if (temp_timer.weekdays == -1)
		{
			String ret = "Invalid command parameters";
			send_custom(ret, response);
		}
		
		if (c.cmd == CMD_TIMER)
		{
			String timer_duration_seconds = read_till_colon(msg, &i, count);
			temp_timer.timer_duration = timer_duration_seconds.toInt();
			process_timer(c.cmd_i_value, &temp_timer, response);
		}
		else // cmd_scheduler
		{
			temp_timer.timer_duration = 0;
			process_schedule(cmd_i_val, &temp_timer, response);
		}
	}
	else if (c.cmd == CMD_SWITCH)
	{

		String brightness_str = read_till_comma(msg, &i, count);
		if(brightness_str == ""){
			brightness_str = "100";
		}
		int brightness = brightness_str.toInt();
		process_switch(c, brightness, response);
	}
	else if (c.cmd == CMD_CONFIG)
	{
		int16_t params_count = 0;
		String parameters[50];
		parse_json(parameters, (msg + i), (count - i), &params_count, response);
		#ifdef SERIAL_PRINT
		Serial.print("Factory Parameters are: ");
		for (int i = 0; i < params_count; i++)
		{
			Serial.print(parameters[i]);
			Serial.print("\t");
		}
		Serial.println();
		#endif
		
		process_config(parameters, params_count, c, response);
	}
}