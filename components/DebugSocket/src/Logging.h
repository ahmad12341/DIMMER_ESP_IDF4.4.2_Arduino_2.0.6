#pragma once
#ifndef LOGGING_H_
#define LOGGING_H_

/**
 * ESP-IDF Arduino core defines some alternative logging functions - this header file should be imported after "Arduino.h" and will redefine the logging macros to use ESP-IDF default macros.
 */
#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

#include "esp_log.h"

#define CUSTOM_LOG_FORMAT(letter, format)  ARDUHAL_LOG_COLOR_ ## letter "[" #letter "][%s:%u] %s(): " format ARDUHAL_LOG_RESET_COLOR , pathToFileName(__FILE__), __LINE__, __FUNCTION__


#ifdef log_v
#undef log_v
#define log_v(format, ...) ESP_LOGV("", CUSTOM_LOG_FORMAT(V, format), ##__VA_ARGS__)
#endif

#ifdef log_d
#undef log_d
#define log_d(format, ...) ESP_LOGD("", CUSTOM_LOG_FORMAT(D, format), ##__VA_ARGS__)
#endif

#ifdef log_i
#undef log_i
#define log_i(format, ...) ESP_LOGI("", CUSTOM_LOG_FORMAT(I, format), ##__VA_ARGS__)
#endif

#ifdef log_w
#undef log_w
#define log_w(format, ...) ESP_LOGW("", CUSTOM_LOG_FORMAT(W, format), ##__VA_ARGS__)
#endif

#ifdef log_e
#undef log_e
#define log_e(format, ...) ESP_LOGE("", CUSTOM_LOG_FORMAT(E, format), ##__VA_ARGS__)
#endif


#endif