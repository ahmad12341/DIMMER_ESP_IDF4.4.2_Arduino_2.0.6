'''
Includes tests for complete setup of the device mimicing what the android app does.

NOTE: Setting up this way will make the android app have invalid credentials as we create a custom user. 
        If just testing control use the normal android setup and then just log that user in using "test_android.py".
'''
import pywifi
import time
import logging
import pytest
import utils
from config import SETUP_REQUEST_STR, SETUP_USER_REQUEST_STR, LOGIN_USER_GET_TOKEN_STR, SERIAL_NUMBER, LOGIN_ANDROID_USER_GET_TOKEN_STR, SETUP_ANDROID_USER_REQUEST_STR
import requests
import os

log = logging.getLogger('root')


@pytest.fixture(scope="session", autouse=True)
def wifi_fixture() -> pywifi.PyWiFi:
    wifi = pywifi.PyWiFi()
    return wifi


@pytest.fixture(scope="session")
def iface_fixture(wifi_fixture) -> pywifi.iface.Interface:
    iface = wifi_fixture.interfaces()[0]
    return iface


@pytest.fixture(scope="session")
def profile_fixture() -> pywifi.Profile:
    profile = pywifi.Profile()
    profile.auth = pywifi.const.AUTH_ALG_OPEN
    profile.akm.append(pywifi.const.AKM_TYPE_WPA2PSK)
    profile.cipher = pywifi.const.CIPHER_TYPE_CCMP
    profile.key = "automation"
    return profile


def filter_wifi_scan(iface, prev_ssid) -> list:
    ''' We need to scan until only the current AP is found. This appears to be a windows issue with not clearing Wifi List fast enough'''
    time.sleep(5) # Give it enough time to scan properly.
    detected_aps = iface.scan_results()
    infitnite_ssids = {ap.ssid for ap in detected_aps if ap.ssid.startswith("infinite_") and ap.ssid != prev_ssid}
    log.debug(infitnite_ssids)
    return list(infitnite_ssids)



def test_wifi_ap_exists(iface_fixture: pywifi.iface.Interface, profile_fixture: pywifi.Profile):
    iface_fixture.scan() # Calling Scan multiple Time cause failure?
    time.sleep(5) # Wait to ensure AP has started
    prev_ssid = utils.db.get('ssid', fallback='')
    infitnite_ssids = filter_wifi_scan(iface_fixture, prev_ssid)
    while len(infitnite_ssids) > 1 or len(infitnite_ssids) == 0:
        # Scan until we only get one AP - Windows doesn't update list fast enough sometimes..
        log.warning("Multiple AP detected - Rescanning until only one found")
        infitnite_ssids = filter_wifi_scan(iface_fixture, prev_ssid)
        time.sleep(1)
    assert "infinite_" in infitnite_ssids[0]
    ssid = infitnite_ssids[0]
    utils.db.set('ssid', ssid)
    log.debug(f"SSID: {ssid}")
    profile_fixture.ssid = ssid


def test_wifi_ap_connect(iface_fixture: pywifi.iface.Interface, profile_fixture: pywifi.Profile):
    print(f"SSID Connect: {profile_fixture.ssid}")
    iface_fixture.disconnect()
    iface_fixture.remove_all_network_profiles()
    profile = iface_fixture.add_network_profile(profile_fixture)
    iface_fixture.connect(profile)
    while iface_fixture.status() != pywifi.const.IFACE_CONNECTED:
        log.debug(f"Inteface Status: {iface_fixture.status()}")
        log.debug("Connecting to AP...")
        time.sleep(1)
    assert iface_fixture.status() == pywifi.const.IFACE_CONNECTED
    log.debug("Connected!")

@pytest.mark.flaky(reruns=2)
@pytest.mark.dependency(name="custom_user")
def test_device_register_custom_user_request():
    '''
    NOTE: This has to be done in AP mode. | This also might not be needed. 
    The token user registered to which then needs to be logged in to generate a token.
    
    Process Requirements:
    CMD_COMFIG = 3
    CMD_BYTE = 0xD0 | 0b11010000

    CMD_BYTE Converts into command:
        cmd = 0b11 = 3 = CMD_CONFIG
        switch = 0b01
        lock = 0b00
        status = 0b00
        cmd_i_value = 0xD0 
    
    Required Parameters Are:
        id: uid str # Some string to identify user id - This is associated with token
        u: deviceUser
        p: devicePwd
    
    Which means the whole request data should be (Request_ID and Device_Number not required while in AP mode):
        CMD_BYTE,{"id": "uid", "u": "{DEVICE_ID}", "p": "{DEVICE_PWD}"}

    NOTE: Requires Device_ID and DEVICE_PWD to be configured. Which requires C3 Request First
    '''
    time.sleep(15) # Wait long enough for stable connection
    res = requests.post('http://192.168.4.1', data=SETUP_USER_REQUEST_STR)
    log.debug(res.text)
    assert res.status_code == 200
    assert "TRUE" in res.text

@pytest.mark.dependency(name='wifi_connected')
def test_wifi_sta_setup_request():
    ''' 
    Get the device to connect to the local Wifi instead of AP mode - This includes MQTT details etc
    NOTE: This does not include the token required for requets that needs to be handled.
    '''
    time.sleep(2)
    res = requests.post('http://192.168.4.1', data=SETUP_REQUEST_STR)
    assert res.text == 'default,C3,TRUE' # Successful Request
    assert res.status_code

@pytest.mark.flaky(reruns=2)
@pytest.mark.dependency(depends=["wifi_connected", "custom_user"])
def test_wifi_get_custom_user_token_request():
    '''
    After User is registered and wifi is connected we can get a token.

    NOTE: Setting up this way make app not work as it has different UID
    
    Command:
        D1 for loggin in.
    Request:
        route: http://{SERIAL_NUMBER} | 
        data str: {REQUEST_ID},{DEVICE_NUMBER},D1, {id: {USER_ID}, "u": {DEVICE_USER}, "p": {DEVICE_PWD}}
    '''
    time.sleep(10)
    res = requests.post(f'http://{SERIAL_NUMBER}', data=LOGIN_USER_GET_TOKEN_STR)
    log.debug(res.text)
    # This Response Includes the TOKEN
    assert res.status_code == 200
    assert "D1" in res.text
    assert "FALSE" not in res.text
    token = res.text.split(',')[-1]
    utils.db.set('custom_token', token)
