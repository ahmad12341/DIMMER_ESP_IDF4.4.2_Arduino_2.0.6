#include "StatusLED.h"

TaskHandle_t status_led_task = NULL;

void status_led_loop(void *){
    if(wifi_connected_flag){
        digitalWrite(STATUS_LED_PIN, HIGH);
    }else{
        digitalWrite(STATUS_LED_PIN, LOW);
        delay(1000);
        digitalWrite(STATUS_LED_PIN, HIGH);
    }
}

void status_led_init(){
    pinMode(STATUS_LED_PIN, OUTPUT);
    xTaskCreate(status_led_loop, "status_led_task", 1024, NULL, 1, &status_led_task);
}

