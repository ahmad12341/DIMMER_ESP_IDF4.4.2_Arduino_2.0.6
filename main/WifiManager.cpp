#include "WifiManager.h"

WebServer server(80);

extern ServerComms server_comms;

void wifi_setup()
{
	wifi_load_sta_info();
	
	if (SYSTEM_CONFIGURATION.wifiSSID == "" || SYSTEM_CONFIGURATION.wifiSSID == "default") // no wifi configured
	{
		wifi_program_status = WIFI_STATUS_AP;
		wifi_hotspot_sta();
	}
	else
	{
		wifi_program_status = WIFI_CONNECTING_STA;
		wifi_sta();
	}
}

int8_t wifi_hotspot_sta()
{
	//WiFi.disconnect();
	log_d("Starting Wifi AP");
	WiFi.setAutoReconnect(false);

	WiFi.mode(WIFI_AP);
	//WiFi.mode(WIFI_AP_STA);
	
	wifi_started_hotspot = 1;
	char* ssid_c = (char*)calloc(SYSTEM_CONFIGURATION.apSSID.length() + 2, 1);
	char* pwd_c = (char*)calloc(SYSTEM_CONFIGURATION.apPWD.length() + 2, 1);
	SYSTEM_CONFIGURATION.apSSID.toCharArray(ssid_c, SYSTEM_CONFIGURATION.apSSID.length() + 1);
	SYSTEM_CONFIGURATION.apPWD.toCharArray(pwd_c, SYSTEM_CONFIGURATION.apPWD.length() + 1);
	WiFi.enableAP(true); // Not needed?
	int8_t ret = WiFi.softAP(ssid_c, pwd_c, 10, 0, 4);
	IPAddress ip = WiFi.softAPIP();
	#ifdef SERIAL_PRINT
	Serial.print("AP Ip address: ");
	Serial.println(ip);
	Serial.printf("ssid: %s\tpwd: %s\tret: %d\n", ssid_c, pwd_c, ret);
	#endif
	
	webserver_init();
	
	free(ssid_c);
	free(pwd_c);
	
	return ret;
}

void wifi_load_sta_info()
{
	if (wifi_variables_flag)
	{
		free(wifi_ssid);
		free(wifi_pass);
	}
	
	wifi_ssid = (char*)calloc(SYSTEM_CONFIGURATION.wifiSSID.length() + 2, 1);
	wifi_pass = (char*)calloc(SYSTEM_CONFIGURATION.wifiPWD.length() + 2, 1);
	wifi_host_name = (char*)calloc(SYSTEM_CONFIGURATION.deviceHostName.length() + 2, 1);
	SYSTEM_CONFIGURATION.wifiSSID.toCharArray(wifi_ssid, SYSTEM_CONFIGURATION.wifiSSID.length() + 1);
	SYSTEM_CONFIGURATION.wifiPWD.toCharArray(wifi_pass, SYSTEM_CONFIGURATION.wifiPWD.length() + 1);
	SYSTEM_CONFIGURATION.deviceHostName.toCharArray(wifi_host_name, SYSTEM_CONFIGURATION.deviceHostName.length() + 1);
	wifi_variables_flag = 1;
}

void wifi_sta()
{
	log_d("Starting Wifi STA");
	WiFi.mode(WIFI_STA);
	wifi_started_hotspot = 0;
	WiFi.setHostname(wifi_host_name);
	if (!WiFi.config(WiFi.localIP(), WiFi.gatewayIP(), WiFi.subnetMask(), IPAddress(8,8,8,8))) {
		log_d("STA Failed to configure");
	}
	//esp_err_t ret = tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_STA ,"Mandeep");
	//if(ret != ESP_OK ){
		#ifdef SERIAL_PRINT
		//Serial.printf("failed to set hostname:%d\n",ret);
		#endif
	//}
	WiFi.begin(wifi_ssid, wifi_pass);
	WiFi.setAutoReconnect(true);
	wifi_sta_start_time = millis();
	wifi_program_status = WIFI_CONNECTING_STA;
}

