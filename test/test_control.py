import pytest
import utils
from utils import CommandEnum, SwitchEnum, LockEnum, StatusEnum, ResponseStatusEnum, SwitchCmdResponse,TimerCmdResponse, TimerStatusEnum, ScheduleCmdResponse
import time
'''
Switch Commands Structure:
    Note the Request Data Structure:
        First Item: Request ID (hex_str)
        Second Item: Device Number (hex_str)
        Third Item: Token (hex_str)
        Fourth Item: Command (hex_str)

    The command is a byte that encodes various data:
        Bits[7,6](MSB): Command - [0: CMD_SWITCH, 1: CMD_TIMER, 2: CMD_SCHEDULER, 3: CMD_CONFIG]
        Bits[5,4]: Switch Selection - [0: SWITCH_A, 1: SWITCH_B, 2: SWITCH_C, 3: SWITCH_D]
        Bits[3,2]: Lock Status - [0: LOCK_STATUS, 1: LOCK_UNLOCK, 2: LOCK_LOCK, 3: LOCK_NO_OP]
        Bits[1,0]: Status - [0: STATUS_STATUS, 1: STATUS_OFF, 2: STATUS_ON, 3: STATUS_TOGGLE]

        Example:
            0b000101 | 0x15 -> Cmd: 0 : CMD_SWITCH | Switch: 1: SWITCH_A | Lock: 1: LOCK_UNLOCK | Status: 1: STATUS_OFF

        utils includes Enums and Command Class for creating valid commands.
'''

'''
NOTE: These tests must be run sequentially otherwise the various tests might interfere with the expected state of the device.
'''

@pytest.fixture(autouse=True, scope='module')
def setup_fixture(token_key):
    print('Turning OFF Both Relays Before Tests.')
    command = utils.SpecialCmd('F0', token_key=token_key)
    # command = utils.CmdSwitch( CommandEnum.CMD_SWITCH, SwitchEnum.SWITCH_A, LockEnum.LOCK_NO_OP, StatusEnum.STATUS_OFF, token_key=token_key)
    # response: SwitchCmdResponse = command.send_command()
    # command = utils.CmdSwitch(CommandEnum.CMD_SWITCH, SwitchEnum.SWITCH_B, LockEnum.LOCK_NO_OP, StatusEnum.STATUS_OFF, token_key=token_key)
    # response: SwitchCmdResponse = command.send_command()
    # command = utils.CmdSwitch(CommandEnum.CMD_SWITCH, SwitchEnum.SWITCH_C, LockEnum.LOCK_NO_OP, StatusEnum.STATUS_OFF, token_key=token_key)
    # response: SwitchCmdResponse = command.send_command()
    # command = utils.CmdSwitch(CommandEnum.CMD_SWITCH, SwitchEnum.SWITCH_D, LockEnum.LOCK_NO_OP, StatusEnum.STATUS_OFF, token_key=token_key)
    response: SwitchCmdResponse = command.send_command()
    if response.status_code != 200:
        raise ValueError('Unexpected Response Code from Both OFF relay fixture.')
    time.sleep(1)

@pytest.mark.order(1)
@pytest.mark.parametrize("switch, status_cmd, status_resp", [(SwitchEnum.SWITCH_A, StatusEnum.STATUS_ON, ResponseStatusEnum.STATUS_ON),
                                                                            (SwitchEnum.SWITCH_A, StatusEnum.STATUS_OFF, ResponseStatusEnum.STATUS_OFF),
                                                                            (SwitchEnum.SWITCH_B, StatusEnum.STATUS_ON, ResponseStatusEnum.STATUS_ON),
                                                                            (SwitchEnum.SWITCH_B, StatusEnum.STATUS_OFF, ResponseStatusEnum.STATUS_OFF),
                                                                            (SwitchEnum.SWITCH_C, StatusEnum.STATUS_ON, ResponseStatusEnum.STATUS_ON),
                                                                            (SwitchEnum.SWITCH_C, StatusEnum.STATUS_OFF, ResponseStatusEnum.STATUS_OFF),
                                                                            (SwitchEnum.SWITCH_D, StatusEnum.STATUS_ON, ResponseStatusEnum.STATUS_ON),
                                                                            (SwitchEnum.SWITCH_D, StatusEnum.STATUS_OFF, ResponseStatusEnum.STATUS_OFF),
                                                                            (SwitchEnum.SWITCH_A, StatusEnum.STATUS_TOGGLE, ResponseStatusEnum.STATUS_ON),
                                                                            (SwitchEnum.SWITCH_A, StatusEnum.STATUS_TOGGLE, ResponseStatusEnum.STATUS_OFF),
                                                                            (SwitchEnum.SWITCH_B, StatusEnum.STATUS_TOGGLE, ResponseStatusEnum.STATUS_ON),
                                                                            (SwitchEnum.SWITCH_B, StatusEnum.STATUS_TOGGLE, ResponseStatusEnum.STATUS_OFF),
                                                                            (SwitchEnum.SWITCH_C, StatusEnum.STATUS_TOGGLE, ResponseStatusEnum.STATUS_ON),
                                                                            (SwitchEnum.SWITCH_C, StatusEnum.STATUS_TOGGLE, ResponseStatusEnum.STATUS_OFF),
                                                                            (SwitchEnum.SWITCH_D, StatusEnum.STATUS_TOGGLE, ResponseStatusEnum.STATUS_ON),
                                                                            (SwitchEnum.SWITCH_D, StatusEnum.STATUS_TOGGLE, ResponseStatusEnum.STATUS_OFF)])
