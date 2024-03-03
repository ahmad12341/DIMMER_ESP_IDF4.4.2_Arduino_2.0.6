import serial
from config import SERIAL_PORT, BAUD_RATE, ERASE_COMMAND, FLASH_COMMAND, RESET_DEVICE_COMMAND, REQUEST_ID, DEVICE_NUMBER, USER_TOKEN, SERIAL_NUMBER, LOGIN_ANDROID_USER_GET_TOKEN_STR
import time
import subprocess
import json
import requests
import kvstore
from dataclasses import dataclass
import threading

from abc import ABC, abstractmethod
from enum import Enum
import logging

log = logging.getLogger('root')

db = kvstore.DBMStore('db/kv')

def read_serial_until(break_line: bytes = b"AP_START", timeout_sec=10) -> bytes:
    start_time = time.time()
    with serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1) as ser:
        while time.time() < start_time + timeout_sec:
            line = ser.readline()
            if line != b'':
                log.debug(line)
            if break_line in line:
                print(f"Break Line Found: {line}")
                return line
    raise TimeoutError(
        f"Unable to find expected log output with {timeout_sec} seconds."
    )


def read_serial_forever():
    with serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1) as ser:
        line = ser.readline()
        if line != b'':
            log.debug(line)

def start_serial_read_thread():
    read_thread = threading.Thread(target=read_serial_forever, daemon=True)
    read_thread.start()



def erase_flash() -> int:
    log.debug("Doing Clean Flash")
    command = ERASE_COMMAND.split(" ")
    log.debug(command)
    # Erase Flash
    process = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE)
    for line in process.stdout:
        log.debug(line)
    process.wait()
    return process.returncode


def flash_firmware():
    command = FLASH_COMMAND.split(" ")
    log.debug(command)
    # Flash Firmware
    process = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE)
    for line in process.stdout:
        log.debug(line)
    process.wait()
    return process.returncode


def device_reset():
    command = RESET_DEVICE_COMMAND.split(" ")
    log.debug(command)
    # Start the device main program from the start
    process = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE)
    for line in process.stdout:
        log.debug(line)
    process.wait()
    return process.returncode


def save_last_ssid(ssid: str):
    ''' Save the last detected SSID so we can ignore it and we don't have to wait for windows to finally update the wifi list.'''
    db.set('ssid', ssid)


def get_last_ssid() -> str:
    ''' Save the last detected SSID so we can ignore it and we don't have to wait for windows to finally update the wifi list.'''
    return db.get('ssid', fallback='')

def save_token(token: str):
    ''' Save the token which will be used to validate requests.'''
    db.set('token', token)

def get_token()->str:
    ''' Get the token to validate requests.'''
    return db.get('token', fallback='')

def decode_cmd(val: int):
    ''' Commands are in hex - This splits it into is values.'''
    cmd = (val & 0b11000000) >> 6
    switch = (val & 0b00110000) >> 4
    lock = (val & 0b00001100) >> 2
    status = (val & 0b00000011)
    print(f'Cmd: {cmd} | Switch: {switch} | Lock: {lock} | Status: {status}')
    return cmd, switch, lock, status


def hex_byte_as_binary_str(hex_str: str) -> str:
    return str(format(int(hex_str, 16), '#010b'))


def int_as_binary_str(val: int) -> str:
    return str(format(val, '#010b'))


def int_as_hex_str(val: int) -> str:
    return str(f'{val:0{2}x}').upper()

def login_android_set_token():
    '''
    Login as the android user and store the token in the KV store.
    '''
    res = requests.post(f'http://{SERIAL_NUMBER}', data=LOGIN_ANDROID_USER_GET_TOKEN_STR)
    if res.status_code != 200:
        raise ValueError(f'Android Login Request Failed - Status Code {res.status_code}')
    elif 'FALSE' in res.text:
        raise ValueError('Android Login Failed - Unexpected value [FALSE] in response.')
    if 'D1' not in res.text:
        raise ValueError('Android Login Failed - Unexpected value [COMMAND] in response.')
    token = res.text.split(',')[-1]
    db.set('android_token', token)
    return res

