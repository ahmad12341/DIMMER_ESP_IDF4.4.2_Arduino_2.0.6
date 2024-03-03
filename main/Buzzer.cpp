#include "Buzzer.h"

void buzzer_init()
{
	if (!buzzer_init_flag)
	{
		buzzer_init_flag = 1;
		ledcSetup(BUZZER_CHANNEL, BUZZER_FREQUENCY, BUZZER_RESOLUTION);
		ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);
		ledcWriteTone(BUZZER_CHANNEL, 0);
	}
}

void buzzer_loop()
{
	if (buzzer_started)
	{
		uint32_t time_diff = millis() - buzzer_timer_start;
		if (time_diff >= buzzer_duration)
		{
			buzzer_count++;
			if (buzzer_count > buzzer_max_count)
			{
				buzzer_started = 0;
				ledcWriteTone(BUZZER_CHANNEL, 0);
				if (buzzer_reboot)
				{
					esp_restart();
				}
			}
			else
			{
				buzzer_timer_start = millis();
				if (buzzer_count %2 == 0)
				{
					ledcWriteTone(BUZZER_CHANNEL, BUZZER_FREQUENCY);
				}
				else
				{
					ledcWriteTone(BUZZER_CHANNEL, 0);
				}
			}
		}
	}
}

void buzzer_beep_duration(int16_t beep_t)
{
	buzzer_init();
	if (beep_t >= BUZZER_MAX_DURATION)
	{
		//spam, ignore it
		return;
	}
	
	buzzer_max_count = 2;
	buzzer_count = 0;
	buzzer_started = 1;
	buzzer_duration = beep_t;
	
	ledcWrite(BUZZER_CHANNEL, BUZZER_DUTY_CYCLE);
	ledcWriteTone(BUZZER_CHANNEL, BUZZER_FREQUENCY);
}

void buzzer_beep_n_times(int16_t beep_count)
{
	if (beep_count >= BUZZER_MAX_COUNT)
	{
		//spam, ignore it
		return;
	}
	
	buzzer_init();
	
	buzzer_max_count = (beep_count) * 2;
	buzzer_count = 0;
	buzzer_started = 1;
	buzzer_duration = BUZZER_DEFAULT_DURATION;
	
	ledcWrite(BUZZER_CHANNEL, BUZZER_DUTY_CYCLE);
	ledcWriteTone(BUZZER_CHANNEL, BUZZER_FREQUENCY);
}