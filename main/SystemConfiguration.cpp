#include "SystemConfiguration.h"

void system_config_check_config_request()
{
	if ((SYSTEM_CONFIGURATION.owner != String("infinite")
	&& SYSTEM_CONFIGURATION.serialNo == String("default")
	&& SYSTEM_CONFIGURATION.mqttTopicSub != String("default/device") 
	&& SYSTEM_CONFIGURATION.mqttTopicPub != String("default/mobile")
	&& SYSTEM_CONFIGURATION.mqttTopicConfig != String("default/config")
	&& SYSTEM_CONFIGURATION.owner != String("")
	&& SYSTEM_CONFIGURATION.mqttTopicSub != String("")
	&& SYSTEM_CONFIGURATION.mqttTopicPub != String("")
	&& SYSTEM_CONFIGURATION.mqttTopicConfig != String("")
	&& mqtt_client.connected())
	|| (SYSTEM_CONFIGURATION.config_owner_changed && mqtt_client.connected()))
	{
		SYSTEM_CONFIGURATION.config_server_request = 1;
	}
	else
	{
		SYSTEM_CONFIGURATION.config_server_request = 0;
	}
}

String system_config_get_all_config()
{
	String ret_str = "{";
	
	ret_str += "\"" + SYS_CONFIG_GANG_TYPE + "\":\"" + SYSTEM_CONFIGURATION.gangType + "\",";
	ret_str += "\"" + SYS_CONFIG_SERIALNO + "\":\"" + SYSTEM_CONFIGURATION.serialNo + "\",";
	ret_str += "\"" + SYS_CONFIG_DEVICENO + "\":\"" + SYSTEM_CONFIGURATION.deviceNo + "\",";
	ret_str += "\"" + SYS_CONFIG_DEVICE_USER + "\":\"" + SYSTEM_CONFIGURATION.deviceUser + "\",";
	ret_str += "\"" + SYS_CONFIG_DEVICE_PWD + "\":\"" + SYSTEM_CONFIGURATION.devicePwd + "\",";
	ret_str += "\"" + SYS_CONFIG_OWNER + "\":\"" + SYSTEM_CONFIGURATION.owner + "\",";

	ret_str += "\"" + SYS_CONFIG_HARD_TIMER_A + "\":\"" + SYSTEM_CONFIGURATION.hard_timer_a + "\",";
	ret_str += "\"" + SYS_CONFIG_HARD_TIMER_B + "\":\"" + SYSTEM_CONFIGURATION.hard_timer_b + "\",";
	ret_str += "\"" + SYS_CONFIG_HARD_TIMER_C + "\":\"" + SYSTEM_CONFIGURATION.hard_timer_c + "\",";
	ret_str += "\"" + SYS_CONFIG_HARD_TIMER_D + "\":\"" + SYSTEM_CONFIGURATION.hard_timer_d + "\",";

	ret_str += "\"" + SYS_CONFIG_DEVICE_IP + "\":\"" + wifi_get_ip() + "\",";
	ret_str += "\"" + SYS_CONFIG_DEVICE_MAC + "\":\"" + WiFi.macAddress() + "\",";
		
	ret_str += "\"" + SYS_CONFIG_DEVICELOCK_CONFIG + "\":\"" + SYSTEM_CONFIGURATION.deviceLockConfig + "\",";
	ret_str += "\"" + SYS_CONFIG_DEVICELOCK_CONTROL + "\":\"" + SYSTEM_CONFIGURATION.deviceLockControl + "\",";
	ret_str += "\"" + SYS_CONFIG_DEVICELOCK_TOTAL + "\":\"" + SYSTEM_CONFIGURATION.deviceLockTotal + "\",";
		
	ret_str += "\"" + SYS_CONFIG_DEVICE_RESET + "\":\"" + SYSTEM_CONFIGURATION.deviceReset + "\",";
	ret_str += "\"" + SYS_CONFIG_DEVICE_HOSTNAME + "\":\"" + SYSTEM_CONFIGURATION.deviceHostName + "\",";
		
	ret_str += "\"" + SYS_CONFIG_AP_SSID + "\":\"" + SYSTEM_CONFIGURATION.apSSID + "\",";
	ret_str += "\"" + SYS_CONFIG_AP_PWD + "\":\"" + SYSTEM_CONFIGURATION.apPWD + "\",";
	ret_str += "\"" + SYS_CONFIG_WIFI_SSID + "\":\"" + SYSTEM_CONFIGURATION.wifiSSID + "\",";
	ret_str += "\"" + SYS_CONFIG_WIFI_PWD + "\":\"" + SYSTEM_CONFIGURATION.wifiPWD + "\",";
		
	ret_str += "\"" + SYS_CONFIG_MQTT_HOST + "\":\"" + SYSTEM_CONFIGURATION.mqttHost + "\",";
	ret_str += "\"" + SYS_CONFIG_MQTT_PORT + "\":\"" + SYSTEM_CONFIGURATION.mqttPort + "\",";
	ret_str += "\"" + SYS_CONFIG_MQTT_USER + "\":\"" + SYSTEM_CONFIGURATION.mqttUser + "\",";
	ret_str += "\"" + SYS_CONFIG_MQTT_PASSWORD + "\":\"" + SYSTEM_CONFIGURATION.mqttPWD + "\",";
	ret_str += "\"" + SYS_CONFIG_MQTT_TOPIC_PUB + "\":\"" + SYSTEM_CONFIGURATION.mqttTopicPub + "\",";
	ret_str += "\"" + SYS_CONFIG_MQTT_TOPIC_SUB + "\":\"" + SYSTEM_CONFIGURATION.mqttTopicSub + "\",";
	ret_str += "\"" + SYS_CONFIG_MQTT_TOPIC_CONFIG + "\":\"" + SYSTEM_CONFIGURATION.mqttTopicConfig + "\",";
	ret_str += "\"" + SYS_CONFIG_RTC_URL + "\":\"" + SYSTEM_CONFIGURATION.rtc_url + "\",";
	ret_str += "\"" + SYS_CONFIG_RTC + "\":\"" + rtc_get_epoch_str() + "\",";
	ret_str += "\"" + SYS_CONFIG_RTC_TIMEZONE + "\":\"" + SYSTEM_CONFIGURATION.timeZone+ "\",";
	ret_str += "\"" + SYS_CONFIG_SWITCH_TYPE + "\":\"" + SYSTEM_CONFIGURATION.switchType + "\",";
	//ret_str += "\"" + SYS_CONFIG_MQTT_cert_URL + "\":\"" + SYSTEM_CONFIGURATION.mqtt_cert_url + "\",";
	ret_str += "\"" + SYS_CONFIG_BROADCAST_MQTT_INTERVAL + "\":\"" + SYSTEM_CONFIGURATION.broadcastIntervalMqtt + "\",";
	ret_str += "\"" + SYS_CONFIG_BROADCAST_WIFI_INTERVAL + "\":\"" + SYSTEM_CONFIGURATION.broadcastIntervalWifi + "\",";

	ret_str += "\"" + SYS_CONFIG_BRIGHTNESS_CEILING_PERCENTAGE + "\":\"" + SYSTEM_CONFIGURATION.brightness_ceiling_percentage + "\",";
	ret_str += "\"" + SYS_CONFIG_BRIGHTNESS_FLOOR_PERCENTAGE + "\":\"" + SYSTEM_CONFIGURATION.brightness_floor_percentage + "\",";

	ret_str += "\"" + SYS_CONFIG_TRIAC_PULSE_PERCENTAGE + "\":\"" + SYSTEM_CONFIGURATION.triac_pulse_percentage + "\",";
	ret_str += "\"" + SYS_CONFIG_MIN_TRIAC_PULSE_US + "\":\"" + SYSTEM_CONFIGURATION.triac_min_pulse_us + "\",";
	ret_str += "\"" + SYS_CONFIG_TRAILING_EDGE + "\":\"" + SYSTEM_CONFIGURATION.trailing_edge + "\",";
	ret_str += "\"" + SYS_CONFIG_STARTUP_BRIGHTNESS + "\":\"" + SYSTEM_CONFIGURATION.startup_brightness + "\"";

	ret_str += "}";
	
	#ifdef SERIAL_PRINT
	//Serial.printf("str_len: %d\n", ret_str.length());
	//Serial.print("ret_str: ");
	//Serial.println(ret_str);
	#endif
	
	return ret_str;
}