void wifi_connected_action()
{
	IPAddress ip = WiFi.localIP();
	#ifdef SERIAL_PRINT
	Serial.printf("Wifi connected. IP is: %d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3]);
	#endif
	wifi_program_status = WIFI_CONNECTED;
	webserver_init();
	//wifi_mdns_init();
	wifi_internet_up = 1;
	mqtt_connect_task();
	WiFi.enableAP(false);
	wifi_started_hotspot = 0;
	ota_broadcast();
	restart_broadcast();
	rtc_init_task();
	wifi_mdns_init();
}

String wifi_get_ip()
{
	String str = "";
	
	IPAddress ip = WiFi.localIP();
	str += ip[0]; str += ".";
	str += ip[1]; str += ".";
	str += ip[2]; str += ".";
	str += ip[3];
	
	return str;
}

void webserver_loop()
{
	server.handleClient();
}

void wifi_loop()
{
	wifi_connection_status_loop();
	wifi_hotspot_loop();
}

void wifi_mdns_init()
{
	bool ret = MDNS.begin(SYSTEM_CONFIGURATION.serialNo.c_str());
	if (ret)
	{
		#ifdef SERIAL_PRINT
		Serial.print("MDNS succeeded. SerialNo: ");
		Serial.println(SYSTEM_CONFIGURATION.serialNo);
		#endif
		MDNS.addService("http", "tcp", 80);
	}
	else
	{
		#ifdef SERIAL_PRINT
		Serial.println("MDNS failed");
		#endif
	}
}

void wifi_hotspot_loop()
{
	/**
	 * Wifi Hotspot Loop - Wifi Reconnect should be disabled.
	 * 
	 */

	static uint8_t auto_reconnect_var = 0;
	if (!wifi_started_hotspot)
	{
		return;
	}
	
	esp_wifi_ap_get_sta_list(&wifi_ap_station_list);
	if (wifi_ap_station_list.num > 0 && !auto_reconnect_var)
	{
		#ifdef SERIAL_PRINT
		Serial.println("device connected");
		#endif
		auto_reconnect_var = 1;
		wifi_ap_connected = 1;
		//Why is Wifi Disconnect Called?
		// WiFi.setAutoReconnect(false);
		// WiFi.disconnect();
	}
	else if (wifi_ap_station_list.num == 0 && auto_reconnect_var)
	{
		#ifdef SERIAL_PRINT
		Serial.println("device disconnected");
		#endif
		auto_reconnect_var = 0;
		wifi_ap_connected = 0;
		// These are WifiSTA methods?
		// WiFi.setAutoReconnect(true);
		// WiFi.reconnect();
	}
}

void wifi_connection_status_loop()
{
	if (!WiFi.isConnected())
	{
		if (wifi_connected_flag)
		{
			wifi_connected_flag = 0;
			wifi_sta_start_time = millis();
			wifi_started_hotspot = 0;
			
		}
		
		uint32_t timeDiff = millis() - wifi_sta_start_time;
		if (timeDiff >= WIFI_AP_SWAP_TIMEOUT && !wifi_started_hotspot && wifi_program_status != WIFI_STATUS_AP) // 15 seconds
		{
			#ifdef SERIAL_PRINT
			Serial.println("Failed to connect to wifi, starting AP");
			#endif
			// Timed out,start AP mode
			wifi_program_status = WIFI_STATUS_AP;
			wifi_ap_start_time = millis();
			wifi_hotspot_sta();
		}
	}
	else
	{
		if (!wifi_connected_flag)
		{
			wifi_connected_flag = 1;
			// enable http server
			wifi_connected_action();
		}
	}
	
	// This seems useless? Wifi_ap_connect is =1 if a device has connected so if no device connected
	// and we are in AP mode and 90seconds has passed then swap to station??
	// NOTE: This looks like it is here to reconnect to STA in the event of something like a router power loss.
	if (wifi_ap_connected == 0 && wifi_program_status == WIFI_STATUS_AP)
	{
		uint32_t time_diff = millis() - wifi_ap_start_time;
		if (time_diff >= 90000) // 90 seconds
		{
			wifi_sta();
			wifi_sta_start_time = millis();
		}
	}
}

void webserver_init()
{
	static uint8_t webserver_initialized = 0;
	if (!webserver_initialized)
	{
		#ifdef SERIAL_PRINT
		Serial.println("Starting webserver");
		#endif
		server.on("/", HTTP_POST, webserver_handle_root_post);
		server.on("/", HTTP_GET, webserver_handle_root_get);
		server.on("/info", HTTP_GET, webserver_handle_get_info);
		server.begin();
		webserver_initialized = 1;
	}
}

void webserver_send_jsresponse(char* msg)
{
	String origin_header_name = "Access-Control-Allow-Origin";
	String origin_header_value = "*";
	server.sendHeader(origin_header_name, origin_header_value);
	server.send(200, "application/json", msg);
}

void webserver_send_jsresponse(String msg)
{
	char* char_arr = (char*)calloc(msg.length() + 4, 1);
	msg.toCharArray(char_arr, msg.length() + 1, 0);
	server.send(200, "application/json", char_arr);
	free(char_arr);
}

void webserver_handle_root_get()
{
	String device_user = server.arg(String("u"));
	String device_pass = server.arg(String("p"));
	String wifi_ssid_str = server.arg(String("ws"));
	String wifi_pwd_str = server.arg(String("wp"));
	
	struct Response_package resp_pack;
	resp_pack.resp_type = SCR_HTTP;
	resp_pack.cmd_i_val = 0xFF;
	resp_pack.req_id = String("GET_WIFI");
	//TODO: Remove This!
	log_d("Device User: %s | Pwd: %s", SYSTEM_CONFIGURATION.deviceUser, SYSTEM_CONFIGURATION.devicePwd);
	log_d("MQTT User: %s | Pwd: %s", SYSTEM_CONFIGURATION.mqttUser, SYSTEM_CONFIGURATION.mqttPWD);

	if (device_user != SYSTEM_CONFIGURATION.deviceUser || device_pass != SYSTEM_CONFIGURATION.devicePwd)
	{
		
		server_comms.send_custom(String("Invalid_device_user_pass"), resp_pack);
		return;
	}
	
	if (wifi_ssid_str == "")
	{
		server_comms.send_custom(String("Wifi_SSID_blank"), resp_pack);
		return;
	}
	
	SYSTEM_CONFIGURATION.wifiSSID = wifi_ssid_str;
	SYSTEM_CONFIGURATION.wifiPWD = wifi_pwd_str;
	server_comms.send_custom(String("OK"), resp_pack);
	files_manager_write_config();
	ESP.restart();
	wifi_load_sta_info();
	wifi_sta();
}

void log_args(){
	for(int i=0; i<server.args(); i++){
		String arg_name = server.argName(i);
		String arg_val = server.arg(arg_name);
		log_d("Argument: %s | Value: %s", arg_name.c_str(), arg_val.c_str());
	}
}

void log_headers(){
	for(int i=0; i<server.headers(); i++){
		String arg_name = server.headerName(i);
		String arg_val = server.header(arg_name);
		log_d("Header: %s | Value: %s", arg_name.c_str(), arg_val.c_str());
	}
}

void webserver_handle_root_post()
{
	String body = server.arg("plain");
	log_d("Webserver POST Body: %s", body.c_str());
	// log_args();
	// log_headers();
	log_d("Device User: %s | Device Pwd %s",SYSTEM_CONFIGURATION.deviceUser.c_str(),SYSTEM_CONFIGURATION.devicePwd.c_str());
	int str_len = body.length() + 1;
	char char_array[str_len];
	body.toCharArray(char_array, str_len);
	char_array[str_len - 1] = 0;
	
	server_comms.factory((uint8_t*)char_array, str_len, SCR_HTTP);
}

void webserver_handle_get_info()
{
	String ret_str = "{";
		
	ret_str += "\"" + SYS_CONFIG_SERIALNO + "\":\"" + SYSTEM_CONFIGURATION.serialNo + "\",";
	ret_str += "\"" + SYS_CONFIG_DEVICENO + "\":\"" + SYSTEM_CONFIGURATION.deviceNo + "\",";
	ret_str += "\"" + SYS_CONFIG_WIFI_SSID + "\":\"" + SYSTEM_CONFIGURATION.wifiSSID + "\",";
	ret_str += "\"" + SYS_CONFIG_AP_SSID + "\":\"" + SYSTEM_CONFIGURATION.apSSID+ "\",";
	ret_str += "\"firmware\":\"" + FIRMWARE_VERSION_NUMBER + "\"}";
	
	uint16_t str_len = ret_str.length() + 1;
	char* char_array = (char*)calloc(str_len, 1);
	ret_str.toCharArray(char_array, str_len);
	webserver_send_jsresponse(char_array);
	
	free(char_array);
}

void webserver_handle_404()
{
	String message = "File Not Found\n\n";
	message += "URI: ";
	message += server.uri();
	message += "\nMethod: ";
	message += (server.method() == HTTP_GET) ? "GET" : "POST";
	message += "\nArguments: ";
	message += server.args();
	message += "\n";
	for (uint8_t i = 0; i < server.args(); i++) {
		message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
	}
	server.send(404, "text/html", message);
}