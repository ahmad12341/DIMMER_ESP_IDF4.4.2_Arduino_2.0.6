#include "main.h"

// // Built in ESP32 temperature Sensor.
// #ifdef __cplusplus

// extern "C" {

// #endif

// uint8_t temprature_sens_read();

// #ifdef __cplusplus

// }

// #endif

// uint8_t temprature_sens_read();

SHTC3 mySHTC3;              // Declare an instance of the SHTC3 class

uint32_t last_temp_read = millis();

void errorDecoder(SHTC3_Status_TypeDef message)                             // The errorDecoder function prints "SHTC3_Status_TypeDef" resultsin a human-friendly way
{
  switch(message)
  {
    case SHTC3_Status_Nominal : log_d("Nominal"); break;
    case SHTC3_Status_Error : log_d("Error"); break;
    case SHTC3_Status_CRC_Fail : log_d("CRC Fail"); break;
    default : log_d("Unknown return code"); break;
  }
}

String system_config_get_all_config();
void system_config_check_config_request();
void serial_loop();

Switches switches;
Relays relays;
Scheduler scheduler;
ServerComms server_comms;
Dimmer dimmer_handler;

#define ENABLE_UDP_DEBUG_LOG 1

unsigned long last_task_print = 0;
char ptrTaskList[1024];

unsigned long last_shtc_check_ms = millis();
unsigned long last_file_update_ms = millis();

void setup()
{
	Serial.begin(115200);
	Serial.println("Starting..");

	// Set General Log Level to debug - other tags can still be disabled.
	esp_log_level_set("*", ESP_LOG_DEBUG); 

	Serial.print("Firmware Version:");
	Serial.println(FIRMWARE_VERSION_NUMBER);

	Wire.begin();
	mySHTC3.begin();

	files_manager_init();

	user_handler_init();
	wifi_setup();
	
	relays.init();
	switches.init(&relays);
	server_comms.init(&relays);
	scheduler.init(&relays, &server_comms);


	buzzer_init();

	current_sensor_init();

	mqtt_init();

	// watchdog_init();

	// Assign Dimmer task to other core.
	Serial.print("Main Task Core: ");
	Serial.println(xPortGetCoreID());
	TaskHandle_t DimmerTask;
	xTaskCreatePinnedToCore(dimmer_task,"DimmerTask", 10000, NULL, 10, &DimmerTask, !xPortGetCoreID());     

}

void dimmer_task(void * pvParameters ){
	/**
	 * @brief Task that initalises the dimmer - this is required to avoid the interrupts from blocking other execution and triggering watchdog.
	 * 
	 * NOTE: Interrupts are handled on the core where they are assigned.
	 */
	dimmer_handler.init();
	for(;;){
		dimmer_handler.loop(); // Update brightness calculation every 10ms
		delay(5); 
	}
}

void udp_log_loop(){
	/**
	 * @brief Checks conditions for UDP logger to start and starts it if these conditions are met.
	 * 
	 */
	if(!UDPLogger::is_alive()){
		if((wifi_program_status == WIFI_CONNECTED || wifi_program_status == WIFI_STATUS_AP)){
			UDPLogger::init(); // Attached udp broadcast to logger on port 8888
		}
	}
}

void loop(){
	/**
	 * @brief Core program loop.
	 * 
	 */
	// if(millis() > last_task_print + 10000){
	// 	log_d("Largest Start Delay %llu", largest_delay_start);
	// 	log_d("Largest End Delay %llu", largest_delay_end);
	// 	log_d("Zero Cross Delay %llu", largest_zero_cross_delay);

	// 	largest_delay_start=0;
	// 	largest_delay_end=0;
	// 	largest_zero_cross_delay = 0;
	// 	// vTaskList(ptrTaskList);
	// 	// log_d("----------------- TASK LIST -----------------");
	// 	// log_d("Task  	State   Prio    Stack    Num   Core");
	// 	// log_d("--------------------------------------------");
	// 	// log_d("\r\n %s", ptrTaskList);
	// 	// log_d("--------------------------------------------");
	// 	last_task_print = millis();
	// }
	if(millis() > last_temp_read + 5000){
		// log_d("Temperature Read");
		SHTC3_Status_TypeDef result = mySHTC3.update();             // Call "update()" to command a measurement, wait for measurement to complete, and update the RH and T members of the object
		// errorDecoder(result);

		last_temp_read = millis();
		if(result == SHTC3_Status_Nominal){
			temperature_c = mySHTC3.toDegC();
			log_d("Temperature Reading: %d C", temperature_c);
		}
	}

	// Serial.print(temprature_sens_read() );

	// Serial.print(" F");

	// Serial.print("______");

	// // Convert raw temperature in F to Celsius degrees

	// Serial.print((temprature_sens_read() - 32) / 1.8);

	// Serial.println(" C");

	if(program_status_change_flag && (millis() > last_file_update_ms + 60000)){ // Update file every 60 sec if changes have been made.
		/**
		 * @brief Checks if any changes were made to the program status - if so tries to find a good time to write to file as to avoid missing interrupts.
		 * 
		 */

		log_d("Updating the program status");
		files_manager_write_program_status();
		last_file_update_ms = millis();
		program_status_change_flag = 0; // Reset flag
	}
	// for(;;){
	// 	delay(1000);
	// }
	#ifdef ENABLE_UDP_DEBUG_LOG
	if(wifi_connected_flag){
		udp_log_loop(); // The reconnect attempts cause socket to hang and raise exception.
	}
	#endif

	webserver_loop();
	delay(1);
	relays.loop();
	delay(1);
	broadcast_loop();
	delay(1);
	switches.loop();
	delay(1);
	buzzer_loop();
	delay(1);
	current_sensor_loop();
	delay(1);
	wifi_loop();
	delay(1);
	mqtt_loop();
	delay(1);
	webserver_loop();
	delay(1);
	user_handler_loop();
	delay(1);
	rtc_loop();
	delay(1);
	scheduler.loop();
	delay(1);
	
}

void serial_loop()
{
	if (Serial.available())
	{
		char input_ch = Serial.read();
		if (input_ch == 'n' || input_ch == 'N')
		{
		}
		else if (input_ch == 'f' || input_ch == 'F')
		{
		}
	}
}
