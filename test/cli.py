from email.policy import default
from attr import mutable
import click
import utils
import multiprocessing
import time
from logger import setup_custom_logger
from utils import CommandEnum, SwitchEnum, LockEnum, StatusEnum, CmdSwitch
log = setup_custom_logger('root')

@click.group()
def cli():
    pass


@cli.command()
def erase_flash():
    click.echo(f"Erased Flash Status: {utils.erase_flash()}")


@cli.command()
def flash_firmware():
    click.echo(f"Flash Status: {utils.flash_firmware()}")

@cli.command()
@click.option('--cmd', default=None, help='Command Hex string you wish to decode.')
def decode_cmd(cmd):
    '''\b
    Decode command from hex string into:
        Command Type | Switch Selection | Lock Mode Selection | Status Mode Selection
    
    '''
    if cmd is None:
        click.echo(f'No Command Provided.')
    cmd_int = int(cmd, 16)
    click.echo(f"Command Binary:{utils.hex_byte_as_binary_str(cmd)}")
    cmd_val, switch, lock, status = utils.decode_cmd(cmd_int)
    click.echo(f'Cmd: {cmd_val} | Switch: {switch} | Lock: {lock} | Status: {status}')

@cli.command()
@click.option('--cmd', default='switch', help='Command Type', show_default=True, type=str)
@click.option('--switch', default='none', help='Switch Selection', show_default=True, type=str)
@click.option('--lock', default='none', help='Lock Mode Selection', show_default=True, type=str)
@click.option('--status', default='none', help='Status Mode Selection', show_default=True, type=str)
def generate_command(cmd, switch, lock, status):
    '''\b
    Generate the hex and binary for a command from text inputs:

        Command Types = swtich, timer, scheduler, config

        Switch Selections = a, b, ab, no_switch[default]

        Lock Mode Selections = lock, unlock, status, no_op[default]

        Status Mode Selections = toggle, on, off, status[default]
    '''
    cmd_parse = CommandEnum.from_str(cmd)
    switch_parse = SwitchEnum.from_str(switch)
    lock_parse = LockEnum.from_str(lock)
    status_parse = StatusEnum.from_str(status)
    command = utils.CmdSwitch(cmd_parse, switch_parse, lock_parse, status_parse)
    click.echo(f"Command Hex: {command.hex_str}")
    click.echo(f"Command Binary: {command.binary_str}")
    cmd_val, switch_val, lock_val, status_val = utils.decode_cmd(command.output)
    click.echo(f'Cmd: {cmd_val} | Switch: {switch_val} | Lock: {lock_val} | Status: {status_val}')


@cli.command()
@click.option('--cmd', default=None, help='Command Hex string you wish to send.')
@click.option('--token_key', default='android_token', help='Token Selection - Must have performed either Android or Custom user login.')
def send_cmd(cmd, token_key):
    '''\b
    Send a hex string encoded command to the device.
        Command: 8 bit hex string that encodes the command parameters.
        Token Key: Token selection (custom_token, android_token) based on what setup processes you followed.
    '''
    cmd_int = int(cmd, 16)
    cmd_val, switch, lock, status = utils.decode_cmd(cmd_int)
    click.echo(f'Cmd: {cmd_val} | Switch: {switch} | Lock: {lock} | Status: {status}')
    command = CmdSwitch(CommandEnum(cmd_val), SwitchEnum(switch), LockEnum(lock), StatusEnum(status), token_key=token_key)
    command.send_command()

@cli.command()
def login_android():
    '''\b
    Log in the Android User and aquire a token which will be used to validate requests.
    '''
    res = utils.login_android_set_token()
    click.echo(f'Android Log Response: {res.status_code}')

if __name__ == "__main__":
    cli()
