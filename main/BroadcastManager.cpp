#include <BroadcastManager.h>

extern Relays relays;

void broadcast_loop()
{
	static uint32_t config_send_timer = 0;
	if (!WiFi.isConnected())
	{
		return;
	}
	
	uint32_t timeDiff = millis() - config_send_timer;
	if (timeDiff >= (SYSTEM_CONFIGURATION.broadcastIntervalWifi * 1000))
	{
		config_send_timer = millis();
		broadcast_send();
	}
}

void broadcast_send_switch_status()
{
	/**
	 * @brief Broadcast switch status:
	 * 4 Channel Dimmer Response Format:
	 *  DeviceNumber, encoded_status_byte, current_usage, relay_a_clicks, relay_b_clicks,  relay_c_clicks,  relay_d_clicks, relay_a_brightness, relay_b_brightness, relay_c_brightness, relay_d_brightness, activepower. 
	 * 
	 *	4 Channel Response Format:
	 *	DeviceNumber, encoded_status_byte, current_usage, relay_a_clicks, relay_b_clicks,  relay_c_clicks,  relay_d_clicks, activepower. 
	 * 	
	 * 	2 Channel Response Format (Outdated):
	 * 	DeviceNumber, encoded_status_byte, current_usage, relay_a_clicks, relay_b_clicks,activepower. 
	 */
	char* char_arr = (char*)calloc(200, 1);	

	int prefix_len = SYSTEM_CONFIGURATION.deviceNo.length() + 1;
	char* prefix = (char*)calloc(prefix_len + 2, 1);
	SYSTEM_CONFIGURATION.deviceNo.toCharArray(prefix, prefix_len);

	relays.build_response_data(prefix, char_arr);
	
	IPAddress subnet = WiFi.subnetMask();
	IPAddress ip = WiFi.localIP();
	IPAddress ba;
	ba[0] = ip[0] | ~subnet[0]; 
	ba[1] = ip[1] | ~subnet[1];
	ba[2] = ip[2] | ~subnet[2];
	ba[3] = ip[3] | ~subnet[3];
	
	broadcast_udp.beginPacket(broadcast_ip, 10000);
	broadcast_udp.printf("%s", char_arr);
	broadcast_udp.endPacket();
	broadcast_udp.beginPacket(ba, 10000);
	broadcast_udp.printf("%s", char_arr);
	broadcast_udp.endPacket();
	
	free(char_arr);
	free(prefix);
}

void broadcast_send(String str)
{
	//str = "0000," + SYSTEM_CONFIGURATION.deviceNo + ", " + str;
	if (wifi_program_status != WIFI_CONNECTED)
	{
		return;
	}
	
	int str_len = str.length() + 1;
	char* char_array = (char*)calloc(str_len + 2, 1);
	str.toCharArray(char_array, str_len);
	
	IPAddress subnet = WiFi.subnetMask();
	IPAddress ip = WiFi.localIP();
	IPAddress ba;
	ba[0] = ip[0] | ~subnet[0];
	ba[1] = ip[1] | ~subnet[1];
	ba[2] = ip[2] | ~subnet[2];
	ba[3] = ip[3] | ~subnet[3];
	
	broadcast_udp.beginPacket(broadcast_ip, 10000);
	broadcast_udp.printf("%s", char_array);
	broadcast_udp.endPacket();
	broadcast_udp.beginPacket(ba, 10000);
	broadcast_udp.printf("%s", char_array);
	broadcast_udp.endPacket();
	
	free(char_array);
}

void broadcast_send()
{
	char* char_array = (char*)calloc(200, 1);

	String prefix_str = "0000,";
	prefix_str += SYSTEM_CONFIGURATION.deviceNo + ",infodevice,";
	prefix_str += WiFi.macAddress();
	relays.build_response_data(prefix_str, char_array);
	
	//get broadcast address
	IPAddress subnet = WiFi.subnetMask();
	IPAddress ip = WiFi.localIP();
	IPAddress ba;
	ba[0] = ip[0] | ~subnet[0];
	ba[1] = ip[1] | ~subnet[1];
	ba[2] = ip[2] | ~subnet[2];
	ba[3] = ip[3] | ~subnet[3];
	
	broadcast_udp.beginPacket(broadcast_ip, 10000);
	broadcast_udp.printf("%s", char_array);
	broadcast_udp.endPacket();
	broadcast_udp.beginPacket(ba, 10000);
	broadcast_udp.printf("%s", char_array);
	broadcast_udp.endPacket();
	free(char_array);
}