String system_config_get_by_params(String parameters[], int16_t params_length)
{
	String ret_str = "{";
	String temp;
	uint8_t flag = 0;
	for (int i = 0; i < params_length; i++)
	{
		flag = 0;
		temp = system_config_get_value_by_name(parameters[i], &flag);
		if (!flag)
		{
			ret_str += "\"" + parameters[i] + "\":\"" + temp + "\"";
			if (i < (params_length - 1))
				ret_str += ",";
		}
	}
	
	ret_str += "}";
	
	return ret_str;
}


void system_config_set_by_params(String parameters[], int16_t params_length)
{
	for (int i = 0; i < params_length; i += 2)
	{
		if (i < params_length - 1) // Not last i, we should never reach this condition, just for protection
		{
			system_config_set_value_by_name(parameters[i], parameters[i + 1]);
		}
	}
	
	files_manager_write_config();
}

uint8_t check_percentage_in_range(uint8_t val, uint8_t default_val){
	if(val > 100 || val < 0){
		return default_val;
	}
	return val;
}

uint8_t constrain_trailing_edge_setting(uint8_t val, uint8_t default_val){
	if(val > 1 || val < 0){
		return default_val;
	}
	return val;
}

String system_config_get_value_by_name(String param_name, uint8_t* flag)
{
	if (param_name == SYS_CONFIG_SERIALNO)
		return SYSTEM_CONFIGURATION.serialNo;
	else if (param_name == SYS_CONFIG_DEVICENO)
		return SYSTEM_CONFIGURATION.deviceNo;
	else if (param_name == SYS_CONFIG_DEVICE_USER)
		return SYSTEM_CONFIGURATION.deviceUser;
	else if (param_name == SYS_CONFIG_DEVICE_PWD)
		return SYSTEM_CONFIGURATION.devicePwd;
	else if (param_name == SYS_CONFIG_GANG_TYPE)
		return SYSTEM_CONFIGURATION.gangType;
	else if (param_name == SYS_CONFIG_HARD_TIMER_A)
		return String(SYSTEM_CONFIGURATION.hard_timer_a);
	else if (param_name == SYS_CONFIG_HARD_TIMER_B)
		return String(SYSTEM_CONFIGURATION.hard_timer_b);
	else if (param_name == SYS_CONFIG_HARD_TIMER_C)
		return String(SYSTEM_CONFIGURATION.hard_timer_c);
	else if (param_name == SYS_CONFIG_HARD_TIMER_D)
		return String(SYSTEM_CONFIGURATION.hard_timer_d);

	if (param_name == SYS_CONFIG_DEVICE_IP)
		return wifi_get_ip();
	else if (param_name == SYS_CONFIG_DEVICE_MAC)
		return WiFi.macAddress();
	
	if (param_name == SYS_CONFIG_DEVICELOCK_CONFIG)
		return SYSTEM_CONFIGURATION.deviceLockConfig;
	else if (param_name == SYS_CONFIG_DEVICELOCK_CONTROL)
		return SYSTEM_CONFIGURATION.deviceLockControl;
	else if (param_name == SYS_CONFIG_DEVICELOCK_TOTAL)
		return SYSTEM_CONFIGURATION.deviceLockTotal;
	
	if (param_name == SYS_CONFIG_DEVICE_RESET)
		return SYSTEM_CONFIGURATION.deviceReset;
	else if (param_name == SYS_CONFIG_DEVICE_HOSTNAME)
		return SYSTEM_CONFIGURATION.deviceHostName;
	
	if (param_name == SYS_CONFIG_AP_SSID)
		return SYSTEM_CONFIGURATION.apSSID;
	else if (param_name == SYS_CONFIG_AP_PWD)
		return SYSTEM_CONFIGURATION.apPWD;
	else if (param_name == SYS_CONFIG_WIFI_SSID)
		return SYSTEM_CONFIGURATION.wifiSSID;
	else if (param_name == SYS_CONFIG_WIFI_PWD)
		return SYSTEM_CONFIGURATION.wifiPWD;
	
	if (param_name == SYS_CONFIG_MQTT_HOST)
		return SYSTEM_CONFIGURATION.mqttHost;
	else if (param_name == SYS_CONFIG_MQTT_PORT)
		return SYSTEM_CONFIGURATION.mqttPort;
	else if (param_name == SYS_CONFIG_MQTT_USER)
		return SYSTEM_CONFIGURATION.mqttUser;
	else if (param_name == SYS_CONFIG_MQTT_PASSWORD)
		return SYSTEM_CONFIGURATION.mqttPWD;
	else if (param_name == SYS_CONFIG_MQTT_TOPIC_PUB)
		return SYSTEM_CONFIGURATION.mqttTopicPub;
	else if (param_name == SYS_CONFIG_MQTT_TOPIC_SUB)
		return SYSTEM_CONFIGURATION.mqttTopicSub;
	else if (param_name == SYS_CONFIG_MQTT_TOPIC_CONFIG)
		return SYSTEM_CONFIGURATION.mqttTopicConfig;
	else if (param_name == SYS_CONFIG_MQTT_cert_URL)
		return SYSTEM_CONFIGURATION.mqtt_cert_url;	
		
	else if (param_name == SYS_CONFIG_RTC_URL)
		return SYSTEM_CONFIGURATION.rtc_url;
	else if (param_name == SYS_CONFIG_RTC_TIMEZONE)
		return String(SYSTEM_CONFIGURATION.timeZone);
	else if (param_name == SYS_CONFIG_RTC)
		return rtc_get_epoch_str();
	else if (param_name == SYS_CONFIG_SWITCH_TYPE)
		return SYSTEM_CONFIGURATION.switchType;
	else if (param_name == SYS_CONFIG_BROADCAST_WIFI_INTERVAL)
		return String(SYSTEM_CONFIGURATION.broadcastIntervalWifi);
	else if (param_name == SYS_CONFIG_BROADCAST_MQTT_INTERVAL)
		return String(SYSTEM_CONFIGURATION.broadcastIntervalMqtt);
	else if (param_name == SYS_CONFIG_OWNER)
		return SYSTEM_CONFIGURATION.owner;
		
	String ret = "";
	*flag = 1;
	return ret;
}

