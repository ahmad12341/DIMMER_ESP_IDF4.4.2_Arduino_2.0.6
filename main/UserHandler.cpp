#include "UserHandler.h"

void user_handler_init()
{
	UUID_STORAGE = (struct uuid_tokens*)calloc(SYSTEM_CONFIGURATION.uuid_max_count, sizeof(struct uuid_tokens));
	files_manager_read_uuid_tokens();
}

void user_handler_expand_uuid_storage()
{
	SYSTEM_CONFIGURATION.uuid_max_count += 5;
	struct uuid_tokens *temp = (struct uuid_tokens*)calloc(SYSTEM_CONFIGURATION.uuid_max_count, sizeof(struct uuid_tokens));
	memcpy(temp, UUID_STORAGE, sizeof(struct uuid_tokens) * SYSTEM_CONFIGURATION.uuid_counter);
	
	free(UUID_STORAGE);
	UUID_STORAGE = temp;
}

void user_handler_shrink_uuid_storage()
{
	SYSTEM_CONFIGURATION.uuid_max_count -= 5;
	struct uuid_tokens *temp = (struct uuid_tokens*)calloc(SYSTEM_CONFIGURATION.uuid_max_count, sizeof(struct uuid_tokens));
	memcpy(temp, UUID_STORAGE, sizeof(struct uuid_tokens) * SYSTEM_CONFIGURATION.uuid_counter);
	
	free(UUID_STORAGE);
	UUID_STORAGE = temp;
}

int8_t user_handler_add_uuid(String id)
{
	if (SYSTEM_CONFIGURATION.uuid_counter == SYSTEM_CONFIGURATION.uuid_max_count)
	{
		user_handler_expand_uuid_storage();
	}
	
	for (int i = 0; i < SYSTEM_CONFIGURATION.uuid_counter; i++)
	{
		if (UUID_STORAGE[i].uuid == id)
		{
			return 0;
		}
	}
	
	UUID_STORAGE[SYSTEM_CONFIGURATION.uuid_counter].uuid = id;
	SYSTEM_CONFIGURATION.uuid_counter++;
	
	files_manager_write_uuid_tokens();
	files_manager_write_config();
	
	return 1;
}

int8_t user_handler_remove_uuid(String id)
{
	int8_t flag = 0;
	for (int i = 0; i < SYSTEM_CONFIGURATION.uuid_counter; i++)
	{
		if (UUID_STORAGE[i].uuid == id)
		{
			flag = 1;
		}
		
		if (flag && (i < (SYSTEM_CONFIGURATION.uuid_counter - 1)))
		{
			UUID_STORAGE[i].uuid = UUID_STORAGE[i + 1].uuid;
			UUID_STORAGE[i].token = UUID_STORAGE[i + 1].token;
		}
	}
	
	if (flag)
	{
		SYSTEM_CONFIGURATION.uuid_counter--;
		files_manager_write_uuid_tokens();
		files_manager_write_config();
	}
	
	if (SYSTEM_CONFIGURATION.uuid_counter < (SYSTEM_CONFIGURATION.uuid_max_count - 6) && SYSTEM_CONFIGURATION.uuid_max_count > 10)
	{
		user_handler_shrink_uuid_storage();
		files_manager_write_config();
	}
	
	return flag;
}

int32_t user_handler_uuid_login(String id)
{
	for (int i = 0; i < SYSTEM_CONFIGURATION.uuid_counter; i++)
	{
		if (UUID_STORAGE[i].uuid == id)
		{
			uint32_t epoch_time = rtc_get_epoch();
			if (!rtc_check_token_timeout(UUID_STORAGE[i].token_start_time))
			{
				#ifdef SERIAL_PRINT
				Serial.printf("Returning token: %d\n", UUID_STORAGE[i].token);
				#endif
				return UUID_STORAGE[i].token;
			}
			
			UUID_STORAGE[i].token = generate_token();
			UUID_STORAGE[i].token_start_time = epoch_time;
			#ifdef SERIAL_PRINT
			Serial.printf("token: %08X\t\ttoken_start_time = %d\n", UUID_STORAGE[i].token, UUID_STORAGE[i].token_start_time);
			#endif
			files_manager_write_uuid_tokens();
			return UUID_STORAGE[i].token;
		}
	}
	
	return 0;
}

