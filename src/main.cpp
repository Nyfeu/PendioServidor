/**********************************************************************************************************************************************************


Projeto Pendio - SW Release para campo
Eng. Nuncio Perrella, MSc
Data:  26 de  Abril 2026

Histórico de gravação:

Pendio 1  - Sistema de Testes POLI Civil - Kaiene
Pendio 2  - Caixa de testes - Geólogos _ Igor
Pendio 3  - Arnaldo
Pendio 4  - USP
PEndio 5  - A ser instalado
Pendio 6  - A ser instalado  (Teste Nuncio 14/11/2024)
Pendio 7  - Raia Olimpica USP
Pendio 8  - Raia Olimpica USP
Pendio 9  - Sensor  14/11/2024
 
 Linha 23 Arquivo Pendio_Lora_Wemos_Robocore.h --> Tempo de amostragem
 #define CFM_TIMEOUT_VALUE      180000      --> 180000 = 3 minutos




**********************************************************************************************************************************************************/






/*
  Pendio LoRa Robocore / Smart Modular Module Handling based on Wemos D1 R32 board
  2023-10-30 - Claudio Sonaglio
*/
// Libraries ----------------------------------------
/*
#include <RoboCore_SMW_SX1262M0.h>
//#include <SoftwareSerial.h>
#include <HardwareSerial.h>
#include <EEPROM.h>
#include <stdint.h>
#include "Pendio_Sensor.h"
#include "Pendio_LoRa_Wemos_Robocore_001.h"
*/






#define MAIN
// Naldo
#include "Aplic.h"

// Variables ----------------------------------------
//SoftwareSerial ss(5,23);
HardwareSerial ss(1);
SMW_SX1262M0 lorawan(ss);
CommandResponse response;
// Naldo
//Pendio_LoRa_Sensor_Data_Type Pendio_LoRa_Sensor_Data;             // Pendio LoRa + Sensor Data structure (union of vector + isolated variables for each sensor)
//TwoWire I2CAHT = TwoWire(0);
CPendio_LoRa_Sensor_Data_Type CPendio_LoRa_Sensor_Data;

uint8_t NVM_LoRaWAN_Cycle_Time = 0;                               // RAM variable to store the EEPROM Value stored in position 0
bool NVM_LoRaWAN_Use_Cfm = false;                                 // RAM variable to store the EEPROM Value stored in the LSbit of position 1

uint16_t Rx_Len;
uint16_t State = STATE_NOT_JOINED;                                // initialize State variable

// Kore Config:
//   DevEUI: 000516800010c95d - AppEUI: 91f26ac8101960cd - AppKey: f8cbc1a1cfe91d411a5e06a468b6e4b7
//   Ativação: OTAA - Critografia: NS - Tamanho Contador: 2 - Classe: A
//   Rx Window: AUTO - Validação de Contados: Inativo - Banda: LA915-928A - Modo ADR: ON
//Everynet Config:
//   DevEUI: 000516800010c95d - AppEUI: 91f26ac8101960cd - AppKey: f8cbc1a1cfe91d411a5e06a468b6e4b7
//   Tags: Pendio Nuncio Robocore - Activation: OTAA - Protocol Version: v1.0.3
//   Band: LA915-928A - ADR: OFF

// DEVEUI = 000516800010C813 - Taken from Read DevEUI @ shield Robocore in use - USE ALWAYS THE NATIVE <DEVEUI> FROM THE MODULE

