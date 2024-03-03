
#include "Dimmer.h"
/**
 * NOTE: Half Cycle for Australian Circuits: 10ms
 * 
 * NOTE: Logging should be avoided in this module as it could result in both cores trying to write at the same time.
 * 
 * NOTE: Flickering appears to be caused by the SDK setting "WIFI AMPDU TX" which groups together wifi data and send it in a single packet. This appears to block the interrupt and results in missing zero crossings.
 * 
 */

extern Relays relays;
#define TEST_SIGNAL_PIN 5

// #define TEST_MODE 1
// #define SIM_MODE 1

// #ifdef SIM_MODE
// #define FALLBACK_US 1000200ULL // 1.0002 Second
// #else
#define FALLBACK_US 10200ULL // 10.2ms
// #endif

volatile uint32_t last_triac_fire = micros();

volatile uint16_t fallback_execution_count = 0;
volatile uint32_t fallback_execution_compounding = 0;

volatile uint32_t isr_count_per_sec = 0;
volatile uint32_t last_isr_count_print = millis();
volatile bool zero_cross_disabled = false; // Flag for when zero cross is enabled/disabled
volatile uint32_t last_zero_cross_us = micros();

volatile uint32_t last_zero_cross_fire_print = millis();

volatile uint32_t last_dimming_smooth_ms = millis();


static uint16_t IRAM_ATTR calculate_pulse_active_us(uint64_t triac_delay_us){
    /**
     * @brief Calculate the amount of time to activate the triac based on "triac_pulse_percentage". If value is less than "triac_min_pulse_us" then this is the value returned.
     * 
     * triac_pulse_active_us = (triac_pulse_percentage /100.0) * (HALF_CYCLE_US - triac_delay_us)
     * 
     */

    double active_fraction = SYSTEM_CONFIGURATION.triac_pulse_percentage / 100.0;
    double result = active_fraction * (MAINS_HALF_CYCLE_US - triac_delay_us);
    uint16_t triac_pulse_active_us = uint16_t(result);
    if(triac_pulse_active_us > SYSTEM_CONFIGURATION.triac_min_pulse_us){
        return triac_pulse_active_us;
    }
    return SYSTEM_CONFIGURATION.triac_min_pulse_us;
}

// Trailing Edge Callbacks
static uint64_t IRAM_ATTR calculate_delay_trailing_us(uint32_t brightness){
    /**
     * @brief Calculates the delay in us required to achieve the desired brightness for trailing edge dimming.
     * 
     * As the driving Voltage is a sine wave switching the triac for 10% of the half cycle does not equate to 10% power output. 
     */
    double power_fraction = (brightness/ 100.0);
    double total_area = power_fraction * 2.0; // Integral of sin(0) to sin(pi) = cos(0) - cos(pi) = 2.0
    // total_area = cos(x) - cos(pi) = cos(x) + 1
    double temp = total_area - 1;
    double rads = acos(temp);
    double rads_per_us = PI / 10000.0;
    double result = (rads / rads_per_us);
    uint64_t res_int = uint64_t(result);
    return res_int;
}


