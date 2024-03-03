#pragma once
#ifndef USER_HANDLER_H_
#define USER_HANDLER_H_
#include <Arduino.h>
#include "Globals.h"
#include "FilesManager.h"
#include "RealTimeClock.h"
#include "Logging.h"

void user_handler_init();
void user_handler_expand_uuid_storage();
void user_handler_shrink_uuid_storage();
int8_t user_handler_add_uuid(String id);
int8_t user_handler_remove_uuid(String id);
int32_t user_handler_uuid_login(String id);
void user_handler_loop();
String user_handler_get_all_uuid();
int8_t user_handler_validate_token(uint32_t token);
int8_t user_handler_register(String parameters[], int16_t params_length);
int32_t user_handler_login(String parameters[], int16_t params_length);
int8_t user_handler_deregister(String parameters[], int16_t params_length);
int32_t generate_token();

#endif