void user_handler_loop()
{
	if (!rtc_init_flag || ((millis() - rtc_start_time) < 10000))
	{
		return;
	}
	
	for (int i = 0; i < SYSTEM_CONFIGURATION.uuid_counter; i++)
	{
		if (UUID_STORAGE[i].token != 0 && rtc_check_token_timeout(UUID_STORAGE[i].token_start_time))
		{			
			#ifdef SERIAL_PRINT
			Serial.print("Setting token to 0 for uuid ");
			Serial.println(UUID_STORAGE[i].uuid);
			#endif
			
			UUID_STORAGE[i].token = UUID_STORAGE[i].token_start_time = 0;
		}
	}
}

String user_handler_get_all_uuid()
{
	String ret = "{";
	if (SYSTEM_CONFIGURATION.uuid_counter == 0)
	{
		ret = "NO_UUIDS";
		return ret;
	}
	
	for (int i = 0; i < SYSTEM_CONFIGURATION.uuid_counter; i++)
	{
		ret += "\"id\":\"" + UUID_STORAGE[i].uuid + "\"";
		if (i < SYSTEM_CONFIGURATION.uuid_counter - 1)
		{
			ret += ",";
		}
	}
	
	ret += "}";
	return ret;
}

int8_t user_handler_validate_token(uint32_t token)
{
	for (int i = 0; i < SYSTEM_CONFIGURATION.uuid_counter; i++)
	{
		if(UUID_STORAGE[i].token !=0){
			log_d("User Token: %x", UUID_STORAGE[i].token);
		}
		if (token == UUID_STORAGE[i].token)
		{
			return 1;
		}
	}
	log_d("Invalid User Token: %lu", token);
	return 0;
}

int8_t user_handler_register(String parameters[], int16_t params_length)
{
	String id = "";
	String user = "";
	String pwd = "";
	for (int i = 0; i < params_length; i += 2)
	{
		if (parameters[i] == "id")
		{
			id = parameters[i + 1];
		}
		else if (parameters[i] == "u")
		{
			user = parameters[i + 1];
		}
		else if (parameters[i] == "p")
		{
			pwd = parameters[i + 1];
		}
	}
	
	if (user == SYSTEM_CONFIGURATION.deviceUser && pwd == SYSTEM_CONFIGURATION.devicePwd)
	{
		// Add new uuid
		log_d("Adding New User ID");
		return user_handler_add_uuid(id);
	}
	log_d("Failed Adding New User ID");
	return 0;
}

int32_t user_handler_login(String parameters[], int16_t params_length)
{
	String user = "";
	String pwd = "";
	String id = "";
	int8_t register_flag = 0;
	for (int i = 0; i < params_length; i += 2)
	{
		if (parameters[i] == "id")
		{
			id = parameters[i + 1];
		}
		else if (parameters[i] == "u")
		{
			user = parameters[i + 1];
		}
		else if (parameters[i] == "p")
		{
			pwd = parameters[i + 1];
		}
	}
	if (user == SYSTEM_CONFIGURATION.deviceUser && pwd == SYSTEM_CONFIGURATION.devicePwd)
	{
		int32_t token = user_handler_uuid_login(id);
		if(token)
		{
			return token;
		}
		else
		{
			return 0;
		}
	}
	
	return 0;
}

int8_t user_handler_deregister(String parameters[], int16_t params_length)
{
	String user = "";
	String pwd = "";
	String id = "";
	int8_t register_flag = 0;
	for (int i = 0; i < params_length; i += 2)
	{
		if (parameters[i] == "id")
		{
			id = parameters[i + 1];
		}
		else if (parameters[i] == "u")
		{
			user = parameters[i + 1];
		}
		else if (parameters[i] == "p")
		{
			pwd = parameters[i + 1];
		}
	}
	
	if (user == SYSTEM_CONFIGURATION.deviceUser && pwd == SYSTEM_CONFIGURATION.devicePwd)
	{
		user_handler_remove_uuid(id);
		return 1;
	}
	
	return 0;
}

int32_t generate_token()
{
	//randomSeed(millis());
	uint32_t ran_num = esp_random();
	uint32_t loop_max = millis() % 10000;
	for (int i = 0; i < loop_max; i++)
	{
		ran_num = esp_random();
	}
	
	return ran_num;
}