static void IRAM_ATTR dimming_handler_trailing(void* arg){
    /**
     * @brief On Zero Crossing - Start timers to turn on triacs for period of half cycle.
     * 
     * NOTE: Timers are used to wait for X seconds before turning ON an output.
     * NOTE: Triac will turn OFF 
     */

    if(zero_cross_disabled){ // If debounce is active
        return;
    }
    zero_cross_disabled = true; // Otherwise set debounce enabled.
    if((micros() - last_zero_cross_us) > largest_zero_cross_delay){
        largest_zero_cross_delay = (micros() - last_zero_cross_us);
    }
    last_zero_cross_us = micros();

    #ifdef TEST_MODE
    isr_count_per_sec++;
    #endif
    // #ifdef SIM_MODE
    // Force Relays Channels OFF
    for(uint8_t i=0; i< NUM_CHANNELS; i++){
        struct relay* r = relays.get_relay(i);
        // GPIO.out_w1tc = ((uint32_t)1 << r->pin); // CLEAR 
        GPIO.out_w1ts = ((uint32_t)1 << r->pin); // SET 

        // r->status = 1;
        // r->brightness = 75;
    }
    // #endif
    int64_t time_now = micros(); //esp_timer_get_time();
    Dimmer* dim = (Dimmer*) arg;
    // Disable Interrupt from firing for another 9 ms
    uint64_t debounce_time_us = 9000;  // 9 ms
    esp_timer_start_once(dim->debounce_timer, debounce_time_us); // Timer will set zero_cross_disabled to false.
    
    // Fallback zero crossing timer if the interrupt doesn't fire in time.
    uint64_t fallback_time_us = FALLBACK_US;  // 10.2 ms
    esp_timer_start_once(dim->fallback_timer, fallback_time_us); // Timer will trigger zero crossing in 10ms if not already fired.

    // uint8_t set_end_timers[NUM_CHANNELS] = {0,0,0,0};
    // bool inital_delay_special_case = false;
    
    // // Special Case Implementation:
    uint8_t special_handle_pin[NUM_CHANNELS] = {0,0,0,0};
    uint8_t num_channels_on = 0;
    uint8_t special_case_detected = 0;
    // Figure out how many and what relays are on
    for(uint8_t i=0; i< NUM_CHANNELS; i++){
        struct relay* r = relays.get_relay(i);
        if(r->status == 1 && r-> current_brightness == 100){
            num_channels_on++;
            special_handle_pin[i] = r->pin;
            special_case_detected = 1;
        }
    }
    // // Calculate the delay for all channels based on expected 150W load
    int16_t special_delay_us = (int16_t)(12.0f + (18.0f * current_draw));
    special_delay_us = special_delay_us - 8; // 10us is the fixed execution overhead.
    if(special_case_detected){
        delayMicroseconds(special_delay_us); // Delay once for all channels
        // Handle turning on the channels as fast as possible
        for(uint8_t i=0; i< NUM_CHANNELS; i++){
            if(special_handle_pin[i] != 0){
                GPIO.out_w1ts = ((uint32_t)1 << special_handle_pin[i]); // SET
                // set_end_timers[i] = 1;
            }
        }
    }
    // --------------- 

    for(uint8_t i=0; i< NUM_CHANNELS; i++){
        struct relay* r = relays.get_relay(i);
        if(r->pin == special_handle_pin[i]){
            // Handled above - Skip
            continue;
        }
        // r->status = 1; //TODO: REMOVE!
        // r->brightness = 50; //TODO: REMOVE!
        if(r->status == 1 && r-> current_brightness > 0){
            // Bufffer Range - Values Near each end the ESP32 has trouble driving.
            if(r->current_brightness <= SYSTEM_CONFIGURATION.brightness_floor_percentage){ 
                GPIO.out_w1tc = ((uint32_t)1 << r->pin); // CLEAR 
                continue;
            }
            struct triac_timer_t* triac_tim = dim->get_triac_timer(i);
            if(triac_tim->timer_active){
                continue;
            }
            triac_tim->timer_active = true; // Indicate the timer is active.
            // uint32_t inverse_brightness = MAX_BRIGHTNESS - r->brightness;
            // triac_tim->timeout_us = calculate_delay_us(inverse_brightness); // NOTE: This can be moved out of this function and done each loop..
            triac_tim->timeout_us = r->timeout_us + 2; // Add 2us stagger
            triac_tim->set_time = time_now;
            esp_timer_start_once(triac_tim->timer, triac_tim->timeout_us);
        }else{
            GPIO.out_w1tc = ((uint32_t)1 << r->pin); // CLEAR 
        }
    }
}


void IRAM_ATTR triac_switching_timer_callback_trailing(void* arg){
    /**
     * @brief Handle Switching On the Channel After timer delay for trailing. 
     * 
     * NOTE: Triac will latch ON so turn off immediately.
     */
    struct triac_timer_t* timer = (struct triac_timer_t*) arg;
    timer->timer_active = false; // Indicate the timer is no longer active.
    int64_t time_now = micros();
    // Set GPIO Registers Directly - This allows for ISR to be stored in IRAM 
    GPIO.out_w1tc = ((uint32_t)1 << timer->relay->pin); // CLEAR
}

static uint64_t IRAM_ATTR calculate_delay_us(uint32_t brightness){
    /**
     * @brief Calculates the delay in us required to achieve the desired brightness.
     * 
     * As the driving Voltage is a sine wave switching the triac for 10% of the half cycle does not equate to 10% power output. 
     */
    double power_fraction = (1.0 - (brightness/ 100.0));
    double total_area = power_fraction * 2.0; // Integral of sin(0) to sin(pi) = cos(0) - cos(pi) = 2.0
    // total_area = cos(x) - cos(pi) = cos(x) + 1
    double temp = total_area - 1;
    double rads = acos(temp);
    double rads_per_us = PI / 10000.0;
    double result = (rads / rads_per_us);
    uint64_t res_int = uint64_t(result);
    return res_int;
}

