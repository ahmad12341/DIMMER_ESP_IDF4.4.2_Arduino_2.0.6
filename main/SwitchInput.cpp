#include "SwitchInput.h"


void Switches::init(Relays* relays){
	/**
	 * @brief Initialise Switches - Setup Pins and Set Switch Type.
	 * 
	 */
	_relays = relays;
	log_d("Relay Switch Mode: %s", SYSTEM_CONFIGURATION.switchType);
	for(int i=0; i< NUM_CHANNELS; i++){
		_switch_arr[i]->pin = _switch_pins[i];
		_switch_arr[i]->relay = _relays->get_relay(i);
		pinMode(_switch_arr[i]->pin, INPUT); // This should have external pullup on device.
		// if (SYSTEM_CONFIGURATION.switchType == "TOGGLE"){
		// 	init_state(_switch_arr[i]->flag, _switch_arr[i]->relay);
		// }
	};
}

// void Switches::init_state(uint8_t &switch_flag, struct relay* r){
// 	switch_flag = (r->status == 1) ? 1 : 0;
// }

void Switches::loop(){
	/**
	* @brief Main Switch Loop - Checks if button presses have triggered reset then handles either push or toggle modes and watchdog feed.
	*/
	reset_handler();
	if (SYSTEM_CONFIGURATION.switchType == "TOGGLE"){
		switch_loop();
	}else{
		button_loop();
	}
	// watchdog_feed();
}

void Switches::button_loop()
{
	/**
	* @brief Switch Toggle Loop - Each Press of the Button will toggle the relay's state. 
	*/
	for(int i=0; i< NUM_CHANNELS; i++){
		button_handler(_switch_arr[i]);
	}
}

void Switches::switch_loop()
{
	/**
	* @brief Switch Push Loop - The relay will be ON while the button HIGH. 
	*/
	for(int i=0; i< NUM_CHANNELS; i++){
		switch_handler(_switch_arr[i]);
	}
}
void Switches::reset_handler(){
	/**
	* @brief Checks if any switch has been pressed +7 times in less than 10 sec.
	*/
	for(int i=0; i< NUM_CHANNELS; i++){
		if(reset_triggered(_switch_arr[i]->counter,_switch_arr[i]->start_time)){
			log_d("Reseting Device...");
			files_manager_reset_config();
			buzzer_beep_duration(1000);
			esp_restart();
		}
	};
}


bool Switches::reset_triggered(uint16_t switch_counter, uint32_t switch_start_time){
	/**
	* @brief Switch Reset Checker - Checks if the button has been pressed 7 times in the last 10 seconds
	*/
	return (switch_counter >= 7 && ((millis() - switch_start_time) <= 10000)) ? true: false;
}

uint8_t Switches::poll_switch(int8_t pin, uint8_t loops, uint8_t polling_delay_ms, uint8_t expected_state){
	/**
	 * @brief Polls the switch multiple times checking its state - This is to filter out noise and ensure the press is actually detected.
	*/
	for(uint8_t i=0; i<loops; i++){
		if(digitalRead(pin) != expected_state){
			return 0;
		}
		delay(polling_delay_ms);
	}
	return 1; // Switch was expected state the whole time.
}

void Switches::switch_handler(struct switch_t* sw){
	/**
	* @brief Switch Handler - Turns on relay while button is HIGH and counts the number of presses. 
	*/
	// If switch is low and switch flag has not been set - turn ON
	if(!digitalRead(sw->pin) && !sw->flag){
		// Switch Toggled 
		uint8_t noise_check_res = poll_switch(sw->pin, SWITCH_POLLING_COUNT, SWITCH_POLLING_TIME_MS, 0);
		if(!noise_check_res){
			log_d("Noise Check Failed!");
			return; // Noise check failed - dont count button press.
		}
		if(sw->counter == 0){
			sw->start_time = millis();
		}else if ((millis() - sw->start_time) > 10000){
			sw->counter = 0;
			sw->start_time = millis();
		}
		sw->counter++;
		log_d("Switch Counter: %u", sw->counter);
		sw->flag = 1;
		if (!sw->relay->lock){
			_relays->set_state( sw->relay, STATUS_ON);
			log_d("Switch for relay %c Turned ON", sw->relay->name);
		}
		delay(100);
	}else if(digitalRead(sw->pin) && sw->flag){
		// Switch Toggled - Back to HIGH
		uint8_t noise_check_res = poll_switch(sw->pin, SWITCH_POLLING_COUNT, SWITCH_POLLING_TIME_MS, 1);
		if(!noise_check_res){
			log_d("Noise Check Failed!");
			return; // Noise check failed - dont count button press.
		}
		sw->flag = 0;
		if (! sw->relay->lock){
			_relays->set_state(sw->relay, STATUS_OFF);
			log_d("Switch for relay %c Turned OFF",sw->relay->name);
		}
		delay(100);
	}
}

void Switches::button_handler(struct switch_t* sw){
	/**
	* @brief Switch Toggle Handler - Toggles the relay state each time the button is pressed. 
	*/
	if(!digitalRead(sw->pin) && !sw->flag){
		// Button Pressed
		uint8_t noise_check_res = poll_switch(sw->pin, SWITCH_POLLING_COUNT, SWITCH_POLLING_TIME_MS, 0);
		if(!noise_check_res){
			return; // Noise check failed - dont count button press.
		}
		log_d("Switch for relay %c Pressed!", sw->relay->name);
		if(sw->counter == 0){
			sw->start_time = millis();
		}else if ((millis() - sw->start_time) > 10000){
			sw->counter = 0;
			sw->start_time = millis();
		}
		sw->flag = 1;
		delay(100);
	}else if(digitalRead(sw->pin) && sw->flag){
		// Button Released
		uint8_t noise_check_res = poll_switch(sw->pin, SWITCH_POLLING_COUNT, SWITCH_POLLING_TIME_MS, 1);
		if(!noise_check_res){
			return; // Noise check failed - dont count button press.
		}
		log_d("Switch for relay %c Released!", sw->relay->name);
		sw->flag = 0;
		sw->counter++;
		if (!sw->relay->lock){
			_relays->set_state(sw->relay, STATUS_TOGGLE);
			log_d("Switch for relay %c TOGGLED", sw->relay->name);
		}
		delay(100);
	}
}