#include "Ota.h"


std::map<ota_errors, std::string> ota_error_msgs = {
    {NVS_ERASE_FAILED, "No free NVS pages and erase failed."},
    {NVS_INIT_FAILED, "Failed to initialise NVS for OTA."},
    {NO_WIFI, "No WiFi Connected - Failed to start OTA."},   
    {PARTITIONS_CORRUPTED, "OTA boot data or prefered boot image is corrupted."},
    {NULL_PARTITION, "Failed to get OTA update partition."},
    {HTTP_INIT_FAILED, "Failed to initialize HTTP for OTA update | Check Internet Connectivity."},
	{HTTP_GET_FAILED, "HTTP Get request failed after mutiple retries."},
    {HTTP_SIZE_ZERO, "Failed to get OTA update size."},
    {HTTP_INVALID_STATUS, "HTTP status code was not the expected 200 or 206."},
    {ESP_OTA_BEGIN_FAILED, "Failed to start esp_ota_begin."},
    {ESP_OTA_WRITE_FAILED, "Failed to write esp_ota data."},
    {ESP_OTA_END_FAILED, "OTA image is invalid."},
    {ESP_OTA_SET_BOOT_FAILED, "Failed to set OTA partition as new boot partition."},
    {HTTP_STREAM_DISCONNECTED, "HTTP stream unexpectedly disconnected."},
    {HTTP_STREAM_RESTART_FAILED, "Failed to restart the HTTP stream."},
    {OTA_TIMEOUT, "OTA took too long and timed out."},
};

uint8_t OTA::init_flag = 0; // initalise static member variable

extern const uint8_t rootca_crt_bundle_start[] asm("_binary_x509_crt_bundle_start");

bool is_https(const std::string& url) {
    return url.compare(0, 8, "https://") == 0;
}

void OTA::set_mutable_url(const char* const_url){
	size_t length = strlen(const_url) + 1; // Add 1 for the null terminator
    url = new char[length];
    strcpy(url, const_url);
}

bool OTA::begin(const char* url){
	if(init_flag){
		log_d("OTA process is already running..");
		return false;
	}
	init_flag = 1;
	set_mutable_url(url);
	ota_errors result = ota_process();
	init_flag = 0; // Clear flag after process complete.
	if(result != SUCCESS){
		log_w("OTA FAILED: %s", ota_error_msgs[result].c_str());
		return false;
	}else{
		log_d("OTA Update Succesful!");
		return true;
	}
}

OTA::~OTA(){
	if(_http != NULL){
		delete _http;
		_http = NULL;
	}
	// Note: _ota_client is used in _http so delete second
	if(_ota_client != NULL){
		delete _ota_client;
		_ota_client = NULL;
	}
	if(url != NULL){
		delete[] url;
		url = NULL;
	}
}

ota_errors OTA::wifi_status_check(){
	return (wifi_ap_connected || WiFi.isConnected()) ? SUCCESS : NVS_INIT_FAILED;
}

ota_errors OTA::nvs_init_check(){
	esp_err_t err = nvs_flash_init();
	if (err == ESP_ERR_NVS_NO_FREE_PAGES){
		if (nvs_flash_erase() != ESP_OK){
			return NVS_ERASE_FAILED;
		}
		err = nvs_flash_init();
	}
	return (err == ESP_OK) ? SUCCESS : NVS_INIT_FAILED;
}

ota_errors OTA::partitions_corruption_check(){
	const esp_partition_t* configured = esp_ota_get_boot_partition();
	const esp_partition_t* running = esp_ota_get_running_partition();
	
	// Make sure that the partitions that we will write on are not corrupted
	if (configured != running)
	{
		log_d("Configured OTA boot partition at offset 0x%08x, but running from offset 0x%08x", configured->address, running->address);
		log_d("(This can happen if either the OTA boot data or preferred boot image become corrupted somehow)");
		return PARTITIONS_CORRUPTED;
	}
	return SUCCESS;
}

ota_errors OTA::init_http_client(){
	_http = new HTTPClient();
	_http->addHeader("Accept-Encoding", "chunked");
    _http->setReuse(false);
	if(is_https(url)){
        _ota_client = new WiFiClientSecure;
        ((WiFiClientSecure*)_ota_client)->setCACertBundle(rootca_crt_bundle_start);
    }else{
        _ota_client = new WiFiClient;
    }
	log_d("OTA URL: %s", url);
	int32_t ret = _http->begin(*_ota_client, url);
	if (!ret){
        log_d("Failed to initialize HTTP for OTA update | Check Internet Connectivity.");
        return HTTP_INIT_FAILED;
    }
    return SUCCESS;
}

ota_errors OTA::get_ota_size(){
	int32_t status_code = http_get_with_retries();
	if(status_code != 200 && status_code != 206) return HTTP_INVALID_STATUS;
	update_size = _http->getSize();
	update_size_save = update_size;
	return (update_size != 0) ? SUCCESS: HTTP_SIZE_ZERO;
}

ota_errors OTA::get_update_partition() {
	update_partition = esp_ota_get_next_update_partition(NULL);
    log_d("Writing to partition subtype %d at offset %08x", update_partition->subtype, update_partition->address);
	return (update_partition != NULL) ? SUCCESS: NULL_PARTITION;
}

ota_errors OTA::check_all_data_valid(){
	return (esp_ota_end(update_handle) == ESP_OK) ? SUCCESS: ESP_OTA_END_FAILED;
}

ota_errors OTA::check_set_boot_partition(){
	return (esp_ota_set_boot_partition(update_partition) == ESP_OK) ? SUCCESS: ESP_OTA_SET_BOOT_FAILED;
}
ota_errors OTA::init_esp_ota(){
    return (esp_ota_begin(update_partition, update_size, &update_handle) == ESP_OK) ? SUCCESS: ESP_OTA_BEGIN_FAILED;
}