//const char APPEUI[] = "91f26ac8101960cd";                             // Geovista Lora 1     Nuncio-      // AppEUI taken from the original code from Pendio - USE THE <APPEUI> FROM THE APPLICATION    
//const char APPEUI[] = "e66394a311509218";                             // Geovista Lora 2                  // AppEUI taken from the original code from Pendio - USE THE <APPEUI> FROM THE APPLICATION
//const char APPEUI[] = "404e5f22284448be";                             // Geovista Lora 3  Arnaldo         // AppEUI taken from the original code from Pendio - USE THE <APPEUI> FROM THE APPLICATION
//const char APPEUI[] = "0595fa4a769c7dc5 ";                            // Geovista Lora 4                  // AppEUI taken from the original code from Pendio - USE THE <APPEUI> FROM THE APPLICATION
//const char APPEUI[] = "b7eee2c39324aff9 ";                            // Geovista Lora 5                  // AppEUI taken from the original code from Pendio - USE THE <APPEUI> FROM THE APPLICATION
//const char APPEUI[] = "6eded0607cbf53d9";                             // Geovista Lora 6 Nuncio           // AppEUI taken from the original code from Pendio - USE THE <APPEUI> FROM THE APPLICATION
//const char APPEUI[] = "df74b1f32166838d";                             // Geovista Lora 7  Raia USP        // AppEUI taken from the original code from Pendio - USE THE <APPEUI> FROM THE APPLICATION
//const char APPEUI[] =  "703dbc674b94cd49";                            // Geovista Lora 8  Raia USP        // AppEUI taken from the original code from Pendio - USE THE <APPEUI> FROM THE APPLICATION
//const char APPEUI[] =  "47977eb3f7e148aa";                            // Geovista Lora 9                  // AppEUI taken from the original code from Pendio - USE THE <APPEUI> FROM THE APPLICATION
//const char APPEUI[] =  "49550531869e88a2";                            // Geovista Lora 10   13012025      // AppEUI taken from the original code from Pendio - USE THE <APPEUI> FROM THE APPLICATION
const char APPEUI[] =  "26e7cc9af428bec1";                                              // Geovista Lora 11   14012025      // AppEUI taken from the original code from Pendio - USE THE <APPEUI> FROM THE APPLICATION


//const char APPKEY[] = "f8cbc1a1cfe91d411a5e06a468b6e4b7";             // Geovista Lora 1  Nuncio          // AppKey taken from the original code from Pendio - USE THE <APPKEY> FROM THE APPLICATION
//const char APPKEY[] = "4b1f19db16953304b591fa5b8aaf4d49";             // Geovista Lora 2                  // AppKey taken from the original code from Pendio - USE THE <APPKEY> FROM THE APPLICATION
//const char APPKEY[] = "ff507ae40f58b99737875697836b1d2e";             // Geovista Lora 3  Arnaldo         // AppKey taken from the original code from Pendio - USE THE <APPKEY> FROM THE APPLICATION
//const char APPKEY[] = "de3ecca1db9df57bbfbb1e3a35949c3e";             // Geovista Lora 4                  // AppKey taken from the original code from Pendio - USE THE <APPKEY> FROM THE APPLICATION
//const char APPKEY[] = "9603df9ec74ef4fc0ded843fb6890ab9";             // Geovista Lora 5                  // AppKey taken from the original code from Pendio - USE THE <APPKEY> FROM THE APPLICATION
//const char APPKEY[] = "97c1f395db0955ef90ca5d30331d5151";             // Geovista Lora 6 Nunccio          // AppKey taken from the original code from Pendio - USE THE <APPKEY> FROM THE APPLICATION
//const char APPKEY[] = "921277b1f387e530ba365cddbb0a5a35";             // Geovista Lora 7 Raia USP         // AppKey taken from the original code from Pendio - USE THE <APPKEY> FROM THE APPLICATION
//const char APPKEY[] = "7ccfd7c7bc02911864f778187b191bd8";             // Geovista Lora 8 Raia USP         // AppKey taken from the original code from Pendio - USE THE <APPKEY> FROM THE APPLICATION
//const char APPKEY[] = "1facfc3ba59d9710067e99f206b3f3a1";             // Geovista Lora 9                  // AppKey taken from the original code from Pendio - USE THE <APPKEY> FROM THE APPLICATION
//const char APPKEY[] = "ee8c1ed049b073b184fd0fa840d11a45";             // Geovista Lora 10  13012025       // AppKey taken from the original code from Pendio - USE THE <APPKEY> FROM THE APPLICATION
const char APPKEY[] = "cfeebad46ac8638d69fa23c5789926f3";               // Geovista Lora 11  14012025       // AppKey taken from the original code from Pendio - USE THE <APPKEY> FROM THE APPLICATION


// Time variables -----------------------------------
unsigned long timeout, timenow, timecycle;
// LoRa variables -----------------------------------
bool joined = false;
int nack_count = 0;                                                 // Counter for messages Not Acknowledged
int err_count = 0;                                                  // Error Counter, used to trigger exception handling

// --------------------------------------------------
// - Simple Togle Led function using Led State variable
// --------------------------------------------------
int LedState = LOW;
void ToggleLed(void)
{
  LedState = !LedState;  digitalWrite(MODULE_LED_PIN,LedState);     // Toggle Module LED
}

