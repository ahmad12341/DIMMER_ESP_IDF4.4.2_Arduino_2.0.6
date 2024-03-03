#include "RealTimeClock.h"

extern Relays relays;

void rtc_init(void* parameter)
{
	if (!wifi_internet_up)
	{
		vTaskDelete(NULL);
	}
	
	HTTPClient http;
	log_d("RTC Url: %s", SYSTEM_CONFIGURATION.rtc_url.c_str());
	http.begin(SYSTEM_CONFIGURATION.rtc_url);
	int32_t ret = http.GET();
	if (ret != 200)
	{
		log_w("Failed to get epoch time.");
		rtc_url_try_flag = 0;
		rtc_url_connect_try_time = millis();
		vTaskDelete(NULL);
	}
	log_d("RTC request response 200 OK");
	String epoch_str = http.getString();
	log_d("RTC epoch str: %s", epoch_str);
	rtc_start_time = rtc_millis();
	_rtc_epoch_time = epoch_str.toInt();
	if (_rtc_epoch_time == 0)
	{
		rtc_url_try_flag = 0;
		rtc_url_connect_try_time = millis();
		vTaskDelete(NULL);
	}
	
	rtc_time_diff = 0;
	
	log_d("Epoch time received: %d | rtc_start_time: %d", _rtc_epoch_time, rtc_start_time);
	rtc_init_flag = 1;
	vTaskDelete(NULL);
}

void rtc_init_task()
{
	static uint8_t started_flag = 0;
	static uint32_t start_time = 0;
	uint32_t timeDiff = millis() - start_time;
	if (!started_flag || timeDiff > 120000)
	{
		started_flag = 1;
		start_time = millis();
		xTaskCreate(rtc_init, "rtc_init", 10000, NULL, 1, NULL);
	}
	
}

void rtc_init(String epoch_str)
{	
	rtc_start_time = rtc_millis();
	_rtc_epoch_time = epoch_str.toInt();
	rtc_time_diff = 0;
	
	log_d("Epoch time received: %d | rtc_start_time: %d", _rtc_epoch_time, rtc_start_time);
	rtc_init_flag = 1;
}

void rtc_init(uint32_t epoch)
{
	rtc_start_time = rtc_millis();
	_rtc_epoch_time = epoch;
	rtc_time_diff = 0;
	log_d("Epoch time received: %d | rtc_start_time: %d", _rtc_epoch_time, rtc_start_time);
	rtc_init_flag = 1;
}

void rtc_loop()
{
	static uint32_t time = 0;
	static const uint32_t one_hour = 1800000L;
	rtc_millis_process();
	
	if (!rtc_init_flag) // Give the device 1 min to load global time
	{
		if (wifi_internet_up)
		{
			rtc_init_task();
		}
		
		return;
	}
	
	uint32_t epoch = rtc_get_epoch();
	uint32_t temp_day = rtc_get_day_num(epoch);
	uint32_t temp_hour = rtc_get_hour_num(epoch);
	if (temp_day > rtc_today)
	{
		// New day, store it
		if (rtc_today == 0)
		{
			rtc_today = temp_day;
		}
		else
		{
			rtc_new_day(temp_day, epoch);
		}
	}
	
	if (temp_hour > rtc_hour)
	{
		if (rtc_hour == 0)
		{
			rtc_hour = temp_hour;
		}
		else
		{
			rtc_new_hour(temp_hour, epoch);
		}
	}
	
	if (epoch < 1583256844L)
	{
		rtc_init_task();
	}
}

void rtc_new_day(uint32_t new_day, uint32_t epoch)
{
	rtc_today = new_day;
}

void rtc_new_hour(uint32_t new_hour, uint32_t epoch) 
{
	epoch = epoch + (SYSTEM_CONFIGURATION.timeZone * one_hour);
	epoch -= one_hour;
	files_manager_write_current_log(epoch);
	current_usage = 0;
	relays.reset_clicks_usage();
	relays.reset_on_time();
	rtc_hour = new_hour;
	program_status_change_flag = 1;
	//files_manager_write_program_status();
}

