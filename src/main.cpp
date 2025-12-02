/* Projeto Pendio (Monitoramento de Taludes) **********************************************

Software Release para Campo (Produção)
Eng. Nuncio Perrella, MSc
Data:  26 de  Abril 2025  

******************************************************************************************/

/**
 * @file main.cpp
 * @brief Firmware principal do sistema de monitoramento Pendio (ESP32 + LoRaWAN).
 * @details Gerencia a máquina de estados, leitura de sensores e telemetria.
 * @author Eng. Nuncio Perrella, MSc
 * @copyright Copyright (c) 2025
 */

#define MAIN

/* Informações adicionais *****************************************************************

  - Os dados são enviados via LoRa a cada 3 minutos (180000 s),
    conforme definido em CFM_TIMEOUT_VALUE.

  - Pendio LoRa Robocore / Smart Modular Module Handling based on Wemos D1 R32 board
    2023-10-30 - Claudio Sonaglio

******************************************************************************************/

// Headers Principais do Projeto

#include <Arduino.h>
#include <HardwareSerial.h>
#include <EEPROM.h>
#include <RoboCore_SMW_SX1262M0.h>

// Headers de Configuração do Projeto

#include "aplic.h"
#include "config.h"
#include "credentials.h"

//*****************************************************************************************
//  DEFINIÇÕES GLOBAIS, CONSTANTES E VARIÁVEIS
//*****************************************************************************************

// Interface Serial (Serial1 para o Módulo LoRa)
HardwareSerial loraSerial(1);           

// Instância do driver LoRaWAN
SMW_SX1262M0 lorawan(loraSerial);       
CommandResponse response;               

// Estrutura de Dados dos Sensores (Definida em Sensores.h/Aplic.h)
CPendio_LoRa_Sensor_Data_Type CPendio_LoRa_Sensor_Data;

// RAM variable to store the EEPROM Value stored in position 0
uint8_t NVM_LoRaWAN_Cycle_Time = 0;                              

// RAM variable to store the EEPROM Value stored in the LSbit of position 1
bool NVM_LoRaWAN_Use_Cfm = false;       

// Variáveis de Controle de Tempo
unsigned long timeout   = 0;
unsigned long timenow   = 0;
unsigned long timecycle = 0;

// Variáveis de Controle LoRa
bool joined     = false;
int nack_count  = 0;              // Contador de não-confirmações (NACK)
int err_count   = 0;              // Contador de exceções sequenciais

// Variável de Estado do LED
int LedState = LOW;

/* Estados da Máquina de Estados Principal ---------------------------------------*/

// Definição dos estados
enum SystemState {
    STATE_NOT_JOINED = 0,          // Aguardando conexão com a rede (Join Accept)
    STATE_READY,                   // Conectado, pronto para leitura e envio
    STATE_WAIT_CFM                 // Pacote enviado, aguardando ACK/Downlink
};

// Inicializa a variável de estado
uint16_t State = STATE_NOT_JOINED; 

/* Configurações de Erro e Retentativa -------------------------------------------*/
constexpr int ERROR_RESTART   = 0; // Limpa erros (reinicia o contador)
constexpr int ERROR_LORAWAN   = 1; // Erro de comunicação no LoRaWAN
constexpr int RESTART_REQUEST = 2; // Solicitação remota de reinício (imediata)
constexpr int ERROR_MAX_SEQ   = 5; // Máximo de erros antes do reset forçado

//*****************************************************************************************
//  PROTÓTIPOS DE FUNÇÕES AUXILIARES
//*****************************************************************************************

// Funções auxiliares
void toggleLed();
void handleException(int exceptionCode);
uint8_t validateCycleTime(uint8_t timeInMinutes);

// Ponteiro para a função reset por software
void (*reset_function)(void) = 0;

//*****************************************************************************************
//  IMPLEMENTAÇÃO
//*****************************************************************************************
                                                