// --------------------------------------------------
// - Error Handling Function
// --------------------------------------------------
void( *reset_function)(void) = 0;

void exception_handling(int Exception_code)
{
  switch (Exception_code)
  {
 /*
    case CLEAR_ERRORS:                      // Restart the error counter (done whenever it works correctly)
      err_count = 0;
    break;
*/
    case ERROR_RESTART:                      // Restart the error counter (done whenever it works correctly)
      err_count = 0;
    break;

    case ERROR_LORAWAN:                     // Process one additional LoRaWAN error, if they achive a limit (below), force a board reset
      Serial.print("Error on :");
      Serial.println(Exception_code);
      err_count++;
      if (err_count > ERROR_MAX_SEQ) { Serial.println("Forced Reset in 30s!!!"); delay(30000); reset_function(); }
    break;
    case RESTART_REQUEST:                   // This is reserved for a remote restart request using downlink message
      Serial.println("Immediate Reset Requested - in 30s!!!"); delay(30000); reset_function();
    break;
    default:                                // Exception unknown... will be ignored but also reported on log
      Serial.println("Undefined Exception - Ignore");
    break;
  }
}

// --------------------------------------------------
// - Validate (and Store) Cycle Time (BYTE 0 of EEPROM)
// --------------------------------------------------
uint8_t Validate_Cycle_Time(uint8_t ct)
{
  unsigned char ret;
  switch (ct)
  {
    case 0: ret = 1; break;                                            // when not using EEPROM, each 1 min (debug purposes)
    case 5: case 10: case 15: case 30: case 60: ret = ct; break;       // 5, 10, 15, 30 or 60 minutes, keep it
    default: ret = 15; break;                                          // any other value, standard 15 min.
  }
#ifdef USE_EEPROM
  EEPROM.update(0,ret);                                                // Cycle time must be store in EEPROM.
#endif
  return(ret);
}

// --------------------------------------------------
// - Validate (and Store) Settings (BYTE 1 of EEPROM)
// --------------------------------------------------
uint8_t Validate_Settings(uint8_t st)
{
  unsigned char ret = st;
#ifdef USE_EEPROM
  EEPROM.update(1,ret);                                                // Settings must be updated in EEPROM.
#endif
  return(ret);
}

// --------------------------------------------------
// - Startup function
// --------------------------------------------------
void setup() {
  char deveui[16];
  iniHW();                                    // Inicializa HW

// initialize Sensors
//  Pendio_Sensor_init();

// initialize LED, Monitor and LoRa serials
  pinMode(MODULE_LED_PIN,OUTPUT); ToggleLed();                      // Start LED from LoRaWAN Module
  // Naldo
  //Serial.begin(9600);                                               // Start the UART for debugging
  Serial.begin(115200);                                             // Start the UART for debugging
  //ss.begin(9600);                                                   // start the UART for the LoRaWAN module
  ss.begin(9600, SERIAL_8N1, RXD1_LoRa, TXD1_LoRa);
  Serial2.begin(4800, SERIAL_8N1, RXD2_RS485, TXD2_RS485); // RS485
  //  pinMode(RXD2_RS485, INPUT_PULLUP);                    // RXD2_RS485 pullup
  Serial2.setRxBufferSize(64);
  Serial2.setTimeout(100);

  if (! aht.begin()) {                                               // inicializa AHT
    Serial.println("Could not find AHT? Check wiring");
  }
  else {
    Serial.println("AHT10 or AHT20 found");
  }

  //  if (!bmp.begin(BMP_ADD)) {
  if (!bmp.begin(END_BMP)) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
    Serial.print("SensorID was: 0x"); Serial.println(bmp.sensorID(), 16);
    g_bBMPPresente = false;
  }
  else {
    Serial.println("BMP280 found");
    Serial.print("SensorID was: 0x"); Serial.println(bmp.sensorID(), 16);
    /* Default settings from datasheet. */
    bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                    Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                    Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                    Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                    Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
    g_bBMPPresente = true;
  }

  delay(1000);                                // tempo para começar a mostrar mensagens iniciais
  Serial.println("Iniciando...");

  Serial.println(Versao);                     // Mostra versão e data
  Serial.println(Data);
  iniSensores(CPendio_LoRa_Sensor_Data.d);    // Incializa Sensores

  lorawan.reset();                                                  // reset the module

 // lorawan.set_debugger(&Serial);                                    // When debug is needed

  Serial.println("Pendio Application Startup");
  Serial.println("==========================");
  Serial.print("LoRaWAN Frame size: ");
  // Naldo
  //Serial.println(sizeof(Pendio_Sensor_Data_Type));
  Serial.println(sizeof(CPendio_LoRa_Sensor_Data));