uint32_t rtc_get_epoch()
{
	int32_t timeDiff = (rtc_millis() - rtc_start_time) / 1000;
	if (timeDiff >= 0)
	{
		rtc_time_diff = timeDiff;
	}
	
	uint32_t epoch_time = _rtc_epoch_time + rtc_time_diff;
	return epoch_time;
}

String rtc_get_epoch_str()
{
	int32_t timeDiff = (rtc_millis() - rtc_start_time) / 1000;
	if (timeDiff >= 0)
	{
		rtc_time_diff = timeDiff;
	}
	uint32_t epoch_time = _rtc_epoch_time + rtc_time_diff;
	String ret = "";
	ret += epoch_time;
	return ret;
}

void rtc_millis_process()
{
	uint32_t ms = millis();
	int32_t timeDiff = ms - rtc_millis_last_time;
	if (timeDiff > 0)
	{
		rtc_millis_value += timeDiff;
	}
	else if (timeDiff < 0) // Rollover detected
	{
		int32_t max_int = (0xFFFFFFFF) / 1000; //microseconds 32bit max in milliseconds
		int32_t rollover_diff = max_int - rtc_millis_last_time; // Time before roll over
		rtc_millis_value += rollover_diff;
		
		rollover_diff = ms - 0; // Time after roll over
		rtc_millis_value += rollover_diff;
	}
	
	rtc_millis_last_time = ms;
}

uint32_t rtc_millis()
{
	rtc_millis_process();
	return rtc_millis_value;
}

int32_t rtc_get_day_num(uint32_t epoch)
{
	epoch = epoch + (SYSTEM_CONFIGURATION.timeZone * one_hour);
	int32_t day_num = epoch / one_day;
	
	return day_num;
}

uint32_t rtc_get_hour_num(uint32_t epoch)
{
	epoch = epoch + (SYSTEM_CONFIGURATION.timeZone * one_hour);
	uint32_t hour_num = epoch / one_hour;
	return hour_num;
}

int8_t rtc_get_weekday(uint32_t epoch)
{
	uint32_t temp_epoch = epoch;
	uint32_t ms_before = millis();
	int32_t days_count = 0;
	while (temp_epoch > one_day)
	{
		temp_epoch -= one_day;
		days_count++;
	}
	
	uint32_t timeDiff = millis() - ms_before;
	days_count = (days_count + 4) % 7;
	
	#ifdef SERIAL_PRINT
	switch (days_count)
	{
		case THURSDAY: Serial.println("Thursday"); break;
		case FRIDAY: Serial.println("Friday"); break;
		case SATURDAY: Serial.println("Saturday"); break;
		case SUNDAY: Serial.println("Sunday"); break;
		case MONDAY: Serial.println("Monday"); break;
		case TUESDAY: Serial.println("Tuesday"); break;
		case WEDNESDAY: Serial.println("Wednesday"); break;
	}
	#endif
	
	return days_count;
}