void system_config_set_value_by_name(String param_name, String param_value)
{
	if (param_name == SYS_CONFIG_SERIALNO)
	{
		SYSTEM_CONFIGURATION.serialNo = param_value;
		wifi_mdns_init();
	}
	else if (param_name == SYS_CONFIG_DEVICENO)
		SYSTEM_CONFIGURATION.deviceNo = param_value;
	else if (param_name == SYS_CONFIG_DEVICE_USER)
	{
		SYSTEM_CONFIGURATION.deviceUser = param_value;
		
		if (SYSTEM_CONFIGURATION.device_configured == 0 && SYSTEM_CONFIGURATION.deviceUser != "default" && SYSTEM_CONFIGURATION.devicePwd != "default" && mqtt_client.connected())
		{
			// Device configured
			SYSTEM_CONFIGURATION.device_configured = 1;
			buzzer_beep_n_times(5);
			buzzer_reboot = 1;
		}
	}
	else if (param_name == SYS_CONFIG_DEVICE_PWD)
	{
		SYSTEM_CONFIGURATION.devicePwd = param_value;
		
		if (SYSTEM_CONFIGURATION.device_configured == 0 && SYSTEM_CONFIGURATION.deviceUser != "default" && SYSTEM_CONFIGURATION.devicePwd != "default" && mqtt_client.connected())
		{
			// Device configured
			SYSTEM_CONFIGURATION.device_configured = 1;
			buzzer_beep_n_times(5);
			buzzer_reboot = 1;
		}	
	}
	
	//else if (param_name == SYS_CONFIG_DEVICE_IP)
		//SYSTEM_CONFIGURATION.deviceIP = param_value;
	//else if (param_name == SYS_CONFIG_DEVICE_MAC)
		//SYSTEM_CONFIGURATION.deviceMac = param_value;
	
	else if (param_name == SYS_CONFIG_DEVICELOCK_CONFIG)
		SYSTEM_CONFIGURATION.deviceLockConfig = param_value;
	else if (param_name == SYS_CONFIG_DEVICELOCK_CONTROL)
		SYSTEM_CONFIGURATION.deviceLockControl = param_value;
	else if (param_name == SYS_CONFIG_DEVICELOCK_TOTAL)
		SYSTEM_CONFIGURATION.deviceLockTotal = param_value;
	
	else if (param_name == SYS_CONFIG_DEVICE_RESET)
		SYSTEM_CONFIGURATION.deviceReset = param_value;
	else if (param_name == SYS_CONFIG_DEVICE_HOSTNAME)
		SYSTEM_CONFIGURATION.deviceHostName = param_value;
	
	else if (param_name == SYS_CONFIG_AP_SSID)
		SYSTEM_CONFIGURATION.apSSID = param_value;
	else if (param_name == SYS_CONFIG_AP_PWD)
	{
		if (param_value.length() >= 8)
		{
			SYSTEM_CONFIGURATION.apPWD = param_value;
		}
	}
	else if (param_name == SYS_CONFIG_WIFI_SSID)
		SYSTEM_CONFIGURATION.wifiSSID = param_value;
	else if (param_name == SYS_CONFIG_WIFI_PWD)
		SYSTEM_CONFIGURATION.wifiPWD = param_value;
	
	else if (param_name == SYS_CONFIG_MQTT_HOST)
	{
		SYSTEM_CONFIGURATION.mqttHost = param_value;
		mqtt_set_server();
	}
	else if (param_name == SYS_CONFIG_MQTT_PORT)
	{
		SYSTEM_CONFIGURATION.mqttPort = param_value;
		mqtt_set_server();
	}
	else if (param_name == SYS_CONFIG_MQTT_USER)
	{
		SYSTEM_CONFIGURATION.mqttUser = param_value;
		mqtt_set_server();
	}
	else if (param_name == SYS_CONFIG_MQTT_PASSWORD)
	{
		SYSTEM_CONFIGURATION.mqttPWD = param_value;
		mqtt_set_server();
	}
	else if (param_name == SYS_CONFIG_MQTT_TOPIC_PUB)
	{
		SYSTEM_CONFIGURATION.mqttTopicPub = param_value;
		mqtt_set_server();
	}
	else if (param_name == SYS_CONFIG_MQTT_TOPIC_SUB)
	{
		SYSTEM_CONFIGURATION.mqttTopicSub = param_value;
		mqtt_set_server();
	}
	else if (param_name == SYS_CONFIG_MQTT_TOPIC_CONFIG)
	{
		SYSTEM_CONFIGURATION.mqttTopicConfig = param_value;
		mqtt_set_server();
	}
		
	else if (param_name == SYS_CONFIG_RTC_URL)
		SYSTEM_CONFIGURATION.rtc_url = param_value;
	else if (param_name == SYS_CONFIG_RTC_TIMEZONE)
		SYSTEM_CONFIGURATION.timeZone = param_value.toInt();
	else if (param_name == SYS_CONFIG_RTC)
		rtc_init(param_value);
	else if (param_name == SYS_CONFIG_SWITCH_TYPE)
		SYSTEM_CONFIGURATION.switchType = param_value;
	else if (param_name == SYS_CONFIG_MQTT_cert_URL)
		SYSTEM_CONFIGURATION.mqtt_cert_url = param_value;
	else if (param_name == SYS_CONFIG_BROADCAST_WIFI_INTERVAL)
		SYSTEM_CONFIGURATION.broadcastIntervalWifi = param_value.toInt();
	else if (param_name == SYS_CONFIG_BROADCAST_MQTT_INTERVAL)
		SYSTEM_CONFIGURATION.broadcastIntervalMqtt = param_value.toInt();
	else if (param_name == SYS_CONFIG_HARD_TIMER_A)
		SYSTEM_CONFIGURATION.hard_timer_a = param_value.toInt();
	else if (param_name == SYS_CONFIG_HARD_TIMER_B)
		SYSTEM_CONFIGURATION.hard_timer_b = param_value.toInt();
	else if (param_name == SYS_CONFIG_HARD_TIMER_C)
		SYSTEM_CONFIGURATION.hard_timer_c = param_value.toInt();
	else if (param_name == SYS_CONFIG_HARD_TIMER_D)
		SYSTEM_CONFIGURATION.hard_timer_d = param_value.toInt();
	else if (param_name == SYS_CONFIG_OWNER)
		SYSTEM_CONFIGURATION.owner = param_value;
	else if (param_name == SYS_CONFIG_MIN_TRIAC_PULSE_US)
		SYSTEM_CONFIGURATION.triac_min_pulse_us = param_value.toInt();
	else if (param_name == SYS_CONFIG_TRIAC_PULSE_PERCENTAGE)
		SYSTEM_CONFIGURATION.triac_pulse_percentage = check_percentage_in_range(param_value.toInt(), 95);
	else if (param_name == SYS_CONFIG_BRIGHTNESS_CEILING_PERCENTAGE)
		SYSTEM_CONFIGURATION.brightness_ceiling_percentage = check_percentage_in_range(param_value.toInt(), 95);
	else if (param_name == SYS_CONFIG_BRIGHTNESS_FLOOR_PERCENTAGE)
		SYSTEM_CONFIGURATION.brightness_floor_percentage = check_percentage_in_range(param_value.toInt(), 5);
	else if (param_name == SYS_CONFIG_TRAILING_EDGE)
		SYSTEM_CONFIGURATION.trailing_edge = constrain_trailing_edge_setting(param_value.toInt(), 0); // Only allow either 0 or 1
	else if (param_name == SYS_CONFIG_STARTUP_BRIGHTNESS)
		SYSTEM_CONFIGURATION.startup_brightness = check_percentage_in_range(param_value.toInt(), 100); // 0 - 100 range

}