#ifdef USE_EEPROM
  NVM_LoRaWAN_Cycle_Time = EEPROM.read(0);                          // NVM_LoRaWAN_Cycle_Time located in the first EEPROM byte in memory (in min)
  NVM_LoRaWAN_Use_Cfm = (NVM_SETTINGS_CFM_BIT == (EEPROM.read(1) & NVM_SETTINGS_CFM_BIT));    // Boolean for the first bit of position 1
#else
  NVM_LoRaWAN_Cycle_Time = 0;                                       // NVM_LoRaWAN_Cycle_Time only 1 min for debug
  NVM_LoRaWAN_Use_Cfm = true;                                       // NVM_LoRaWAN_Use_Cfm can be changed here during debug
#endif
  NVM_LoRaWAN_Cycle_Time = Validate_Cycle_Time(NVM_LoRaWAN_Cycle_Time);

  Serial.println(F("--- SMW_SX1262M0 Uplink (OTAA) ---"));
  response = lorawan.reset();                                       // Resets the Module
  if(response == CommandResponse::OK){ Serial.println(F("Reset done")); }
    else { Serial.println(F("Error reseting the module")); }

  response = lorawan.get_DevEUI(deveui);                            // read the Device EUI
  if(response == CommandResponse::OK){ Serial.print(F("DevEUI: ")); Serial.write(deveui, 16); Serial.println(); }
    else { Serial.println(F("Error getting the Device EUI")); }

  response = lorawan.set_AppEUI(APPEUI);                            // set the Application EUI
  if(response == CommandResponse::OK){ Serial.print(F("Application EUI set (")); Serial.write(APPEUI, 16); Serial.println(')'); }
    else { Serial.println(F("Error setting the Application EUI")); }

  response = lorawan.set_AppKey(APPKEY);                            // set the Application Key
  if(response == CommandResponse::OK){ Serial.print(F("Application Key set (")); Serial.write(APPKEY, 32); Serial.println(')'); }
    else { Serial.println(F("Error setting the Application Key")); }

  response = lorawan.set_JoinMode(SMW_SX1262M0_JOIN_MODE_OTAA);     // set join mode to OTAA
  if(response == CommandResponse::OK){ Serial.println(F("Mode set to OTAA")); }
    else { Serial.println(F("Error setting the join mode")); }

  if (true == NVM_LoRaWAN_Use_Cfm) {
    response = lorawan.set_CFM(SMW_SX1262M0_CFM_ON);                // set Confirmation Mode ON
    if(response == CommandResponse::OK){ Serial.println(F("Confirmation Mode ON")); }
      else { Serial.println(F("Error setting confirmation mode on")); } }
  else {
    response = lorawan.set_CFM(SMW_SX1262M0_CFM_OFF);               // set Confirmation Mode OFF
    if(response == CommandResponse::OK){ Serial.println(F("Confirmation Mode OFF")); }
      else { Serial.println(F("Error setting confirmation mode off")); } }

#if LORA_ADR_ON
  response = lorawan.set_ADR(SMW_SX1262M0_ADR_ON);                  // set ADR = ON
  if(response == CommandResponse::OK){ Serial.println(F("Turn ADR on")); }
    else { Serial.println(F("Error enabling the ADR")); }
#else
  response = lorawan.set_ADR(SMW_SX1262M0_ADR_OFF);                 // set ADR = OFF
  if(response == CommandResponse::OK){ Serial.println(F("Turn ADR off")); }
    else { Serial.println(F("Error disabling the ADR")); }

  response = lorawan.set_DR(LORA_FIXED_DR);                         // set DR to default
  if(response == CommandResponse::OK){ Serial.print(F("Set DR to ")); Serial.println(LORA_FIXED_DR); }
    else { Serial.println(F("Error setting the datarate")); }