/**
 * @brief Alterna o estado do LED de status.
 */
void ToggleLed(void) {

  LedState = !LedState; 
  digitalWrite(MODULE_LED_PIN,LedState);

}

/**
 * @brief Gerencia erros críticos e reinícios do sistema.
 * @param Exception_code Código do erro (ERROR_RESTART, ERROR_LORAWAN, etc).
 */
void exception_handling(int Exception_code) {

  switch (Exception_code) {
    case ERROR_RESTART:
      // Sucesso: Zera contador de erros
      err_count = 0;
      break;

    case ERROR_LORAWAN:
      // Processa um erro adicional LoRaWAN
      Serial.print("Error Code: ");
      Serial.println(Exception_code);
      err_count++;
      // Caso o contador de erros exceder o limite de erros consecutivos, força reinício
      if (err_count > ERROR_MAX_SEQ) { 
        Serial.println("Forced Reset in 30s!!!"); 
        delay(30000); 
        reset_function(); 
      }
      break;

    case RESTART_REQUEST:
      Serial.println("Immediate Reset Requested - in 30s!!!"); delay(30000); reset_function();
      break;

    default:
      Serial.println("Undefined Exception - Ignore");
      break;

  }

}

/**
 * @brief Valida o tempo de ciclo lido da EEPROM.
 * @param ct Tempo em minutos.
 * @return uint8_t Tempo validado.
 */
uint8_t Validate_Cycle_Time(uint8_t ct) {

  unsigned char ret;
  switch (ct) {

    case 0: 
      ret = 1; // Modo Debug (1 min)
      break;

    case 5: 
    case 10: 
    case 15: 
    case 30: 
    case 60: 
      ret = ct; // Valores válidos
      break;
      
    default: 
      ret = 15; // Padrão (15 min)
      break;

  }

  #ifdef USE_EEPROM
    EEPROM.update(0,ret); // Cycle time must be store in EEPROM.
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
  EEPROM.update(1,ret); // Settings must be updated in EEPROM.
#endif
  return(ret);
}