static void IRAM_ATTR dimming_handler(void* arg){
    /**
     * @brief On Zero Crossing - Start timers to turn on triacs for period of half cycle.
     * 
     * NOTE: Timers are used to wait for X seconds before turning ON an output.
     * NOTE: Triac will turn OFF 
     */

    if(zero_cross_disabled){ // If debounce is active
        return;
    }
    zero_cross_disabled = true; // Otherwise set debounce enabled.
    if((micros() - last_zero_cross_us) > largest_zero_cross_delay){
        largest_zero_cross_delay = (micros() - last_zero_cross_us);
    }
    last_zero_cross_us = micros();

    #ifdef TEST_MODE
    isr_count_per_sec++;
    #endif
    // #ifdef SIM_MODE
    // Force Relays Channels OFF
    for(uint8_t i=0; i< NUM_CHANNELS; i++){
        struct relay* r = relays.get_relay(i);
        GPIO.out_w1tc = ((uint32_t)1 << r->pin); // CLEAR 
        delayMicroseconds(50);
        // r->status = 1;
        // r->brightness = 75;
    }
    // #endif
    int64_t time_now = micros(); //esp_timer_get_time();
    Dimmer* dim = (Dimmer*) arg;
    // Disable Interrupt from firing for another 9 ms
    uint64_t debounce_time_us = 9000;  // 9 ms
    esp_timer_start_once(dim->debounce_timer, debounce_time_us); // Timer will set zero_cross_disabled to false.
    
    // Fallback zero crossing timer if the interrupt doesn't fire in time.
    uint64_t fallback_time_us = FALLBACK_US;  // 10.2 ms
    esp_timer_start_once(dim->fallback_timer, fallback_time_us); // Timer will trigger zero crossing in 10ms if not already fired.

    uint8_t set_end_timers[NUM_CHANNELS] = {0,0,0,0};
    bool inital_delay_special_case = false;
    
    // Special Case Implementation:
    uint8_t special_handle_pin[NUM_CHANNELS] = {0,0,0,0};
    uint8_t num_channels_on = 0;
    uint8_t special_case_detected = 0;
    // Figure out how many and what relays are on
    for(uint8_t i=0; i< NUM_CHANNELS; i++){
        struct relay* r = relays.get_relay(i);
        if(r->status == 1 && r-> current_brightness == 100){
            num_channels_on++;
            special_handle_pin[i] = r->pin;
            special_case_detected = 1;
        }
    }
    // Calculate the delay for all channels based on expected 150W load
    int16_t special_delay_us = (int16_t)(12.0f + (18.0f * current_draw));
    special_delay_us = special_delay_us - 8; // 10us is the fixed execution overhead.
    if(special_case_detected){
        delayMicroseconds(special_delay_us); // Delay once for all channels
        // Handle turning on the channels as fast as possible
        for(uint8_t i=0; i< NUM_CHANNELS; i++){
            if(special_handle_pin[i] != 0){
                GPIO.out_w1ts = ((uint32_t)1 << special_handle_pin[i]); // SET
                set_end_timers[i] = 1;
            }
        }
    }
    // --------------- 

    for(uint8_t i=0; i< NUM_CHANNELS; i++){
        struct relay* r = relays.get_relay(i);
        if(r->pin == special_handle_pin[i]){
            // Handled above - Skip
            continue;
        }
        // r->status = 1; //TODO: REMOVE!
        // r->brightness = 50; //TODO: REMOVE!
        if(r->status == 1 && r-> current_brightness > 0){
            // Bufffer Range - Values Near each end the ESP32 has trouble driving.
            if(r->current_brightness <= SYSTEM_CONFIGURATION.brightness_floor_percentage){ 
                GPIO.out_w1tc = ((uint32_t)1 << r->pin); // CLEAR 
                continue;
            }
            // else if(r->brightness >= SYSTEM_CONFIGURATION.brightness_ceiling_percentage){
            //     GPIO.out_w1ts = ((uint32_t)1 << r->pin); // SET
            //     continue;
            // }
            struct triac_timer_t* triac_tim = dim->get_triac_timer(i);
            if(triac_tim->timer_active){
                continue;
            }
            triac_tim->timer_active = true; // Indicate the timer is active.
            // uint32_t inverse_brightness = MAX_BRIGHTNESS - r->brightness;
            // triac_tim->timeout_us = calculate_delay_us(inverse_brightness); // NOTE: This can be moved out of this function and done each loop..
            triac_tim->timeout_us = r->timeout_us + 2; // Add 2us stagger
            triac_tim->set_time = time_now;
            esp_timer_start_once(triac_tim->timer, triac_tim->timeout_us);
        }else{
            GPIO.out_w1tc = ((uint32_t)1 << r->pin); // CLEAR 
        }
    }

    // Loop for handling setting END timers after all triacs that need to be fired at 100% have been.
    for(uint8_t i=0; i< NUM_CHANNELS; i++){
        if(set_end_timers[i]){
             struct triac_timer_t* end_timer = (struct triac_timer_t*) dim->get_triac_timer(i)->end_timer;
            uint32_t triac_pulse_us = calculate_pulse_active_us(2);
            end_timer->timeout_us = triac_pulse_us;
            end_timer->set_time = time_now;
            esp_timer_start_once(end_timer->timer, end_timer->timeout_us);
        }
    }
}