def test_relays_on_off(switch: SwitchEnum, status_cmd: StatusEnum, status_resp: ResponseStatusEnum, token_key):
    ''' 
    Sends ON/OFF commands for relay A & B over the local network
    '''
    command = utils.CmdSwitch(
        CommandEnum.CMD_SWITCH, switch, LockEnum.LOCK_NO_OP, status_cmd, token_key=token_key)
    response: SwitchCmdResponse = command.send_command()
    if switch == SwitchEnum.SWITCH_A:
         assert response.relay_a.status == status_resp.value
    elif switch == SwitchEnum.SWITCH_B:
         assert response.relay_b.status == status_resp.value
    elif switch == SwitchEnum.SWITCH_C:
         assert response.relay_c.status == status_resp.value
    elif switch == SwitchEnum.SWITCH_D:
         assert response.relay_d.status == status_resp.value
    assert response.command == command.hex_str
    assert response.status_code == 200
    time.sleep(0.5)

@pytest.mark.order(2)
def test_relay_get_status(token_key):
    ''' 
    Send OFF command - Then check state to ensure it is as expected.
    '''
    command = utils.CmdSwitch( CommandEnum.CMD_SWITCH, SwitchEnum.SWITCH_A, LockEnum.LOCK_NO_OP, StatusEnum.STATUS_OFF, token_key=token_key)
    response: SwitchCmdResponse = command.send_command()
    command = utils.CmdSwitch(CommandEnum.CMD_SWITCH, SwitchEnum.SWITCH_B, LockEnum.LOCK_NO_OP, StatusEnum.STATUS_OFF, token_key=token_key)
    response: SwitchCmdResponse = command.send_command()
    command = utils.CmdSwitch(CommandEnum.CMD_SWITCH, SwitchEnum.SWITCH_C, LockEnum.LOCK_NO_OP, StatusEnum.STATUS_OFF, token_key=token_key)
    response: SwitchCmdResponse = command.send_command()
    command = utils.CmdSwitch(CommandEnum.CMD_SWITCH, SwitchEnum.SWITCH_D, LockEnum.LOCK_NO_OP, StatusEnum.STATUS_OFF, token_key=token_key)
    response: SwitchCmdResponse = command.send_command()
    assert response.relay_a.status == ResponseStatusEnum.STATUS_OFF.value
    assert response.relay_b.status == ResponseStatusEnum.STATUS_OFF.value
    assert response.relay_c.status == ResponseStatusEnum.STATUS_OFF.value
    assert response.relay_d.status == ResponseStatusEnum.STATUS_OFF.value
    assert response.command == command.hex_str
    assert response.status_code == 200
    # After Off Command Successfully Applied - Do state check.
    command = utils.CmdSwitch(
        CommandEnum.CMD_SWITCH, SwitchEnum.SWITCH_A, LockEnum.LOCK_NO_OP, StatusEnum.STATUS_STATUS, token_key=token_key)
    response: SwitchCmdResponse = command.send_command()
    # This depends on previous commands
    assert response.relay_a.status == ResponseStatusEnum.STATUS_OFF.value
    assert response.relay_b.status == ResponseStatusEnum.STATUS_OFF.value
    assert response.relay_c.status == ResponseStatusEnum.STATUS_OFF.value
    assert response.relay_d.status == ResponseStatusEnum.STATUS_OFF.value
    assert response.command == command.hex_str
    assert response.status_code == 200
    time.sleep(0.5)


