from logger import setup_custom_logger

log = setup_custom_logger('root')

import subprocess
import pytest
import utils
import time

def pytest_addoption(parser):
    parser.addoption("--clean", action="store", default='False')
    parser.addoption("--restart", action="store", default='False')
    parser.addoption("--token_key", action="store", default='android_token', type=str)
    parser.addoption("--delay_sec", action="store", default=30, type=int)


def pytest_generate_tests(metafunc):
    # This is called for every test. Only get/set command line arguments
    # if the argument is specified in the list of test "fixturenames".
    option_value = metafunc.config.option.clean
    if "clean" in metafunc.fixturenames and option_value is not None:
        metafunc.parametrize("clean", [option_value])

    option_value = metafunc.config.option.restart
    if "restart" in metafunc.fixturenames and option_value is not None:
        metafunc.parametrize("restart", [option_value])

    option_value = metafunc.config.option.token_key
    if "token_key" in metafunc.fixturenames and option_value is not None:
        metafunc.parametrize("token_key", [option_value], scope='session')

    option_value = metafunc.config.option.delay_sec
    if "delay_sec" in metafunc.fixturenames and option_value is not None:
        metafunc.parametrize("delay_sec", [option_value])
        

def pytest_sessionstart(session):
    cmd = "putty -serial COM3 -sercfg 115200,8,n,1,N".split(" ")
    restart = session.config.getoption("--restart")
    if restart != 'True':
        utils.db.set('ssid','') # Last SSID is now Valid Since we didnt setup a new one
        process = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE)
        return
    clean = session.config.getoption("--clean")
    if clean != 'True':
        utils.device_reset()
    else:
        # If clean argument passed to a clean install of the firmware.
        utils.erase_flash()
        utils.flash_firmware()
    delay_sec = session.config.getoption("--delay_sec")
    process = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE)
    time.sleep(delay_sec) # Wait enough time for SPIFFS to be configured.