void IRAM_ATTR triac_switching_timer_callback(void* arg){
    /**
     * @brief Handle Switching On the Channel After timer delay. 
     * 
     * NOTE: Triac will latch ON so turn off immediately.
     */
    struct triac_timer_t* timer = (struct triac_timer_t*) arg;
    timer->timer_active = false; // Indicate the timer is no longer active.
    int64_t time_now = micros();
    // Set GPIO Registers Directly - This allows for ISR to be stored in IRAM 
    GPIO.out_w1ts = ((uint32_t)1 << timer->relay->pin); // SET
    // Calculate the triac pulse duration
    uint32_t triac_pulse_us = calculate_pulse_active_us(timer->timeout_us);
    struct triac_timer_t* end_timer = (struct triac_timer_t*) timer->end_timer;
    end_timer->timeout_us = triac_pulse_us;
    end_timer->set_time = time_now;
    esp_timer_start_once(end_timer->timer, end_timer->timeout_us);
}

void IRAM_ATTR triac_end_pulse_callback(void* arg){
    /**
     * @brief Disable Triac Gate - Non-blocking way to keep triac gate open for longer periods.
     * 
     */
    struct triac_timer_t* timer = (struct triac_timer_t*) arg;
    uint32_t time_now = micros();
    if((time_now - timer->set_time - timer->timeout_us) > largest_delay_end){
        largest_delay_end = (time_now - timer->set_time - timer->timeout_us);
    }
    GPIO.out_w1tc = ((uint32_t)1 << timer->relay->pin); // CLEAR
}

static void IRAM_ATTR zero_cross_isr_handler(void* arg){
    /**
     * @brief Detects zero crossing and will clear fallback count if zero crossing is not already triggered.
     * 
     */
    Dimmer* dim = (Dimmer*)arg;
    if(!zero_cross_disabled){ 
        fallback_execution_count =0;
        if(SYSTEM_CONFIGURATION.trailing_edge && dim->currently_trailing_edge){
            dimming_handler_trailing(arg);
        }else{
            dimming_handler(arg);
        }
    }
}

static void IRAM_ATTR fallback_timer_callback(void* arg){
    /**
     * @brief Handle the zero crossing fallback timer.
     * 
     */
    if(fallback_execution_count > 3){
        return; // If this has triggered multiple times then don't execute
    }
    if(!zero_cross_disabled){ 
        // Only execute if the zero crossing ready for new signal.
        fallback_execution_count++;
        fallback_execution_compounding++;
        dimming_handler(arg);
    }else{
        fallback_execution_count =0;
    }
}

static void IRAM_ATTR debounce_timer_callback(void* arg){
    /**
     * @brief Re-enable GPIO interrupt after timer expires.
     * 
     */
    // gpio_intr_enable(ZERO_CROSSING_PIN);
    zero_cross_disabled = false;
}


static void dummy_triac_timer_callback(void* arg){
    /**
     * @brief Test method which would toggle the Channels OFF after 10ms.
     * 
     */
    relays.force_all_off();
}



static void signal_generator_timer_callback(void* arg){
    /**
     * @brief Generates a LOW interrupt every 10ms to simulate the Voltage Monitor
     * 
     */
    digitalWrite(TEST_SIGNAL_PIN, HIGH);
    digitalWrite(TEST_SIGNAL_PIN, LOW);
}

