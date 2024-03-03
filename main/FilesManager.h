#pragma once
#ifndef FILES_MANAGER_H_
#define FILES_MANAGER_H_
#include <Arduino.h>
#include "Globals.h"
#include "SPIFFS.h"
#include "ServerComm.h"
#include "RealTimeClock.h"
#include "SystemConfiguration.h"
#include "Logging.h"

void files_manager_init();
int32_t files_manager_get_free_space();
void files_manager_reset_config();
void files_manager_set_default_config_gang();
void files_manager_set_default_config();
void files_manager_read_config();
void files_manager_write_config();
void files_manager_write_current_log(uint32_t epoch);
void files_manager_read_current_log(struct Response_package response);
void files_manager_clear_current_log();
void files_manager_write_clear_log_file(struct Response_package response);
uint8_t files_manager_delete_log_file();
void files_manager_read_uuid_tokens();
void files_manager_write_uuid_tokens();
void files_manager_read_program_status();
void files_manager_write_program_status();
void files_manager_write_rtc();
void files_manager_read_rtc();
#endif