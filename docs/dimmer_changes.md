# Dimmer Changes
## Changelog

| Date  | Description |
|---|---|
| 02/09/22  | Document Creation  |
## Introduction
Updates to the code to enable brightness control for the 4 Channel Dimmer board. These changes are in addition to the 4 Channel LV changes and that document should be reviewed first.

## Switch Commands
Switch commands now accept an appended brightness value. If no brightness value is appended then the stored brightness will remain unchanged.

### Standard Switch Command Structure
```
Request_ID,Device_No,Token,Command
```

### Switch Command Structure with Brightness
```
Request_ID,Device_No,Token,CMD:Brightness
```

Where brightness is a value between 0 [Off] and 100 [Full Brightness]. 

### Example
Command that will turn ON Channel B to 75% brightness.
```
1234567891234,123456789,ABCD43F1,15:75
```

## Special Commands
An additional command was added to allow for bulk setting the brightness:

    0xF2 - Bulk Set Brightness.

### Bulk Set Brightness Structure
```
Request_ID,Device_No,Token,Command(F2),Params_JSON
```

### Example

```
1234567891234,123456789,ABCD43F1,F2,{"brightness": "100"}
```


## Response Examples
These responses build on the previous 4 Channel low voltage changes with the addition of including brightness values.

## MQTT Periodic Update
### Response Structure
```
Request_ID,Device_No,infodevice,Mac_Address,Status,Current_Usage,Relay_A_Clicks,Relay_B_Clicks,Relay_C_Clicks,Relay_D_Clicks,Relay_A_Brightness,Relay_B_Brightness,Relay_C_Brightness,Relay_D_Brightness,Active_Power
```
### Example
```
0000,123456789,infodevice,C1:C2:A1:C2:12:AB,33,0.000000,2,2,2,2,100,100,100,50,0
```

## Send Switch Status
### Response Structure
```
Request_ID,Device_No,Command,Status,Current_Usage,Relay_A_Clicks,Relay_B_Clicks,Relay_C_Clicks,Relay_D_Clicks,Relay_A_Brightness,Relay_B_Brightness,Relay_C_Brightness,Relay_D_Brightness,Active_Power
```
### Example
```
1234567891234,123456789,1F,33,0.000000,2,4,2,2,100,100,100,50,0
```

## Broadcast UDP
### Response Structure
```
Request_ID,Device_No,infodevice,Mac_Address,Status,Current_Usage,Relay_A_Clicks,Relay_B_Clicks,Relay_C_Clicks,Relay_D_Clicks,Relay_A_Brightness,Relay_B_Brightness,Relay_C_Brightness,Relay_D_Brightness,Active_Power
```
### Example
```
0000,123456789,infodevice,C1:C2:A1:C2:12:AB,33,0.000000,2,2,2,2,100,100,100,50,0
```

## Notes
### Brightness Thresholds
Brightness values of less than 5% and greather than 95% will be treated as 0% and 100% respecively. This is done to ensure that small delays in the switching time do not create flickering. This flickering occurs with the lower thresholds where the delay results in the start of the next half cycle being ON instead of only the last part of the previous half cycle.

### Timers & Schedules
Currently Timers and Schedules will just turn on the light at whatever the previous brightness value was.