void Dimmer::loop(){
    /**
     * @brief Updates the timeout_us required for each relays brightness
     * 
     */
    if(SYSTEM_CONFIGURATION.trailing_edge != this->currently_trailing_edge){
        // If our current trailing/leading does match the expected state re-init the timers.
        reinit();
    }

    if(millis()> last_dimming_smooth_ms + 20){
        for(uint8_t i=0; i< NUM_CHANNELS; i++){
            struct relay* r = relays.get_relay(i);
            if(r->current_brightness < r->brightness){
                r->current_brightness++;
            }else if(r->current_brightness > r->brightness){
                r->current_brightness--;
            }
        }
        last_dimming_smooth_ms = millis();
    }

    for(uint8_t i=0; i< NUM_CHANNELS; i++){
        struct relay* r = relays.get_relay(i);
        uint32_t inverse_brightness = MAX_BRIGHTNESS - r->current_brightness;
        if(SYSTEM_CONFIGURATION.trailing_edge){
            r->timeout_us = calculate_delay_trailing_us(inverse_brightness);
        }else{
            r->timeout_us = calculate_delay_us(inverse_brightness);

        }
    }
    // if(millis() > last_zero_cross_fire_print + 10000){
    //     for (uint8_t i = 0; i< 4; i++){
    //         for(uint8_t j=0; j< NUM_CHANNELS; j++){
    //             log_d("Matches[%u][%u]: %u", i,j, matches[i].matches[j]);
    //         }
    //     }
    //     last_zero_cross_fire_print = millis();
    // }
    #ifdef TEST_MODE
    if(millis() > last_isr_count_print + 10000){
        log_d("Zero Crossings Per 10 Second: %u", isr_count_per_sec);
        isr_count_per_sec = 0;
        last_isr_count_print = millis();
        for(uint8_t i=0; i< NUM_CHANNELS; i++){
            struct relay* r = relays.get_relay(i);
            // log_d("Relay %c | Status %u", r->name, r->status);
        }
        log_d("Fallbacks Per 10 Second: %u", fallback_execution_compounding);
        fallback_execution_compounding =0;
    }
    #endif
}

void Dimmer::init(){
    /**
     * @brief Initialise triac timers and zero crossing interrupt.
     * 
     * TODO: Raise Flag on Fail - Handle on Main Core.
     */
    init_debounce_timer();
    init_fallback_timer();
    for(uint8_t i=0; i<NUM_CHANNELS; i++){
        if(SYSTEM_CONFIGURATION.trailing_edge){
            init_triac_timer(_triac_timers_arr[i], i,  _channel_names[i], &triac_switching_timer_callback_trailing, true);
            this->currently_trailing_edge = 1;
        }else{
            init_triac_timer(_triac_timers_arr[i], i,  _channel_names[i], &triac_switching_timer_callback, true);
            this->currently_trailing_edge = 0;
        }
    }
    init_zero_crossing();
    // testing_signal_generator();
}

void Dimmer::deinit_triac_timer(struct triac_timer_t* timer, uint8_t channel_num, bool main_timer){
    esp_timer_delete(timer->timer); // Delete timer
    if(main_timer){
        // Recusive also delete secondary timer
        deinit_triac_timer(_triac_end_timers_arr[channel_num], channel_num, false);
    }
}


void Dimmer::reinit(){
    for(uint8_t i=0; i<NUM_CHANNELS; i++){
        // De-init the timers
        deinit_triac_timer(_triac_timers_arr[i], i, true);
        // Re-init based on the current trailing edge state.
        if(SYSTEM_CONFIGURATION.trailing_edge){
            init_triac_timer(_triac_timers_arr[i], i,  _channel_names[i], &triac_switching_timer_callback_trailing, true);
            this->currently_trailing_edge = 1;
        }else{
            init_triac_timer(_triac_timers_arr[i], i,  _channel_names[i], &triac_switching_timer_callback, true);
            this->currently_trailing_edge = 0;
        }
    }
}

void Dimmer::init_fallback_timer(){
    /**
     * @brief Create a timer which will trigger the zero crossing if none is detected in the expected timer frame.
     * 
     */
    Dimmer* ptr = this;
    fallback_timer_config = {
        .callback = &fallback_timer_callback,
        .arg = (void*) (ptr),
        .dispatch_method = ESP_TIMER_ISR,
        .name = "fallback",
        .skip_unhandled_events = true
    };
    esp_err_t ret = esp_timer_create(&fallback_timer_config, &fallback_timer);
    if(ret != ESP_OK){
        return;
    }
}

