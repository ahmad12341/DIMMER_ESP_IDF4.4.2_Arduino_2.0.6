#pragma once
#ifndef BROADCAST_H_
#define BROADCAST_H_
#include <Arduino.h>
#include "Globals.h"
#include "RelayController.h"
#include "Logging.h"

void broadcast_loop();
void broadcast_send_switch_status();
void broadcast_send(String str);
void broadcast_send();

#endif