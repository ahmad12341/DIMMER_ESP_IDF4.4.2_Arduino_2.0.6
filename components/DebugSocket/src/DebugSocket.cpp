#include "DebugSocket.h"

namespace UDPLogger{
	/**
	 * Broadcasts the device log to the local network via UDP socket on port 8888
	 * 
 	*/

	IPAddress ip_addr(255, 255, 255, 255);;

	int sock;
	static struct sockaddr_in serveraddr;
	static uint8_t buf[DEBUG_LOG_UDP_MAX_PAYLOAD];

	int is_alive(){
		return sock;
	}

	int get_error(){
		/**
		 * @brief Get the socket error number.
		 * 
		 */
		int result;
		uint32_t optlen = sizeof(int);
		if(getsockopt(sock, SOL_SOCKET, SO_ERROR, &result, &optlen) == -1) {
			printf("getsockopt failed");
			return -1;
		}
		return result;
	}

	int print_error(){
		/**
		 * @brief Decode socket error number and print in readable format.
		 * 
		 */
		int err = get_error();
		printf("Debug UDP socket error %d %s", err, strerror(err));
		return err;
	}

	int kill(va_list l){
		/**
		 * @brief Close the debugging UDP socket.
		 * 
		 */
		int err = 0;
		char *err_buf;
		esp_log_set_vprintf(vprintf);
		if( (err = shutdown(sock, 2)) == 0 ){
			vprintf("\nDebug UDP socket shutdown!", l);
		}else{
			asprintf(&err_buf, "\nShutting-down Debug UDP socket failed: %d!\n", err);
			vprintf(err_buf, l);
		}

		if( (err = close( sock )) == 0 )
		{
			vprintf("\nDebug UDP socket closed!", l);
			sock = 0; // Make sure sock is zero
		}else
		{
			asprintf(&err_buf, "\n Closing Debug UDP socket failed: %d!\n", err);
			vprintf(err_buf, l);
		}
	}


	int send_log(const char *str, va_list l){
		/**
		 * @brief Wraps vprintf to allow for additional logging behaviours.
		 * 
		 */
		int err = 0;
		int len;
		char task_name[16];
		char *cur_task = pcTaskGetTaskName(xTaskGetCurrentTaskHandle());
		strncpy(task_name, cur_task, 16);
		task_name[15] = 0;
		// tiT is the tcp/ip task - using udp during this task will cause issues so it is ommitted.
		if (strncmp(task_name, "tiT", 16) != 0){
			len = vsprintf((char*)buf, str, l);
			if( (err = sendto(sock, buf, len, 0, (struct sockaddr *)&serveraddr, sizeof(serveraddr))) < 0 )
			{
				print_error();
				vprintf("\nFreeing UDP Logging. sendto failed!\n", l);
				kill(l);
				return vprintf("UDP Logging freed!\n\n", l);
			}
		}
		return vprintf( str, l );
	}


	int init(){
		/**
		 * @brief Creates UDP socket which will then be attached to logger for UDP logging.
		 * 
		 */
		struct timeval send_timeout = {1,0};
		sock = 0;
		if( (sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		ESP_LOGE("UDP_LOGGING", "Cannot open socket!");
		return -1;
		}
		memset( &serveraddr, 0, sizeof(serveraddr) );
		serveraddr.sin_family = AF_INET;
		serveraddr.sin_port = htons( DEBUG_LOG_UDP_PORT );
		serveraddr.sin_addr.s_addr = (uint32_t)ip_addr;

		int err = setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char *)&send_timeout, sizeof(send_timeout));
		if (err < 0) {
		ESP_LOGE("UDP_LOGGING", "Failed to set SO_SNDTIMEO. Error %d", err);
		}

		esp_log_set_vprintf(send_log);
		return 0;
	}



}