//*****************************************************************************************
//  SETUP
//*****************************************************************************************
void setup() {
  char deveui[16];

  // 1. Inicialização do Hardware Básico

  // Configura os pinos (HW.cpp)
  iniHW();                                   

  // Inicializa o LED da placa LoRaWAN
  pinMode(MODULE_LED_PIN,OUTPUT); 
  ToggleLed();

  // 2. Inicialização das Interfaces Seriais

  // Comunicação UART para DEBUG
  Serial.begin(115200);
  
  // Comunicação UART para o módulo LoRa
  loraSerial.begin(9600, SERIAL_8N1, RXD1_LoRa, TXD1_LoRa);

  // Comunicação UART para os sensores SPendio (RS485)
  Serial2.begin(4800, SERIAL_8N1, RXD2_RS485, TXD2_RS485); // RS485
  Serial2.setRxBufferSize(64);
  Serial2.setTimeout(100);

  // 3. Inicialização dos Sensores I2C
  
  // Sensor AHT (Temp/Umid)
  if (!aht.begin()) Serial.println("[ERRO] AHT10/20 não encontrado. Verifique conexões.");
  else Serial.println("[INFO] AHT10/20 detectado.");

  // Sensor BMP (Pressão)
  if (!bmp.begin(END_BMP)) {
    Serial.println("[ERRO] BMP280 não encontrado.");
    g_bBMPPresente = false;
  } else {
    Serial.println("[INFO] BMP280 detectado.");
    // Configuração padrão de acordo com o datasheet
    bmp.setSampling(
      Adafruit_BMP280::MODE_NORMAL,     // Operating Mode. 
      Adafruit_BMP280::SAMPLING_X2,     // Temp. oversampling 
      Adafruit_BMP280::SAMPLING_X16,    // Pressure oversampling 
      Adafruit_BMP280::FILTER_X16,      // Filtering. 
      Adafruit_BMP280::STANDBY_MS_500   // Standby time. 
    ); 
    g_bBMPPresente = true;
  }

  // Delay para estabilização
  delay(1000);

  // 4. Mensagem de Boas-vindas
  Serial.println(F("\n=== PENDIO SERVIDOR - INICIANDO ==="));
  Serial.print(F("Versão: ")); Serial.println(Versao);
  Serial.print(F("Data:   ")); Serial.println(Data);
  
  // Inicializa estruturas de dados dos sensores
  iniSensores(CPendio_LoRa_Sensor_Data.d);

  // 5. Configuração LoRaWAN
  Serial.println(F("[LoRa] Inicializando módulo..."));
  lorawan.reset();                                                  
  //lorawan.set_debugger(&Serial); // quando debug é necessário -> substituir por macro (diretiva)
  Serial.print("[LoRa] Frame size: ");
  Serial.println(sizeof(CPendio_LoRa_Sensor_Data));

  // Carrega informações da EEPROM
  #ifdef USE_EEPROM
    NVM_LoRaWAN_Cycle_Time = EEPROM.read(0);                          // NVM_LoRaWAN_Cycle_Time located in the first EEPROM byte in memory (in min)
    NVM_LoRaWAN_Use_Cfm = (NVM_SETTINGS_CFM_BIT == (EEPROM.read(1) & NVM_SETTINGS_CFM_BIT));    // Boolean for the first bit of position 1
  #else
    NVM_LoRaWAN_Cycle_Time = 0;                                       // NVM_LoRaWAN_Cycle_Time only 1 min for debug
    NVM_LoRaWAN_Use_Cfm = true;                                       // NVM_LoRaWAN_Use_Cfm can be changed here during debug
  #endif
  NVM_LoRaWAN_Cycle_Time = Validate_Cycle_Time(NVM_LoRaWAN_Cycle_Time);

  // Configuração e Join
  Serial.println(F("--- SMW_SX1262M0 Uplink (OTAA) ---"));
  response = lorawan.reset();

  // Reset Lógico
  if(response == CommandResponse::OK){ Serial.println(F("[INFO] Reset OK")); }
    else { Serial.println(F("[ERRO] Falha no Reset")); }

  // Leitura DevEUI
  response = lorawan.get_DevEUI(deveui);
  if(response == CommandResponse::OK){ Serial.print(F("[INFO] DevEUI: ")); Serial.write(deveui, 16); Serial.println(); }
    else { Serial.println(F("[ERRO] Error getting the Device EUI")); }

  // Configura Aplicação EUI
  response = lorawan.set_AppEUI(APPEUI);
  if(response == CommandResponse::OK){ Serial.print(F("[INFO] Application EUI set (")); Serial.write(APPEUI, 16); Serial.println(')'); }
    else { Serial.println(F("[ERRO] Error setting the Application EUI")); }

  // Configura a Application Key (APPKEY)
  response = lorawan.set_AppKey(APPKEY);
  if(response == CommandResponse::OK){ Serial.print(F("[INFO] Application Key set (")); Serial.write(APPKEY, 32); Serial.println(')'); }
    else { Serial.println(F("[ERRO] Error setting the Application Key")); }

  // Configura o JoinMode para OTAA
  response = lorawan.set_JoinMode(SMW_SX1262M0_JOIN_MODE_OTAA);
  if(response == CommandResponse::OK){ Serial.println(F("[INFO] Mode set to OTAA")); }
    else { Serial.println(F("[ERRO] Error setting the join mode")); }

  // Configura Confirmação LoRaWAN
  if (true == NVM_LoRaWAN_Use_Cfm) {

    // Configura a Confirmação para MODE ON
    response = lorawan.set_CFM(SMW_SX1262M0_CFM_ON);
    if(response == CommandResponse::OK) Serial.println(F("[INFO] Confirmation Mode ON"));
    else Serial.println(F("[ERRO] Error setting confirmation mode on"));
  
  } else {

    // Configura a Confirmação para MODE OFF  
    response = lorawan.set_CFM(SMW_SX1262M0_CFM_OFF);
    if(response == CommandResponse::OK) Serial.println(F("[INFO] Confirmation Mode OFF"));
    else Serial.println(F("[ERRO] Error setting confirmation mode off"));
  
  }

  // Configura ADR (Adaptive Data Rate)
  #if LORA_ADR_ON

    // Configura ADR para MODE ON - ATIVO
    response = lorawan.set_ADR(SMW_SX1262M0_ADR_ON);
    if(response == CommandResponse::OK) Serial.println(F("[INFO] Turn ADR on"));
    else Serial.println(F("[ERRO] Error enabling the ADR"));

  #else

    // Configura ADR para MODE OFF - DESABILITADO
    response = lorawan.set_ADR(SMW_SX1262M0_ADR_OFF);
    if(response == CommandResponse::OK) Serial.println(F("[INFO] Turn ADR off"));
    else Serial.println(F("[ERRO] Error disabling the ADR"));

    // Configura ADR para Data Rate Fixo (FIXED_DR) DEFAULT
    response = lorawan.set_DR(LORA_FIXED_DR);
    if(response == CommandResponse::OK){ 
      Serial.print(F("[INFO] Set DR to ")); 
      Serial.println(LORA_FIXED_DR); 
    } else Serial.println(F("[ERRO] Error setting the datarate"));

  #endif

  // Salva as configurações do LoRaWAN (opcional)
  response = lorawan.save();
  if(response == CommandResponse::OK){ Serial.println(F("[INFO] Settings saved")); }
  else Serial.println(F("[ERRO] Error on saving"));

  // Inicia JOIN
  delay(500); 
  ToggleLed();
  Serial.println(F("[INFO] Primeira tentativa de conexão à rede (JOIN)..."));
  lorawan.join();

  // Define TIMERS iniciais
  timeout = millis() + JOIN_TIMEOUT_VALUE; // Timeout para o processo de Join
  timecycle = JOIN_TIMEOUT_VALUE;          // Timecycle para comparação posterior

}