void Dimmer::init_debounce_timer(){
    /**
     * @brief Create a timer which will be used to handle debouncing the zero crossing.
     * 
     */
    debounce_timer_config = {
        .callback = &debounce_timer_callback,
        .arg = (void*) NULL,
        .dispatch_method = ESP_TIMER_ISR,
        .name = _db_timer_name.c_str(),
        .skip_unhandled_events = true
    };
    esp_err_t ret = esp_timer_create(&debounce_timer_config, &debounce_timer);
    if(ret != ESP_OK){
        return;
    }
}


void Dimmer::testing_signal_generator(){
    /**
     * @brief Timers to generate test signals to see how functions behave.
     * 
     */
    // ----------------- TESTING ONLY ------------------
    // // Dummy Timer to simulate the Triac | Turns all relays OFF after 5ms
    // const esp_timer_create_args_t periodic_timer_args = {
    //         .callback = &dummy_triac_timer_callback,
    //         .arg = (void*) NULL,
    //         .dispatch_method = ESP_TIMER_TASK,
    //         .name = String('periodic').c_str(),
    //         .skip_unhandled_events = true
    // };
    // esp_timer_handle_t periodic_timer;
    // esp_timer_create(&periodic_timer_args, &periodic_timer);
    // esp_timer_start_periodic(periodic_timer, 10000);
    
    // Generate Signal - Simulates the expected output of Vol Mon
    pinMode(TEST_SIGNAL_PIN, OUTPUT);
    digitalWrite(TEST_SIGNAL_PIN, HIGH);
    const esp_timer_create_args_t periodic_timer_args = {
            .callback = &signal_generator_timer_callback,
            .arg = (void*) NULL,
            .dispatch_method = ESP_TIMER_TASK,
            .name = String('wav').c_str(),
            .skip_unhandled_events = true
    };
    esp_timer_handle_t periodic_timer;
    esp_timer_create(&periodic_timer_args, &periodic_timer);
    esp_timer_start_periodic(periodic_timer, 10000);
    // ----------------- END TESTING ------------------

}
void Dimmer::init_zero_crossing(){
    /**
     * @brief Configure the interrupt for zero crossing.
     * 
     */
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_ANYEDGE;
    io_conf.pin_bit_mask = ZERO_CROSSING_PIN_MASK;
    io_conf.mode = GPIO_MODE_INPUT;
    // #ifdef ZERO_CROSS_ACTIVE_LOW 
    // io_conf.intr_type = GPIO_INTR_NEGEDGE; // Older versions of the board appear to be active LOW.
    // io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    // log_d("Active LOW!");
    // #else
    // io_conf.intr_type = GPIO_INTR_POSEDGE; // Older versions of the board appear to be active LOW.
    // io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
    // log_d("Active HIGH!");
    // #endif
    gpio_config(&io_conf);
    gpio_uninstall_isr_service(); // Uninstall Previous Interrupt installation (Arduino)
    gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
    Dimmer* ptr = this;
    gpio_isr_handler_add(ZERO_CROSSING_PIN, zero_cross_isr_handler, (void*) (ptr));
}

struct triac_timer_t* Dimmer::init_triac_timer(struct triac_timer_t* timer, uint8_t channel_num, const char* name, void (*callback)(void *), bool main_timer){
    /**
     * @brief Initalises the configuration for each of the dimmer channel timers.
     * 
     *  NOTE: Using ESP_TIMER_ISR is faster but requires functions to be in IRAM.
     */
    timer->config = {
        .callback = callback,
        .arg = (void*) timer,
        .dispatch_method = ESP_TIMER_ISR,
        .name = name,
        .skip_unhandled_events = true
    };
    timer->channel_num = channel_num;
    timer->relay = relays.get_relay(channel_num);
    esp_err_t ret = esp_timer_create(&timer->config, &timer->timer);
    // Create End Timer and attach it to the main timer struct.
    if(main_timer){
        timer->end_timer = init_triac_timer(_triac_end_timers_arr[channel_num], channel_num, "end", &triac_end_pulse_callback, false);
    }
    return timer;
}

triac_timer_t* IRAM_ATTR Dimmer::get_triac_timer(uint8_t channel_num){
    /**
     * @brief Just a Getter - However needs to be in IRAM as it is used by the ISR.
     */
    return _triac_timers_arr[channel_num];
}