int8_t rtc_check_token_timeout(uint32_t epoch_time)
{
	static uint32_t full_day = 60 * 60 * 24;
	
	if (epoch_time == 0)
		return 1;
	
	if (!rtc_init_flag)
	{
		log_w("Token Timeout check failed - RTC not started.");
		return 0; // RTC hasn't started yet
	}
	
	uint32_t current_time = rtc_get_epoch();
	
	int32_t time_diff = current_time - epoch_time;
	if (time_diff >= full_day)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

DateTime rtc_get_date(uint32_t epoch)
{
	uint8_t leap_year = 1;
	DateTime dt;
	dt.month = 1; dt.day = 1; dt.year = 1972; dt.hour = 0; dt.minute = 0; dt.second = 0;
	
	epoch = epoch - (2 * one_year); // Start with 1972, make things easier
	
	while(epoch > four_years)
	{
		epoch -= four_years;
		dt.year += 4;
	}
	
	if (epoch >= one_year)
	leap_year = 0;
	
	while (epoch >= one_year)
	{
		epoch -= one_year;
		dt.year++;
	}
	
	if (epoch >= (one_day * 31)) // January
	{
		epoch = epoch - (one_day * 31);
		dt.month++;
	}
	else
	{
		rtc_process_days_minutes_seconds(epoch, &dt);
	}
	
	if (leap_year) // February
	{
		if (epoch >= (one_day * 29))
		{
			epoch = epoch - (one_day * 29);
			dt.month++;
		}
		else
		{
			rtc_process_days_minutes_seconds(epoch, &dt);
		}
	}
	else
	{
		if (epoch >= (one_day * 28))
		{
			epoch = epoch - (one_day * 28);
			dt.month++;
		}
		else
		{
			rtc_process_days_minutes_seconds(epoch, &dt);
		}
	}
	
	if (epoch >= (one_day * 31)) // March
	{
		epoch = epoch - (one_day * 31);
		dt.month++;
	}
	else
	{
		rtc_process_days_minutes_seconds(epoch, &dt);
	}
	
	if (epoch >= (one_day * 30)) // April
	{
		epoch = epoch - (one_day * 30);
		dt.month++;
	}
	else
	{
		rtc_process_days_minutes_seconds(epoch, &dt);
	}
	
	if (epoch >= (one_day * 31)) // May
	{
		epoch = epoch - (one_day * 31);
		dt.month++;
	}
	else
	{
		rtc_process_days_minutes_seconds(epoch, &dt);
	}
	
	if (epoch >= (one_day * 30)) // June
	{
		epoch = epoch - (one_day * 30);
		dt.month++;
	}
	else
	{
		rtc_process_days_minutes_seconds(epoch, &dt);
	}
	
	if (epoch >= (one_day * 31)) // July
	{
		epoch = epoch - (one_day * 31);
		dt.month++;
	}
	else
	{
		rtc_process_days_minutes_seconds(epoch, &dt);
	}
	
	if (epoch >= (one_day * 31)) // August
	{
		epoch = epoch - (one_day * 31);
		dt.month++;
	}
	else
	{
		rtc_process_days_minutes_seconds(epoch, &dt);
	}
	
	if (epoch >= (one_day * 30)) // September
	{
		epoch = epoch - (one_day * 30);
		dt.month++;
	}
	else
	{
		rtc_process_days_minutes_seconds(epoch, &dt);
	}
	
	if (epoch >= (one_day * 31)) // October
	{
		epoch = epoch - (one_day * 31);
		dt.month++;
	}
	else
	{
		rtc_process_days_minutes_seconds(epoch, &dt);
	}
	
	if (epoch >= (one_day * 30)) // November
	{
		epoch = epoch - (one_day * 30);
		dt.month++;
	}
	else
	{
		rtc_process_days_minutes_seconds(epoch, &dt);
	}
	
	if (epoch >= (one_day * 31)) // December
	{
		epoch = epoch - (one_day * 31);
		dt.month++;
	}
	else
	{
		rtc_process_days_minutes_seconds(epoch, &dt);
	}
	
	log_d("Date: %d-%d-%d\t%02d:%02d:%02d", dt.month, dt.day, dt.year, dt.hour, dt.minute, dt.second);
	
	return dt;
}

void rtc_process_days_minutes_seconds(uint32_t epoch, DateTime* dt)
{
	while (epoch >= one_day)
	{
		dt->day++;
		epoch -= one_day;
	}
	
	while (epoch >= one_hour)
	{
		dt->hour++;
		epoch -= one_hour;
	}
	
	while (epoch >= one_minute)
	{
		dt->minute++;
		epoch -= one_minute;
	}
	
	dt->second = epoch;
	
	return;
}