ota_errors OTA::ota_process(){
	ota_errors result = wifi_status_check();
	if (result != SUCCESS) return result;

	result = nvs_init_check();
	if (result != SUCCESS) return result;

	result = partitions_corruption_check();
	if (result != SUCCESS) return result;
	result = get_update_partition();
	if (result != SUCCESS) return result;

	result = init_http_client();
	if (result != SUCCESS) return result;

	result = get_ota_size();
	if (result != SUCCESS) return result;

	result = init_esp_ota();
	if (result != SUCCESS) return result;

	result = download_chunks();
	if (result != SUCCESS) return result;

	_http->end();

	result = check_all_data_valid();
	if (result != SUCCESS) return result;

	result = check_set_boot_partition();
	if (result != SUCCESS) return result;

	log_d("[UPDATING] All Succeeded | Rebooting ESP32");
	SYSTEM_CONFIGURATION.ota_broadcast = 1;
	files_manager_write_config();
    return SUCCESS;
}

int32_t OTA::http_get_with_retries(){
	int32_t ret = 0;
	for (int i = 0; i < 5; i++){
		if (!ret){
			ret = _http->begin(*_ota_client, url);
		}
		ret = _http->GET();
		log_d("OTA HTTP Get Return Code: %d", ret);
		if (ret == 200 || ret == 206){
			return ret;
		}
		delay(1);
	}
	return ret;
}

ota_errors OTA::restart_with_range(){
	// _http->end();
	int32_t range_to_call = update_size_save - update_size;
	String range = "bytes=" + String(range_to_call) + "-";
	delay(500);
	int32_t ret = _http->begin(*_ota_client, url);
	log_d("Restart HTTP Begin ret: %d | Range %d", ret, range_to_call);
	_http->addHeader("Range", range);
	int32_t status_code = http_get_with_retries();
	if (status_code == 206){
		log_d("[UPDATING] OTA Update Successfully Restarted");
		stream = _http->getStreamPtr();
		log_d("[UPDATING] Get Code: %d | Size %d", ret, _http->getSize());
		delay(2000);
		packet_update_time = start_update_time = millis();
		return SUCCESS;
	}else{
		log_w("OTA Update failed to restart | Return Code:  %d", ret);
		return HTTP_STREAM_RESTART_FAILED;
	}
}

ota_errors OTA::download_chunks(){
	log_d("[UPDATING] Update Begining...");
	log_d("0 ---------- 100");
	stream = _http->getStreamPtr();
	stream->setTimeout(60);
	ota_errors ret = SUCCESS;
	while ((update_size > 0)){
		avail_size = stream->available();
		delay(1); // Make sure watchdog isn't triggered.
		if (avail_size > 0){
			ret = store_chunk(); // If something available store it.
			if (ret != SUCCESS) return ret;
		}else{
			if(!stream->connected()){
				log_w("OTA Update Stream Disconnected");
				return HTTP_STREAM_DISCONNECTED;
			}
		}
		timeDiff = millis() - packet_update_time;
		if (timeDiff >= download_stalled_timeout){ 
			log_w("OTA Download Stalled - Attempting to restart...");
			ret = restart_with_range();
			if (ret != SUCCESS) return ret;
		}
		timeDiff = millis() - start_update_time;
		if (timeDiff >= main_download_timeout){
			log_w("OTA Failed - Download time exceded maximum timeout: %lu ms", main_download_timeout);
			return OTA_TIMEOUT;
		}
	}
	log_d("Exited OTA Update Loop.");
	return ret;
}

ota_errors OTA::store_chunk(){
	// stream_ret = stream->readBytes(ota_write_data, avail_size > OTA_BUFFSIZE ? OTA_BUFFSIZE : avail_size);
	stream_ret = stream->readBytes(ota_write_data, avail_size > OTA_BUFFSIZE ? OTA_BUFFSIZE : avail_size);
	total_size += stream_ret;
	if (stream_ret > 0){
		delay(100);
		int32_t ret = esp_ota_write(update_handle, (const void*)ota_write_data, stream_ret);
		if (ret != ESP_OK){
			log_w("Failed to write OTA");
			return ESP_OTA_WRITE_FAILED;
		}
	}
	update_size -= stream_ret;
	packet_update_time = millis();
	log_d("[UPDATING] Chunk Size: %d | Stream Ret %d | Update Size: %d | Counter: %d", avail_size, stream_ret, update_size, counter++);
	esp_task_wdt_reset();
	return SUCCESS;
}


void ota_broadcast(){
	/**
	 * Publishes to MQTT & UDP after a successful OTA.
	 */
	if (SYSTEM_CONFIGURATION.ota_broadcast != 1){
		return;
	}
	SYSTEM_CONFIGURATION.ota_broadcast = 0;
	files_manager_write_config();
	String msg = "0000," + SYSTEM_CONFIGURATION.deviceNo + ",C7," + FIRMWARE_VERSION_NUMBER + ", OTA update successful";
	broadcast_send(msg);
	mqtt_publish_msg(msg);
	
	wifi_mdns_init();
}

void restart_broadcast(){
	/**
	 * Publishes to MQTT & UDP after reboot (specifically 0xC6 CMD).
	 */
	if (SYSTEM_CONFIGURATION.restart_broadcast != 1){
		return;
	}
	SYSTEM_CONFIGURATION.restart_broadcast = 0;
	files_manager_write_config();
	String msg = "0000," + SYSTEM_CONFIGURATION.deviceNo + ",C6,TRUE";
	broadcast_send(msg);
	mqtt_publish_msg(msg);
	WiFi.macAddress();
	WiFi.BSSID();
}