@pytest.fixture(scope='function')
def turn_off_relays(token_key):
    '''
    Turn OFF both relays before setting Timer.
    '''
    print('Turning OFF Both Relays Before Tests.')
    command = utils.CmdSwitch(CommandEnum.CMD_SWITCH, SwitchEnum.SWITCH_A, LockEnum.LOCK_NO_OP, StatusEnum.STATUS_OFF, token_key=token_key)
    response: SwitchCmdResponse = command.send_command()
    command = utils.CmdSwitch(CommandEnum.CMD_SWITCH, SwitchEnum.SWITCH_B, LockEnum.LOCK_NO_OP, StatusEnum.STATUS_OFF, token_key=token_key)
    response: SwitchCmdResponse = command.send_command()
    command = utils.CmdSwitch(CommandEnum.CMD_SWITCH, SwitchEnum.SWITCH_C, LockEnum.LOCK_NO_OP, StatusEnum.STATUS_OFF, token_key=token_key)
    response: SwitchCmdResponse = command.send_command()
    command = utils.CmdSwitch(CommandEnum.CMD_SWITCH, SwitchEnum.SWITCH_D, LockEnum.LOCK_NO_OP, StatusEnum.STATUS_OFF, token_key=token_key)
    response: SwitchCmdResponse = command.send_command()
    if response.status_code != 200:
        raise ValueError('Unexpected Response Code from Both OFF relay fixture.')
    time.sleep(1)

@pytest.mark.order(3)
@pytest.mark.parametrize("switch, status, response_status",   [(SwitchEnum.SWITCH_A, TimerStatusEnum.STATUS_ON, ResponseStatusEnum.STATUS_ON),
                                                                (SwitchEnum.SWITCH_B, TimerStatusEnum.STATUS_ON, ResponseStatusEnum.STATUS_ON),
                                                                (SwitchEnum.SWITCH_C, TimerStatusEnum.STATUS_ON, ResponseStatusEnum.STATUS_ON),
                                                                (SwitchEnum.SWITCH_D, TimerStatusEnum.STATUS_ON, ResponseStatusEnum.STATUS_ON),
                                                                (SwitchEnum.SWITCH_A, TimerStatusEnum.STATUS_OFF, ResponseStatusEnum.STATUS_OFF),
                                                                (SwitchEnum.SWITCH_B, TimerStatusEnum.STATUS_OFF, ResponseStatusEnum.STATUS_OFF),
                                                                (SwitchEnum.SWITCH_C, TimerStatusEnum.STATUS_OFF, ResponseStatusEnum.STATUS_OFF),
                                                                (SwitchEnum.SWITCH_D, TimerStatusEnum.STATUS_OFF, ResponseStatusEnum.STATUS_OFF)])
def test_set_timer_inital_state_off(turn_off_relays, switch, status, response_status,  token_key):
    '''
    Set Timer on Device and evaluate state before, during and after.

    NOTE: See Timer data and CmdTimer for details on how Timer commands are structured.

    Timer Request Structure:
        Request ID,Device Number,Token,CMD:TIMER_ID:EPOCH:WEEKDAYS:REPEATS:DURATION_REAPEAT:SWITCH_ID:STATUS:TIMER_SECONDS

        NOTE: Status is 0 - OFF | 1 - ON.
    '''
    epoch = int(time.time() + 2)
    timer = utils.TimerData(epoch=epoch, duration_sec=5, switch_id=switch.value,  status=status.value)
    command: utils.CmdTimer = utils.CmdTimer(CommandEnum.CMD_TIMER, timer_data=timer, token_key=token_key)
    response: TimerCmdResponse = command.send_command()
    assert response.status_code == 200
    assert command.timer_data in response.timers_data
    # Wait until Timer is triggered - Check Status.
    time.sleep(3)
    command = utils.CmdSwitch(CommandEnum.CMD_SWITCH, SwitchEnum.SWITCH_A, LockEnum.LOCK_NO_OP, StatusEnum.STATUS_STATUS, token_key=token_key)
    response: SwitchCmdResponse = command.send_command()
    assert response.status_code == 200
    if switch == SwitchEnum.SWITCH_A:
        assert response.relay_a.status == response_status.value
    elif switch == SwitchEnum.SWITCH_B:
        assert response.relay_b.status == response_status.value
    elif switch == SwitchEnum.SWITCH_C:
        assert response.relay_c.status == response_status.value
    elif switch == SwitchEnum.SWITCH_D:
        assert response.relay_d.status == response_status.value
    # Wait until Timer Ends - Check Status.
    time.sleep(6)
    command = utils.CmdSwitch(CommandEnum.CMD_SWITCH, SwitchEnum.SWITCH_A, LockEnum.LOCK_NO_OP, StatusEnum.STATUS_STATUS, token_key=token_key)
    response: SwitchCmdResponse = command.send_command()
    assert response.status_code == 200
    assert response.relay_a.status == ResponseStatusEnum.STATUS_OFF.value
    assert response.relay_b.status == ResponseStatusEnum.STATUS_OFF.value
    assert response.relay_c.status == ResponseStatusEnum.STATUS_OFF.value
    assert response.relay_d.status == ResponseStatusEnum.STATUS_OFF.value

    time.sleep(0.5)


