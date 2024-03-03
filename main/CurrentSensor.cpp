#include "CurrentSensor.h"

extern Relays relays;

void current_sensor_init()
{
	/**
	 * @brief Sets up the power monitoring.
	 * 
	 * NOTE: Looks like we can't use interrupts - the dimmer interrupts take priority and result in misses.
	 */
	hlw8012.begin(HLW8012_CF_PIN, HLW8012_CF1_PIN, HLW8012_SEL_PIN, HIGH, false, HLW8012_PULSE_TIMEOUT);
	hlw8012.setResistors(HLW8012_CURRENT_RESISTOR, HLW8012_RESISTOR_UPSTREAM, HLW8012_RESISTOR_DOWNSTREAM);
}


void current_sensor_loop()
{	
	uint32_t timeDiff = millis() - hlw8012_update_time;
	int relays_on = relays.any_relays_on();

	if (timeDiff >= HLW8012_UPDATE_TIME && relays_on)
	{
		hlw8012_update_time = millis();

		activepower = hlw8012.getActivePower();
		
		current_draw = (double)activepower / 240.0f;
		current_usage += (((double)activepower / 3600.0f) * (timeDiff / 1000)); // This is Wh
		log_d("Active Power: %u W", activepower);
		log_d("Current Draw: %f A", ((double)activepower / 240.0f));
		log_d("Current Usage: %f Wh", current_usage);
		// hlw8012.toggleMode();
	}
	
	if (!relays_on)
	{
		activepower = 0;
	}
}