class CommandEnum(Enum):
    CMD_SWITCH = 0
    CMD_TIMER = 1
    CMD_SCHEDULER = 2
    CMD_CONFIG = 3

    @staticmethod
    def from_str(command_str:str):
        if(command_str == 'switch'):
            return CommandEnum.CMD_SWITCH
        elif(command_str == 'timer'):
            return CommandEnum.CMD_TIMER
        elif(command_str == 'scheduler'):
            return CommandEnum.CMD_SCHEDULER
        elif(command_str == 'config'):
            return CommandEnum.CMD_CONFIG


class SwitchEnum(Enum):
    SWITCH_A = 0
    SWITCH_B = 1
    SWITCH_C = 2
    SWITCH_D = 3

    @staticmethod
    def from_str(command_str:str):
        if(command_str == 'a'):
            return SwitchEnum.SWITCH_A
        elif(command_str == 'b'):
            return SwitchEnum.SWITCH_B
        elif(command_str == 'c'):
            return SwitchEnum.SWITCH_C
        elif(command_str == 'd'):
            return SwitchEnum.SWITCH_D

class LockEnum(Enum):
    LOCK_STATUS = 0
    LOCK_UNLOCK = 1
    LOCK_LOCK = 2
    LOCK_NO_OP = 3

    @staticmethod
    def from_str(command_str:str):
        if(command_str == 'lock'):
            return LockEnum.LOCK_LOCK
        elif(command_str == 'unlock'):
            return LockEnum.LOCK_UNLOCK
        elif(command_str == 'status'):
            return LockEnum.LOCK_STATUS
        else:
            return LockEnum.LOCK_NO_OP

class StatusEnum(Enum):
    STATUS_STATUS = 0
    STATUS_OFF = 1
    STATUS_ON = 2
    STATUS_TOGGLE = 3

    @staticmethod
    def from_str(command_str:str):
        if(command_str == 'toggle'):
            return StatusEnum.STATUS_TOGGLE
        elif(command_str == 'on'):
            return StatusEnum.STATUS_ON
        elif(command_str == 'off'):
            return StatusEnum.STATUS_OFF
        else:
            return StatusEnum.STATUS_STATUS


class TimerStatusEnum(Enum):
    '''
    Timers use default Status: 0 - OFF | 1 - ON

    NOTE: Not to be confused with Switch Status which is in a different format.
    '''
    STATUS_OFF = 0
    STATUS_ON = 1

class ResponseStatusEnum(Enum):
    STATUS_OFF = 0
    STATUS_ON = 1


@dataclass
class Relay:
    name: str
    status: int = 0
    lock: int = 0

@dataclass
class TimerScheduleDataABC(ABC):
    @abstractmethod
    def formatted(self):
        pass

@dataclass
class TimerData(TimerScheduleDataABC):
    '''
    Required data for timers to be encoded and paired with command request.
    '''
    timer_id: int = -1
    epoch: int = time.time()
    weekdays:str = 'NNNNNNN'
    repeats: int = 0
    duration_repeat:int = 0
    switch_id: int = SwitchEnum.SWITCH_A.value
    status: int = TimerStatusEnum.STATUS_ON.value
    duration_sec: int = 0

    def formatted(self):
        return f'{self.timer_id}:{self.epoch}:{self.weekdays}:{self.repeats}:{self.duration_repeat}:{self.switch_id}:{self.status}:{self.duration_sec}'


    def __eq__(self, other):
        '''
        Equality Dunder Method - Allows us to check if the timer is in the response (timer_id is ignored as they are just numbered).
        '''
        if isinstance(other, TimerData):
            for key, value in self.__dict__.items():
                if key != 'timer_id' and  str(other.__dict__[key]) != str(value):
                    return False
            return True
        return False


@dataclass
class ScheduleData(TimerScheduleDataABC):
    # 0:1660749965:NNNNNNN:-1:0:0:1:0
    # id:epoch:weekdays:repeats:duration_repeat:status:status:0
    schedule_id: int = -1
    epoch: int = time.time()
    weekdays:str = 'NNNNNNN'
    repeats: int = 0
    duration_repeat:int = 0
    switch_id: int = SwitchEnum.SWITCH_A.value
    status: int = TimerStatusEnum.STATUS_ON.value
    
    def formatted(self):
        return f'{self.schedule_id}:{self.epoch}:{self.weekdays}:{self.repeats}:{self.duration_repeat}:{self.switch_id}:{self.status}'

    def __eq__(self, other):
        '''
        Equality Dunder Method - Allows us to check if the timer is in the response (timer_id is ignored as they are just numbered).
        '''
        if isinstance(other, ScheduleData):
            for key, value in self.__dict__.items():
                if key != 'schedule_id' and  str(other.__dict__[key]) != str(value):
                    return False
            return True
        return False