#endif

  response = lorawan.save();                                        // save the current configuration (optional)
  if(response == CommandResponse::OK){ Serial.println(F("Settings saved")); }
    else { Serial.println(F("Error on saving")); }

  delay(500); ToggleLed();
  Serial.println(F("First attempt to Join the network"));
  lorawan.join();                                                   // join the network
  timeout = millis() + JOIN_TIMEOUT_VALUE;                          // set timeout for Join process
  timecycle = JOIN_TIMEOUT_VALUE;                                   // set timecycle for later comparison
}

// --------------------------------------------------
// - Loop function
// --------------------------------------------------
//   Loop is oriented as follows:
//   - To sending of one LoRa message per execution
//   - There is a fixed number of LoRa Frames that need to be sent to cover all "registers".
//   - When sending each of such frames, wait for TIMER1 before going to the next register.
//   - When this is finished, wait for TIMER2 before start the whole process again
//   - Everytime this process starts, all Modbus registers are read in sequence to fill the Shared Table Memory
//
//   Timing:
//   - Each time a confirmed uplink is sent, the module waits 5..6s for the Ack
//   - If it doesn´t get, the module tried again (up to 8 times) to resend the same message. Each attempt takes in total 8s. So, it may take up to 64s to send one message.
//   - After all attempts it gives up on sending (lost message)
//   - If gets Ack, LoRaWAN Module internal state machine free to send more messages as soon as gets such Ack.
// --------------------------------------------------