//*****************************************************************************************
//  LOOP
//*****************************************************************************************
/*  

  Loop orientado da seguinte forma:
  - Enviar uma mensagem LoRa por execução.
  - Existe um número fixo de quadros LoRa (Frames) que precisam ser enviados
    para cobrir todos os "registradores".
  - Ao enviar cada um desses quadros, aguardar o TIMER1 antes de avançar
    para o próximo registrador.
  - Quando isso terminar, aguardar o TIMER2 antes de iniciar todo o processo novamente.
  - Sempre que o processo iniciar, todos os registradores Modbus são lidos em sequência
    para preencher a Tabela de Memória Compartilhada (Shared Table Memory).

  Temporização:
  - Cada vez que um uplink confirmado é enviado, o módulo aguarda 5..6s pelo Ack.
  - Se não receber, o módulo tenta novamente (até 8 vezes) reenviar a mesma mensagem.
    Cada tentativa leva no total 8s. Portanto, pode levar até 64s para enviar uma mensagem.
  - Após todas as tentativas, o módulo desiste do envio (mensagem perdida).
  - Se receber Ack, a máquina de estados interna do módulo LoRaWAN fica livre para
    enviar mais mensagens assim que o Ack for recebido.

******************************************************************************************/
void loop() {
  uint8_t x, byte;
  char data[16];          // Buffer auxiliar para leitura de downlink
  uint8_t port;
  Buffer buffer;

  timenow = millis();     // sample running time only here for all uses (including future calculations)
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
          if(!joined){ Serial.println(F("[INFO] Joined")); joined = true; }                        // print the "joined" message & set first message after Join to be sent
          State = STATE_READY;
        } else {
          Serial.println(F("[INFO] Another attempt to Join the network"));
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