class SwitchCmdResponse:
    '''
    Response from a Switch Command - Decodes status nibble as well as store status code.
    '''
    def __init__(self, response_str: str, status_code: int) -> None:
        self.status_code = status_code
        self.request_id, self.device_number, self.command, self.status_byte, self.current_usage, self.relay_a_clicks, self.relay_b_clicks, self.relay_c_clicks, self.relay_d_clicks, self.active_power = response_str.split(
            ',')
        self.relay_a, self.relay_b, self.relay_c, self.relay_d = self._decode_status_byte()

    def _decode_status_byte(self):
        val: int = int(self.status_byte, 16)
        relay_d_lock = (val & 0b10000000) >> 7
        relay_c_lock = (val & 0b01000000) >> 6
        relay_d_status = (val & 0b00100000) >> 5
        relay_c_status = (val & 0b0010000) >> 4
        relay_b_lock = (val & 0b1000) >> 3
        relay_a_lock = (val & 0b0100) >> 2
        relay_b_status = (val & 0b0010) >> 1
        relay_a_status = (val & 0b0001)
        return Relay('A', relay_a_status, relay_a_lock), Relay('B', relay_b_status, relay_b_lock), Relay('C', relay_c_status, relay_c_lock), Relay('D', relay_d_status, relay_d_lock)

class TimerCmdResponse:
    '''
    Response from a Timer Command - Parses Response and creates TimerData instance from response data.

    NOTE: Response sends back all current timers separated by commas.
    '''
    def __init__(self, response_str: str, status_code: int) -> None:
        self.status_code = status_code
        data: list = response_str.split(',', maxsplit=4)
        self.request_id, self.device_number, self.command, self.timers_str = data[0], data[1], data[2], data[3]
        self.timers_list = self.timers_str.split(',')
        self.timers_data = [TimerData(*timer.split(':')) for timer in self.timers_list]


class ScheduleCmdResponse:
    '''
    Response from a Timer Command - Parses Response and creates TimerData instance from response data.

    NOTE: Response sends back all current timers separated by commas.
    '''
    def __init__(self, response_str: str, status_code: int) -> None:
        self.status_code = status_code
        data: list = response_str.split(',', maxsplit=4)
        self.request_id, self.device_number, self.command, self.timers_str = data[0], data[1], data[2], data[3]
        print(self.timers_str)
        self.timers_list = self.timers_str.split(',')
        self.timers_data = [ScheduleData(*timer.split(':')) for timer in self.timers_list]

class Command(ABC):
    # TODO: Create general command creator.
    @abstractmethod
    def send_command(self):
        pass
    
    @abstractmethod
    def _format_request_str(self) -> str:
        pass

class SpecialCmd(Command):
    '''
    Moving to 4 Channels removed the ability to switch multiple relays with one command as the two bits were used by each of the
    4 channels. To resolve this some special commands where addded which use 0xF0 & 0xF1 to toggle all relays ON/OFF.

    '''
    def __init__(self, hex_str: str, token_key='custom_token') -> None:
        self.hex_str = hex_str
        self.token = db.get(token_key, fallback='')

    def _format_request_str(self) -> str:
        return f'{REQUEST_ID},{DEVICE_NUMBER},{self.token},{self.hex_str}'

    def send_command(self) -> SwitchCmdResponse:
        res = requests.post(f'http://{SERIAL_NUMBER}', data=self._format_request_str())
        print(res.text)
        return SwitchCmdResponse(res.text, res.status_code)

