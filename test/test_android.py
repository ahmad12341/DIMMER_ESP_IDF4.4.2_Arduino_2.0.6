'''
Test the device after it has been setup via the android app.

This assumes that the android app was used to setup the device.
'''
import pytest
import requests
import logging
import time
import utils
from config import LOGIN_ANDROID_USER_GET_TOKEN_STR, SERIAL_NUMBER
log = logging.getLogger('root')


@pytest.mark.flaky(reruns=2)
def test_login_android_user_get_token(delay_sec):
    '''
    Android setup should have created an android user on the device. We now use the credentials to login that user.

    This will return a token which is used to validate future requests.
    
    Command:
        D1 for loggin in.
    Request:
        route: http://{SERIAL_NUMBER} | 
        data str: {REQUEST_ID},{DEVICE_NUMBER},D1, {id: {USER_ID}, "u": {DEVICE_USER}, "p": {DEVICE_PWD}}
    '''
    time.sleep(delay_sec)
    res = utils.login_android_set_token()
    log.debug(res.text)
    assert res.status_code == 200
    assert "D1" in res.text
    assert "FALSE" not in res.text