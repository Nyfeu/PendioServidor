/*
  Pendio LoRa Wemos + Robocore - Header File
  2023-10-30 - Claudio Sonaglio
*/
#ifndef PendioLoRaWemosRobocore001_h
#define PendioLoRaWemosRobocore001_h

#include <Arduino.h>

//Specific Module Related Defines (USE WEMOS D1 R32 BOARD!!!)
#define MODULE_LED_PIN               2                      // pin for Module Led (In this case here, Wemos D1 R32)

//Operational defines
//#define USE_EEPROM                                          // define here if EEPROM needs to be activated in final release
//#define FAKE_JOIN

//LoRaWAN Protocol related defines
#define JOIN_TIMEOUT_VALUE       10000                      // [ms] (10 s) wait this time before any new Join Attempt (5-6s normal)
//#define CFM_TIMEOUT_VALUE         8000                      // [ms] (8 s)  wait time for check if confirmation arrived (after Tx... 6s + 2s = 8s)

                                                            // Tempo de transmissão LORA

#define CFM_TIMEOUT_VALUE      180000                     // 3min[ms] (8 s)  wait time for check if confirmation arrived (after Tx... 6s + 2s = 8s)                                                           
//#define CFM_TIMEOUT_VALUE      1800000                      // 30min[ms] (8 s)  wait time for check if confirmation arrived (after Tx... 6s + 2s = 8s)

#define NXTMSG_TIMEOUT_VALUE     20000                      // [ms] (20 s) wait time for the next uplink message
//#define NXTMSG_TIMEOUT_VALUE     1800000                      // [ms] (20 s) wait time for the next uplink message
#define LORA_MSG_MAX_LEN           100                      // Max number of bytes in a LoRaWAN Payload (being Pos 0 the MsgID)
#define LORA_MAX_NACK                9                      // Max number of Nack attempts to restart next message

#define LORA_FIXED_DR                2                      // In case ADR is disabled, use fixed DR specified here
#define LORA_ADR_ON               true                      // Define if Adaptive Data Rate is in use

// Exception Handling defines
#define ERROR_MAX_SEQ                5                      // Max number of Unexplained errors before a restart
#define ERROR_RESTART                0                      // No Error, so restart counter
#define ERROR_LORAWAN                1                      // Error cause was in LoRaWAN
#define RESTART_REQUEST              2                      // Explicit Restart Request (immediate)

#define STATE_NOT_JOINED              0                     // During Startup and until Join Accept arrives
#define STATE_READY                   1                     // After Join Accept is received OR after TX + RX happened ok
#define STATE_WAIT_CFM                2                     // After Sent message, wait for confirmation

//Non Volatile Memory (EEPROM)
#define NVM_SETTINGS_CFM_BIT       0x01                     // Define for the first bit of Settings in EEPROM

//General use macros
#define Nib(x)      ((x > 9)?('A' + x - 0xa):('0' + x))

//Data Types
// DANGER!!! Max LoRaWAN message size (depending on LNS and device) can be considerably lower than 256...
// Always be aware tha the size of this structure will define if data can be sent in only one message or split in parts
/* Naldo
typedef union _Pendio_LoRa_Sensor_Data_Type {
    unsigned char Bytes[sizeof(Pendio_Sensor_Data_Type)];
    Pendio_Sensor_Data_Type d;
} Pendio_LoRa_Sensor_Data_Type;

extern Pendio_LoRa_Sensor_Data_Type Pendio_LoRa_Sensor_Data;
*/

#endif // PendioLoRaWemosRobocore001_h