class CmdSwitch(Command):
    ''' 
    Command Class specifically for type Switch.

    Switch commands are encoded into 8 bits with:
    [7-6]: Command Bits
    [5-4]: Switch Select
    [3-2]: Lock Mode Select
    [1-0]: Status Mode Select

    Example:
        0b000101 | 0x15 -> Cmd: 0 : CMD_SWITCH | Switch: 1: SWITCH_B | Lock: 1: LOCK_UNLOCK | Status: 1: STATUS_OFF
    '''
    def __init__(self, cmd: CommandEnum, switch: SwitchEnum, lock: LockEnum, status: StatusEnum, token_key='custom_token') -> None:
        self.cmd = cmd # This should be CommandEnum.SWITCH
        self.switch = switch
        self.lock = lock
        self.status = status
        # Initalise empty int which will have bits shifted into it.
        self.output = 0
        self.output = (cmd.value << 6)
        self.output = self.output | (switch.value << 4)
        self.output = self.output | (lock.value << 2)
        self.output = self.output | status.value
        self.hex_str = int_as_hex_str(self.output)
        self.binary_str = int_as_binary_str(self.output)
        self.token = db.get(token_key, fallback='')
        log.debug(self.hex_str)
        log.debug(self.binary_str)


    def _format_request_str(self) -> str:
        return f'{REQUEST_ID},{DEVICE_NUMBER},{self.token},{self.hex_str}'

    def send_command(self) -> SwitchCmdResponse:
        res = requests.post(
            f'http://{SERIAL_NUMBER}', data=self._format_request_str())
        print(res.text)
        return SwitchCmdResponse(res.text, res.status_code)

class CmdTimer(Command):
    '''
    Command Class specific for Timers - Each command is encoded slightly different.

    Unlike Switch Commands the command appears to only encode the Command Type:
        CMD_TIMER = 1
        Shifted left 6 bits
        0b01000000 | 0x40 - No other data appears to be encoded in this byte.

    The switch selection, status etc is encoded with the timer data (see TimerData).
    
    Timer Command Format:
        Request_id,Device_Number,Token,CMD:timer_id:epoch:weekdays:repeats:repeat_duration:switch_id:status:duration
    '''
    def __init__(self, cmd: CommandEnum, timer_data: TimerData,  token_key='custom_token') -> None:
        self.cmd = cmd # This should be CommandEnum.TIMER
        self.output = (cmd.value << 6) # Just timer command is encoded.
        self.hex_str = int_as_hex_str(self.output)
        self.binary_str = int_as_binary_str(self.output)
        self.token = db.get(token_key, fallback='')
        self.timer_data = timer_data
        self.command_str = f'{self.hex_str}:{self.timer_data.formatted()}'

    def _format_request_str(self) -> str:
        return f'{REQUEST_ID},{DEVICE_NUMBER},{self.token},{self.command_str}'

    def send_command(self) -> TimerCmdResponse:
        res = requests.post(
            f'http://{SERIAL_NUMBER}', data=self._format_request_str())
        print(res.text)
        return TimerCmdResponse(res.text, res.status_code)


class CmdScheduler(Command):
    '''
    Command Class specific for Schedulers - Each command is encoded slightly different.

    Unlike Switch Commands the command appears to only encode the Command Type:
        CMD_TIMER = 2
        Shifted left 6 bits
        0b01000000 | 0x80 - No other data appears to be encoded in this byte.

    The switch selection, status etc is encoded with the timer data (see TimerData).
    
    Timer Command Format:
        Request_id,Device_Number,Token,CMD:timer_id:epoch:weekdays:repeats:repeat_duration:switch_id:status
    '''
    def __init__(self, cmd: CommandEnum, timer_data: ScheduleData,  token_key='custom_token') -> None:
        self.cmd = cmd # This should be CommandEnum.TIMER
        self.output = (cmd.value << 6) # Just timer command is encoded.
        self.hex_str = int_as_hex_str(self.output)
        self.binary_str = int_as_binary_str(self.output)
        self.token = db.get(token_key, fallback='')
        self.timer_data = timer_data
        self.command_str = f'{self.hex_str}:{self.timer_data.formatted()}'

    def _format_request_str(self) -> str:
        return f'{REQUEST_ID},{DEVICE_NUMBER},{self.token},{self.command_str}'

    def send_command(self) -> ScheduleCmdResponse:
        res = requests.post(
            f'http://{SERIAL_NUMBER}', data=self._format_request_str())
        print(res.text)
        return ScheduleCmdResponse(res.text, res.status_code)