void loop() {
  uint8_t x, byte;
// Naldo
//  char data[LORA_MSG_MAX_LEN];
  char data[16];
  uint8_t port;
  Buffer buffer;

  timenow = millis();                                                                       // sample running time only here for all uses (including future calculations)
  if(((unsigned long)(timeout - timenow))>((unsigned long)(-timecycle)))                    // compare if time has come, but also during passage through zero (each ~49..50 days)
  {
    switch(State)
    {
      case STATE_NOT_JOINED:          // IF NOT JOINED YET...
#ifdef FAKE_JOIN
        if(true) {
#else
        if(lorawan.isConnected()) {
#endif
          if(!joined){ Serial.println(F("Joined")); joined = true; }                        // print the "joined" message & set first message after Join to be sent
          State = STATE_READY;
        } else {
          Serial.println(F("Another attempt to Join the network"));
          lorawan.join();
        }
        timecycle = JOIN_TIMEOUT_VALUE;                                                     // Joined or not, wait the shortest time to start something
      break;
      case STATE_READY:               // IF ALREADY JOINED OR TX + RX COMPLETE...
        // Process Data Generation Functions (sensors read) = Here
        // Naldo
/*        
        Pendio_Sensor_angulo_sensores();
        Pendio_Sensor_temp_umid();
        Pendio_Sensor_bateria();
        Pendio_Sensor_impedancia_solo();
        Pendio_Sensor_chuva();
        Pendio_Sensor_barometro();
        for (x=0; x < sizeof(Pendio_Sensor_Data_Type); x++)
        {
          byte = Pendio_LoRa_Sensor_Data.Bytes[x];
          data[2*x] = Nib(byte/16);
          data[2*x+1] = Nib(byte%16);
        }
        data[2*x] = 0;
*/
        varrSensores(CPendio_LoRa_Sensor_Data.d);     // Varre Sensores
        nack_count = 0;
#if not LORA_ADR_ON 
        response = lorawan.set_DR(LORA_FIXED_DR);                                           // set DR to default
        if(response == CommandResponse::OK){ Serial.print(F("Set DR to ")); Serial.println(LORA_FIXED_DR); }
          else { Serial.println(F("Error setting the datarate")); }
#endif
// Naldo
//        Serial.print(F("Data: ")); Serial.println(data);
//        response = lorawan.sendX(1, data);                                                  // Try to send the message in the vector...
        Serial.print(F("Data: ")); Serial.println(CPendio_LoRa_Sensor_Data.Bytes);
        response = lorawan.sendX(1, CPendio_LoRa_Sensor_Data.Bytes);                          // Try to send the message in the vector...
        if(response == CommandResponse::OK) {
//          State = STATE_AFTER_SEND;                                                         // Goes to After Send State and check what to do there
          State = STATE_WAIT_CFM;                                                           // Goes to After Send State and check what to do there
          timecycle = CFM_TIMEOUT_VALUE;                                                    // After a message has been accepted, wait for some time.
          timenow = millis();                                                               // for TX resample running time
          Serial.println("Tx Accepted");                                                    // Log that Tx was accepted
//          exception_handling(CLEAR_ERRORS); }                                               // Clear Error counter
          exception_handling(ERROR_RESTART); }                                               // Clear Error counter
        else {
          State = STATE_NOT_JOINED;                                                         // This should not happen... It was supposed to be ready... Go back to start
          timecycle = JOIN_TIMEOUT_VALUE;                                                   // As if it was a Join process
          Serial.println("Tx Denied ERROR - Restart Join");
          exception_handling(ERROR_LORAWAN); }
      break;
//      case STATE_AFTER_SEND:                                                                // After TX gets here to check what else to do
      case STATE_WAIT_CFM:                                                                  // After TX gets here to check what else to do
        if(true == NVM_LoRaWAN_Use_Cfm) {                                                   // If confirmation was expected...
          if (lorawan.isConfirmed()) {                                                      // ...and message has been confirmed...
            Serial.println("Ack received");
//            exception_handling(CLEAR_ERRORS); }                                             // Clear Error counter
            exception_handling(ERROR_RESTART); }                                             // Clear Error counter
          else {
            Serial.println("No Ack received!");
            exception_handling(ERROR_LORAWAN); }                                            // Otherwise report error
          response = lorawan.readX(port, buffer);                                           // ...try to read an eventually incoming message - check if a downlink has also been received
          if(response == CommandResponse::OK) {
            if(buffer.available()){                                                         // the buffer cannot empty for a valid message
              Serial.print(F("Rx Message on port "));                                       // Message received!
              Serial.println(port);
              for(x=0; x<5; x++) {
                data[x] = buffer.read(); }
              // Hardcoded evaluation of downlink messages... 
              if (data[0] == '8') {                                                         // 0x8n - Hardcoded - downlink message to evaluate
                if ((data[1] == '0') && (data[4]==0x0)) {                                   // 0x81 0xCT - Hardcoded - update of the LoRaWAN Cycle Time
                  NVM_LoRaWAN_Cycle_Time = (data[2]-'0')*16 + (data[3]-'0');                // (if needed in Base64 gAU= 5min gg== Restart Request
                  Serial.print("New Cycle Time: ");
                  NVM_LoRaWAN_Cycle_Time = Validate_Cycle_Time(NVM_LoRaWAN_Cycle_Time);     // Validate and store new value
                  Serial.println(NVM_LoRaWAN_Cycle_Time); }
                if ((data[1] == '2') && (data[2]==0x0)) {                                   // 0x82 - Restart Request
                  exception_handling(RESTART_REQUEST); }
                if ((data[1] == '4') && (data[4]==0x0)) {                                   // 0x84 0xNN - Confirmation required? X1 = true else X0 = false
                  NVM_LoRaWAN_Use_Cfm = (NVM_SETTINGS_CFM_BIT == ((data[3]-'0') & NVM_SETTINGS_CFM_BIT));             // Set value of bit 0 of received Hex data
                  Serial.print("New CFM: ");
                  if (true == NVM_LoRaWAN_Use_Cfm) { Serial.println("true"); }
                  else { Serial.println("false"); } }
              }
              ToggleLed();                                                                  // Signal through LED message received
            }
          } else {
            Serial.println("No downlink message arrived until now");
          }
          State = STATE_READY;                                                              // Go back to restart the whole process
// CCS          timecycle = NVM_LoRaWAN_Cycle_Time * 60000;                                     // ...wait complete cycle time to next message
          timecycle = 20000;                                     // CCS - Hardcoded 20s
        } else {
          timecycle = NXTMSG_TIMEOUT_VALUE;                                                 // ...next message in a shorter time
          Serial.println("No Ack");
          nack_count++;
          if (nack_count++ > LORA_MAX_NACK) {
            nack_count = 0;
            State = STATE_READY;                                                            // Go back to restart the whole process
            exception_handling(ERROR_LORAWAN);
          }
        }
      break;
      default:
        State = STATE_NOT_JOINED;
        timecycle = JOIN_TIMEOUT_VALUE;                                                     // Joined or not, wait the shortest time to start something
        exception_handling(ERROR_LORAWAN);
      break;
    }
    timeout = timenow + timecycle;                                                          // update the timeout using timenow (since the start of processing) and timecycle
  }
}
