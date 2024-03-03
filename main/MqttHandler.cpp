#include "MqttHandler.h"

extern ServerComms server_comms;
extern Relays relays;

void mqtt_init()
{
	if (!mqtt_server_set)
		mqtt_set_server();
}

void mqtt_set_server()
{
	if (mqtt_server_set && SYSTEM_CONFIGURATION.mqttHost != "" && SYSTEM_CONFIGURATION.mqttPort != "")
	{
		free(mqtt_server_domain);
		free(mqtt_broker_user);
		free(mqtt_broker_pass);
		free(mqtt_broker_id);
		free(mqtt_subscribe_topic);
		free(mqtt_publish_topic);
		free(mqtt_config_topic);
		if (mqtt_client.connected())
		{
			mqtt_client.disconnect();
		}
		
		mqtt_server_set = 0;
	}
	
	if (!mqtt_server_set && SYSTEM_CONFIGURATION.mqttHost != "" && SYSTEM_CONFIGURATION.mqttPort != "")
	{
		int str_len = SYSTEM_CONFIGURATION.mqttHost.length() + 1;
		mqtt_server_domain = (char*)calloc(str_len, 1);
		mqtt_port = SYSTEM_CONFIGURATION.mqttPort.toInt();
		SYSTEM_CONFIGURATION.mqttHost.toCharArray(mqtt_server_domain, str_len);
		mqtt_client.setServer(mqtt_server_domain, mqtt_port);
		mqtt_server_set = 1;
		
		str_len = SYSTEM_CONFIGURATION.mqttUser.length() + 1;
		mqtt_broker_user = (char*)calloc(str_len, 1);
		SYSTEM_CONFIGURATION.mqttUser.toCharArray(mqtt_broker_user, str_len);
		
		str_len = SYSTEM_CONFIGURATION.deviceNo.length() + 1;
		mqtt_broker_id = (char*)calloc(str_len, 1);
		SYSTEM_CONFIGURATION.deviceNo.toCharArray(mqtt_broker_id, str_len);
		
		str_len = SYSTEM_CONFIGURATION.mqttPWD.length() + 1;
		mqtt_broker_pass = (char*)calloc(str_len, 1);
		SYSTEM_CONFIGURATION.mqttPWD.toCharArray(mqtt_broker_pass, str_len);
		
		str_len = SYSTEM_CONFIGURATION.mqttTopicSub.length() + 1;
		mqtt_subscribe_topic = (char*)calloc(str_len, 1);
		SYSTEM_CONFIGURATION.mqttTopicSub.toCharArray(mqtt_subscribe_topic, str_len);
		
		str_len = SYSTEM_CONFIGURATION.mqttTopicPub.length() + 1;
		mqtt_publish_topic = (char*)calloc(str_len, 1);
		SYSTEM_CONFIGURATION.mqttTopicPub.toCharArray(mqtt_publish_topic, str_len);
		
		str_len = SYSTEM_CONFIGURATION.mqttTopicConfig.length() + 1;
		mqtt_config_topic = (char*)calloc(str_len, 1);
		SYSTEM_CONFIGURATION.mqttTopicConfig.toCharArray(mqtt_config_topic, str_len);
		
		mqtt_time = 0;
	}
}

void mqtt_loop()
{
	static uint32_t config_send_timer = 0;
	static uint8_t mqtt_connection_print_flag = 1;
	int32_t time_diff = rtc_millis() - mqtt_time;
	
	mqtt_client_connected = mqtt_client.connected();
	if (!mqtt_client_connected && !mqtt_connection_print_flag)
	{
		log_d("MQTT Connection Lost rc=%i", mqtt_client.state());
		mqtt_connection_print_flag = 1;
	}
	else if (mqtt_client_connected && mqtt_connection_print_flag)
	{
		mqtt_connection_print_flag = 0;
	}
	
	static uint32_t config_request_send_timer = 0;
	if (SYSTEM_CONFIGURATION.config_server_request)
	{
		if ((millis() - config_request_send_timer) > 10000)
		{
			mqtt_send_config_request();
			config_request_send_timer = millis();
		}
	}
	
	if (wifi_connected_flag && time_diff >= MQTT_RECONNECT_TIMER && !mqtt_client_connected)
	{
		//broadcast_send(String("before starting MQTT connection task from Loop"));
		mqtt_connect_task();
		mqtt_time = rtc_millis();
		//return;
	}
	else if (wifi_connected_flag && mqtt_client_connected)
	{
		mqtt_client.loop();
			
		uint32_t timeDiff = millis() - config_send_timer;
		uint16_t interval = 10;
		if (SYSTEM_CONFIGURATION.broadcastIntervalMqtt > 10)
		{
			interval = SYSTEM_CONFIGURATION.broadcastIntervalMqtt;
		}
		if (timeDiff >= (interval * 1000))
		{
			config_send_timer = millis();
			mqtt_send_periodic_status();
		}
	}
}

volatile uint8_t mqtt_task_started_flag = 0;

void mqtt_connect_task()
{
	static uint32_t start_time = 0;
	uint32_t timeDiff = millis() - start_time;
	if (!mqtt_task_started_flag )
	{
		mqtt_task_started_flag  = 1;
		start_time = millis();
		//broadcast_send(String("before starting MQTT connection task"));
		xTaskCreate(mqtt_connect_toserver, "mqtt_connection", 10000, NULL, 1, NULL);
	}
}

