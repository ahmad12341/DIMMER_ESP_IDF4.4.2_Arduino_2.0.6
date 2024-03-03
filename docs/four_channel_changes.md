# 4 Channel Updates

## Changelog

| Date  | Description |
|---|---|
| 22/08/22  | Document Creation  |
| 28/08/22  | Added Hard Timer C & D |
| 02/09/22  | Revised Reset Clicks Commands |

## Introduction

Allowing the firmware to support 4 channels required some changes to the command and response structure.

 
Switch commands were encoded into 8 bits with bits [5:4] being used for switch selection. The options for these two bits were **NO_SWITCH, SWITCH_A, SWITCH_B, SWITCH_AB**. As we required all the use of all 4 values of the 2 bits for the 4 channels **NO_SWITCH** & **SWITCH_AB** were removed. The switch selections are now **SWITCH_A, SWITCH_B, SWITCH_C, SWITCH_D**. Special commands **0xF0** & **0xF1** were added to support turning all relays ON and OFF respectively.

Switch status responses have been updated to include switch c & d clicks and the status byte has been updated to include Relay C & D Data.

## Switch Command
Switch command byte structure:

    [7-6][MSB]: Command Bits: [0 - Switch,  1 - Timer, 2 - Schedule, 3 - Config]
    [5-4]: Switch Select: [0 - Channel A, 1 - Channel B, 2 - Channel C, 3 - Channel D]
    [3-2]: Lock Mode Select: [0 - Lock Status, 1 - Lock Unlock, 2 - Lock Lock, 3 - Lock No Operation]
    [1-0][LSB]: Status Mode Select: [0 - Status Status, 1 - Status OFF, 2 - Status ON, 3 - Status TOGGLE]

For 4 Channel the switch select has been updated such that:

    0 (0b00): [NEW] SWITCH_A | [PREV] NO_SWITCH
    1 (0b01): [NEW] SWITCH_B | [PREV] SWITCH_A
    2 (0b10): [NEW] SWITCH_C | [PREV] SWITCH_B
    3 (0b11): [NEW] SWITCH_D | [PREV] SWITCH_AB

Example:

    0b00010101 | 0x15 -> Cmd: 0 : CMD_SWITCH | Switch: 1: SWITCH_B | Lock: 1: LOCK_UNLOCK | Status: 1: STATUS_OFF

### Command Request Structure
```
Request_ID,Device_No,Token,Command
```
### Example 
```
1234567891234,123456789,ABCD43F1,0E
```

## Status Byte Changes

The response for most switch commands includes a status byte which encodes the status of the various relays. The status byte previously only used the first nibble to encode the data with the rest remaining unused. To support 4 channels the entire status byte is now used:
```
[7][MSB]: Relay D Lock (0 - Unlocked | 1 - Locked)
[6]: Relay C Lock (0 - Unlocked | 1 - Locked)
[5]: Relay D status (0 - OFF | 1 - ON)
[4]: Relay C status (0 - OFF | 1 - ON)
[3]: Relay B Lock (0 - Unlocked | 1 - Locked
[2]: Relay A Lock (0 - Unlocked | 1 - Locked)
[1]: Relay B status (0 - OFF | 1 - ON)
[0][LSB]: Relay A status (0 - OFF | 1 - ON)
```

## Special Commands

Special commands for turning all relays ON & OFF were added to compensate for the removal of switch selection SWITCH_AB.

    0xF0 - All Relays OFF.
    0xF1 - All Relays ON.

### Command Request Structure
```
Request_ID,Device_No,Token,Command
```
### Example
```
1234567891234,123456789,ABCD43F1,F0
```

### Response Structure
```
Request_ID,Device_No,Command,Status_Byte,Current_Usage,Relay_A_Clicks,Relay_B_Clicks,Relay_C_Clicks,Relay_D_Clicks,Active_Power
```
### Example
```
1659700043793,da10ea386,F0,00,0.000000,2,2,1,1,0
```
## MQTT Periodic Update
### Response Structure
```
0000,Device_No,infodevice,Mac_Address,Status,Current_Usage,Relay_A_Clicks,Relay_B_Clicks,Relay_C_Clicks,Relay_D_Clicks,Active_Power
```
### Example
```
0000,123456789,infodevice,C1:C2:A1:C2:12:AB,33,0.000000,2,2,2,2,0
```

## Send Switch Status
### Response Structure
```
Request_ID,Device_No,Command,Status,Current_Usage,Relay_A_Clicks,Relay_B_Clicks,Relay_C_Clicks,Relay_D_Clicks,Active_Power
```
### Example
```
1234567891234,123456789,1F,33,0.000000,2,4,2,2,0
```

## Broadcast UDP
### Response Structure
```
0000,Device_No,infodevice,Mac_Address,Status,Current_Usage,Relay_A_Clicks,Relay_B_Clicks,Relay_C_Clicks,Relay_D_Clicks,Active_Power
```
### Example
```
0000,123456789,infodevice,C1:C2:A1:C2:12:AB,33,0.000000,2,2,2,2,0
```

## Config Responses
Commands 0xE0 and 0xE1 are configuration commands that return status values related to the channels.

### Response Structure
```
Request_ID,Device_No,Command,Current_Usage,Relay_A_Clicks,Relay_B_Clicks,Relay_C_Clicks,Relay_D_Clicks
```
### Example
```
1234567891234,123456789,E0,0.000000,0,0,0,0
```

## Commands Revisions

Several configuration commands are associated with the channels for obtaining various status values. To ensure that there remains room for expansion of the command structure several commands have been depreciated or reassigned.

    0xE6 [Depreciated] - Total Click Usage A (Current)
    0xE7 [Depreciated] - Total Click Usage A (Day)
    0xE8 [Re-assigned] - Click Usage A Reset.
    0xE9 [Depreciated] - Total Click Usage B (Current)
    0xEA [Depreciated] - Total Click Usage B (Day)
    0xEB [Re-assigned] - Click Usage B Reset.

The recommended method for obtaining these click values is to request either 0xE0 for "Current" data and 0xE1 for "Day" data. These will return the values for all 4 channels which can then be parsed for the desired value.

### New Commands

To ensure functional parity with Relays A and B some additonal commands were required. Additonally "Click Usage A Reset" & "Click Usage B Reset" were reassigned to ensure the commands were sequential.

    0xE6 - Click Usage A Reset.
    0xE7 - Click Usage B Reset.
    0xE8 - Click Usage C Reset.
    0xE9 - Click Usage D Reset.
    0xEA - [Unused]
    0xEB - [Unused]

### Response Structure
Commands **0xE6 - 0xE9** have the following response structure:
```
Request_ID,Device_No,CMD,Sucessful_Request 
```

### Example
```
1234567891234,123456789,E8,TRUE
```

## Setting Hard Timers

Hard timers are set via a command to the configuration. There is no change other than the addition of hardTimerC and hardTimerD. These values are also added to the system configuration with the names "hardTimerC", "hardTimerD".

### Command Structure

```
Request_ID,Device_No,Token,Command(C3),Params_JSON
```

### Example
```
1234567891234,123456789,ABCD43F1,C3,{"hardTimerC":"60"}
```

### Response Structure
```
Request_ID,Device_No,Command,Sucessful_Request
```
### Example
```
1234567891234,123456789,C3,TRUE
```
