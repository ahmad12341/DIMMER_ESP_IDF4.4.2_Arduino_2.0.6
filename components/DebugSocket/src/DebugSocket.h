#pragma once
#ifndef DEBUG_SOCKET_H_
#define DEBUG_SOCKET_H_

#include "Arduino.h"
#include <lwip/sockets.h>
#include <lwip/netdb.h>
#include <errno.h>
#define DEBUG_LOG_UDP_MAX_PAYLOAD 2048
#define DEBUG_LOG_UDP_PORT 8888

namespace UDPLogger{
    int init();
    int is_alive();
}
#endif