@pytest.fixture(scope='function')
def turn_on_relays(token_key):
    '''
    Turn ON Both Relays before setting timers.
    '''
    print('Turning ON Both Relays Before Tests.')
    command = utils.CmdSwitch(CommandEnum.CMD_SWITCH, SwitchEnum.SWITCH_A, LockEnum.LOCK_NO_OP, StatusEnum.STATUS_ON, token_key=token_key)
    response: SwitchCmdResponse = command.send_command()
    command = utils.CmdSwitch(CommandEnum.CMD_SWITCH, SwitchEnum.SWITCH_B, LockEnum.LOCK_NO_OP, StatusEnum.STATUS_ON, token_key=token_key)
    response: SwitchCmdResponse = command.send_command()
    command = utils.CmdSwitch(CommandEnum.CMD_SWITCH, SwitchEnum.SWITCH_C, LockEnum.LOCK_NO_OP, StatusEnum.STATUS_ON, token_key=token_key)
    response: SwitchCmdResponse = command.send_command()
    command = utils.CmdSwitch(CommandEnum.CMD_SWITCH, SwitchEnum.SWITCH_D, LockEnum.LOCK_NO_OP, StatusEnum.STATUS_ON, token_key=token_key)
    response: SwitchCmdResponse = command.send_command()
    if response.status_code != 200:
        raise ValueError('Unexpected Response Code from Both OFF relay fixture.')
    time.sleep(1)

@pytest.mark.order(4)
@pytest.mark.parametrize("switch, status, response_status",   [(SwitchEnum.SWITCH_A, TimerStatusEnum.STATUS_ON, ResponseStatusEnum.STATUS_ON),
                                                                (SwitchEnum.SWITCH_B, TimerStatusEnum.STATUS_ON, ResponseStatusEnum.STATUS_ON),
                                                                (SwitchEnum.SWITCH_C, TimerStatusEnum.STATUS_ON, ResponseStatusEnum.STATUS_ON),
                                                                (SwitchEnum.SWITCH_D, TimerStatusEnum.STATUS_ON, ResponseStatusEnum.STATUS_ON),
                                                                (SwitchEnum.SWITCH_A, TimerStatusEnum.STATUS_OFF, ResponseStatusEnum.STATUS_OFF),
                                                                (SwitchEnum.SWITCH_B, TimerStatusEnum.STATUS_OFF, ResponseStatusEnum.STATUS_OFF),
                                                                (SwitchEnum.SWITCH_C, TimerStatusEnum.STATUS_OFF, ResponseStatusEnum.STATUS_OFF),
                                                                (SwitchEnum.SWITCH_D, TimerStatusEnum.STATUS_OFF, ResponseStatusEnum.STATUS_OFF)])
