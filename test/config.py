SERIAL_PORT = "COM3"
BAUD_RATE = 115200

BUILD_DIR = "C:/Users/Admin/Documents/PlatformIO/Projects/infinite_tech_ir_esp-idf/sample_project/build"

MQTT_USER = "m0c1e052aad09447b9d6f30b06e8d5b0c"
MQTT_PWD = "pf15abf74f41a41"
MQT_TOPIC = "m0c1e052aad09447b9d6f30b06e8d5b0c/mobile"
MQTT_CALLBACK = "m0c1e052aad09447b9d6f30b06e8d5b0c/device"


FLASH_COMMAND =  f"python -m esptool -p {SERIAL_PORT} -b 460800 --before default_reset --after hard_reset " \
                f"--chip esp32 write_flash --flash_mode dio --flash_freq 40m --flash_size detect 0x10000 " \
                f"{BUILD_DIR}/main.bin 0x1000 {BUILD_DIR}/bootloader/bootloader.bin 0x8000 {BUILD_DIR}/partition_table/partition-table.bin"\
                f" 0xe000 {BUILD_DIR}/ota_data_initial.bin"

ERASE_COMMAND = f"python -m esptool -p {SERIAL_PORT} erase_flash"

RESET_DEVICE_COMMAND = f"python -m esptool -p {SERIAL_PORT} --chip esp32 --before default_reset run"

SETUP_REQUEST_STR = 'C3,{"owner":"ksuaning@gmail.com","mqttUser":"m0c1e052aad09447b9d6f30b06e8d5b0c","mqttPwd":"pf15abf74f41a41","mqttHost":"inet.infiniteautomation.net","mqttPort":"1883","mqttTopicPub":"m0c1e052aad09447b9d6f30b06e8d5b0c/mobile","mqttTopicConfig":"m0c1e052aad09447b9d6f30b06e8d5b0c/config","mqttTopicSub":"m0c1e052aad09447b9d6f30b06e8d5b0c/device","wifiSSID":"Telstra17F9","wifiPWD":"4901677210","broadcastMQTTInterval":"300"}'


SERIAL_NUMBER = "A1659013276546" # This is used for mDNS so we can use this value instead of the device IP
REQUEST_ID = '1659700043793' #Think this is just a random string.
DEVICE_NUMBER = 'da10ea386'
USER_ID = '123'  # User ID 
ANDROID_ID = "ANDROID-INF-ebcc1ee903978155"
USER_TOKEN = 'E84BB7C3'
DEVICE_USER = "u80487548"
DEVICE_PWD = "pb6b833e2"

# SETUP_TOKEN_REQUEST_STR = f'D0,{{"id":"{USER_TOKEN}","u":"{DEVICE_USER}","p":"{DEVICE_PWD}"}}'
SETUP_USER_REQUEST_STR = f'D0,{{"id":"{USER_ID}","u":"default","p":"default"}}'
SETUP_ANDROID_USER_REQUEST_STR = f'D0,{{"id":"{ANDROID_ID}","u":"default","p":"default"}}'

# SETUP_TOKEN_REQUEST_STR = f'D1,{{"id":"{USER_TOKEN}","u":"default","p":"default"}}'

LOGIN_USER_GET_TOKEN_STR = f'{REQUEST_ID},{DEVICE_NUMBER},D1,{{"id":"{USER_ID}","u":"{DEVICE_USER}","p":"{DEVICE_PWD}"}}'
LOGIN_ANDROID_USER_GET_TOKEN_STR = f'{REQUEST_ID},{DEVICE_NUMBER},D1,{{"id":"{ANDROID_ID}","u":"{DEVICE_USER}","p":"{DEVICE_PWD}"}}'