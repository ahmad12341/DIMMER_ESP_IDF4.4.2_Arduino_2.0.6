#include "Watchdog.h"

void IRAM_ATTR watchdog_woof()
{
	#ifdef SERIAL_PRINT
	Serial.println("Resetting ESP32");
	#endif
	files_manager_write_rtc();
	esp_restart();
}

void watchdog_init()
{
	watchdog_timer = timerBegin(0, 80, true);
	timerAttachInterrupt(watchdog_timer, &watchdog_woof, true);
	timerAlarmWrite(watchdog_timer, WATCHDOG_TIMEOUT, false);
	timerAlarmEnable(watchdog_timer);
}

void watchdog_init_mqtt()
{
	SYSTEM_CONFIGURATION.mqtt_connection_trial = 1;
	files_manager_write_config();
	watchdog_timer = timerBegin(0, 80, true);
	timerAttachInterrupt(watchdog_timer, &watchdog_woof, true);
	uint32_t timeout = 6 * 1000 * 1000;
	timerAlarmWrite(watchdog_timer, timeout, false);
	timerAlarmEnable(watchdog_timer);
}

void watchdog_feed()
{
	timerWrite(watchdog_timer, 0);
}