def test_set_timer_inital_state_on(turn_on_relays, switch, status, response_status,  token_key):
    '''
    Set a timer when the relay is already ON so on completion of the timer it should revert to its ON state.
    '''
    # Set Timer
    epoch = int(time.time() + 2)
    timer = utils.TimerData(epoch=epoch, duration_sec=5, switch_id=switch.value,  status=status.value)
    command: utils.CmdTimer = utils.CmdTimer(CommandEnum.CMD_TIMER, timer_data=timer, token_key=token_key)
    response: TimerCmdResponse = command.send_command()
    assert response.status_code == 200
    assert command.timer_data in response.timers_data
    # Wait until Timer is triggered - Check Status.
    time.sleep(3)
    command = utils.CmdSwitch(CommandEnum.CMD_SWITCH, SwitchEnum.SWITCH_A, LockEnum.LOCK_NO_OP, StatusEnum.STATUS_STATUS, token_key=token_key)
    response: SwitchCmdResponse = command.send_command()
    assert response.status_code == 200
    if switch == SwitchEnum.SWITCH_A:
         assert response.relay_a.status == response_status.value
    elif switch == SwitchEnum.SWITCH_B:
         assert response.relay_b.status == response_status.value
    elif switch == SwitchEnum.SWITCH_C:
         assert response.relay_c.status == response_status.value
    elif switch == SwitchEnum.SWITCH_D:
         assert response.relay_d.status == response_status.value
    # Wait until Timer Ends - Check Status.
    time.sleep(6) 
    command = utils.CmdSwitch(CommandEnum.CMD_SWITCH, SwitchEnum.SWITCH_A, LockEnum.LOCK_NO_OP, StatusEnum.STATUS_STATUS, token_key=token_key)
    response: SwitchCmdResponse = command.send_command()
    assert response.status_code == 200
    assert response.relay_a.status == ResponseStatusEnum.STATUS_ON.value
    assert response.relay_b.status == ResponseStatusEnum.STATUS_ON.value
    assert response.relay_c.status == ResponseStatusEnum.STATUS_ON.value
    assert response.relay_d.status == ResponseStatusEnum.STATUS_ON.value
    time.sleep(0.5)



@pytest.mark.order(5)
@pytest.mark.parametrize("switch, status, response_status",   [(SwitchEnum.SWITCH_A, TimerStatusEnum.STATUS_ON,ResponseStatusEnum.STATUS_ON),
                                                                (SwitchEnum.SWITCH_B, TimerStatusEnum.STATUS_ON, ResponseStatusEnum.STATUS_ON),
                                                                (SwitchEnum.SWITCH_C, TimerStatusEnum.STATUS_ON, ResponseStatusEnum.STATUS_ON),
                                                                (SwitchEnum.SWITCH_D, TimerStatusEnum.STATUS_ON, ResponseStatusEnum.STATUS_ON),
                                                                (SwitchEnum.SWITCH_A, TimerStatusEnum.STATUS_OFF, ResponseStatusEnum.STATUS_OFF),
                                                                (SwitchEnum.SWITCH_B, TimerStatusEnum.STATUS_OFF, ResponseStatusEnum.STATUS_OFF),
                                                                (SwitchEnum.SWITCH_C, TimerStatusEnum.STATUS_OFF, ResponseStatusEnum.STATUS_OFF),
                                                                (SwitchEnum.SWITCH_D, TimerStatusEnum.STATUS_OFF, ResponseStatusEnum.STATUS_OFF)])
def test_set_schedule_inital_state_on(turn_on_relays, switch, status, response_status,  token_key):
    epoch = int(time.time() + 2)
    timer = utils.ScheduleData(epoch=epoch, switch_id=switch.value, status=status.value)
    command: utils.CmdScheduler = utils.CmdScheduler(CommandEnum.CMD_SCHEDULER, timer_data=timer, token_key=token_key)
    response: ScheduleCmdResponse = command.send_command()
    assert response.status_code == 200
    assert command.timer_data in response.timers_data
    # Wait until Timer is triggered - Check Status.
    time.sleep(3)
    command = utils.CmdSwitch(CommandEnum.CMD_SWITCH, switch, LockEnum.LOCK_NO_OP, StatusEnum.STATUS_STATUS, token_key=token_key)
    response: SwitchCmdResponse = command.send_command()
    assert response.status_code == 200
    if switch == SwitchEnum.SWITCH_A:
         assert response.relay_a.status == response_status.value
    elif switch == SwitchEnum.SWITCH_B:
         assert response.relay_b.status == response_status.value
    elif switch == SwitchEnum.SWITCH_C:
         assert response.relay_c.status == response_status.value
    elif switch == SwitchEnum.SWITCH_D:
         assert response.relay_d.status == response_status.value
    # Channel should stay on as Schedules have no duration.
    time.sleep(0.5)