void mqtt_send_periodic_status()
{
	log_d("MQTT Periodic Update.");
	char* char_array = (char*)calloc(160, 1);
	String str = "0000,";
	str += SYSTEM_CONFIGURATION.deviceNo + ",infodevice,";
	str += WiFi.macAddress();
	uint16_t length = relays.build_response_data(str, char_array);
	mqtt_publish_msg(char_array, length);
	free(char_array);
}


void mqtt_callback(char* topic, byte* payload, unsigned int length)
{
	payload[length] = 0;
	char msg[length];
	for(int i=0; i< length; i++){
		msg[i] = (char)payload[i];
	}
	log_d("MQTT Callback TOPIC: %s | MSG: %s", topic, msg);

	uint8_t ret_channel = mqtt_msg_channel(topic);
	if (ret_channel == MQTT_CHANNEL_SUB)
	{
		server_comms.factory((uint8_t*)payload, length, SCR_MQTT);
	}
	else
	{
		server_comms.factory_config((uint8_t*)payload, length, SCR_MQTT);
	}
}

uint8_t mqtt_msg_channel(char* topic)
{
	uint8_t channel_found_flag = 1;
	
	for (int i = 0; i < SYSTEM_CONFIGURATION.mqttTopicConfig.length(); i++)
	{
		
		if (topic[i] != SYSTEM_CONFIGURATION.mqttTopicConfig[i])
		{
			channel_found_flag = 0;
			break;
		}
	}
	
	if (channel_found_flag)
	{
		return MQTT_CHANNEL_CONFIG;
	}
	else
	{
		return MQTT_CHANNEL_SUB;
	}
	
	
}

void mqtt_config_callback(char* topic, byte* payload, unsigned int length)
{
	payload[length] = 0;
	server_comms.factory_config((uint8_t*)payload, length, SCR_MQTT);
}

void mqtt_connect_toserver(void* parameter)
{
	wifi_client.flush();
	wifi_client.stop();
	
	static uint8_t mqtt_connection_trial = 0;
	
	if (!mqtt_server_set)
		mqtt_set_server();
	
	static int flag = 0;
	if (SYSTEM_CONFIGURATION.mqttUser == "" || SYSTEM_CONFIGURATION.mqttPWD == "" || SYSTEM_CONFIGURATION.mqttHost == "")
	{
		mqtt_task_started_flag = 0;
		vTaskDelete(NULL);
	}
	
	uint32_t millis_before = millis();

	Serial.println("Before MQTT connection trial");
	if (mqtt_client.connect(mqtt_broker_id, mqtt_broker_user, mqtt_broker_pass))
	{
		wifi_internet_up = 1;
		log_d("MQTT Connected to server.");
		mqtt_client.setCallback(mqtt_callback);
		mqtt_client.subscribe(mqtt_subscribe_topic);
		mqtt_client.subscribe(mqtt_config_topic);
		
		system_config_check_config_request();

		SYSTEM_CONFIGURATION.mqtt_connection_trial = 0;
		if (SYSTEM_CONFIGURATION.wifi_configured)
		{
			mqtt_send_config_request();
			SYSTEM_CONFIGURATION.wifi_configured = 0;
		}
		
		if (SYSTEM_CONFIGURATION.device_configured == 0 && SYSTEM_CONFIGURATION.deviceUser != "default" && SYSTEM_CONFIGURATION.devicePwd != "default")
		{
			// Device configured
			SYSTEM_CONFIGURATION.device_configured = 1;
			buzzer_beep_n_times(5);
			buzzer_reboot = 1;
		}
		
		files_manager_write_config();
		
		if (!flag)
		{
			struct Response_package resp_pack;
			resp_pack.req_id = "0000";
			resp_pack.cmd_i_val = 0x3C;
			resp_pack.resp_type = SCR_MQTT;
			server_comms.send_switch_status(resp_pack);
		}
		
		mqtt_task_started_flag = 0;
		vTaskDelete(NULL);
	}
	else
	{
		log_d("Failed to connect to MQTT server: rc=%i", mqtt_client.state());

		uint32_t time_diff = millis() - millis_before;
		mqtt_connection_trial++;
		if(mqtt_connection_trial > 200)
		{
			mqtt_connection_trial = 0;
		}
	}
	
	mqtt_task_started_flag = 0;
	vTaskDelete(NULL);
}

uint8_t mqtt_publish_msg(char* str, uint16_t data_len)
{
	mqtt_last_time = rtc_get_epoch();
	if (mqtt_client.publish(mqtt_publish_topic, str, data_len))
	{
		log_d("MQTT Publish TOPIC: %s | MSG: %s", mqtt_publish_topic,  (char*)str);
		return 1;
	}
	
	return 0;
}

uint8_t mqtt_publish_msg(String str)
{
	int str_len = str.length() + 1;
	char char_array[str_len];
	str.toCharArray(char_array, str_len);
	mqtt_publish_msg(char_array, str_len);
	return 1;
}

void mqtt_send_config_request()
{
	if (!mqtt_client.connected())
	{
		return;
	}
	
	String msg_to_send = String("0000,") + SYSTEM_CONFIGURATION.serialNo + String(",config,") + WiFi.macAddress() + String(",") + SYSTEM_CONFIGURATION.owner + String (",") + SYSTEM_CONFIGURATION.gangType;
	mqtt_publish_msg(msg_to_send);
}

void mqtt_publish_msg(int32_t* val)
{
	char* str = (char*)calloc(20, 1);
	uint16_t data_len = sprintf(str, "%08X", *val);
	mqtt_publish_msg(str, data_len);
	free(str);
}