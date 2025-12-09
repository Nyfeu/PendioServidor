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

// Headers Principais do Projeto

#include <Arduino.h>
#include <HardwareSerial.h>
#include <EEPROM.h>

// Headers de Configuração do Projeto

#include "aplic.h"
#include "config.h"
#include "credentials.h"

// Headers de Comunicação

#include "CommunicationHandler.h"
#include "LoRaHandler.h"
#include "Logger.h"

//*****************************************************************************************
//  DEFINIÇÕES GLOBAIS, CONSTANTES E VARIÁVEIS
//*****************************************************************************************

// Interface Serial (Serial1 para o Módulo LoRa)
HardwareSerial loraSerial(1);

// Configuração do LoRa Handler
LoRaConfig loraConfig = {
    .serial = &loraSerial,
    .appEUI = (const uint8_t*)APPEUI,
    .appKey = (const uint8_t*)APPKEY,
    .useConfirmation = false,  // Será atualizado pela EEPROM
    .useADR = LORA_ADR_ON,
    .fixedDR = LORA_FIXED_DR,
    .joinTimeout = JOIN_TIMEOUT_VALUE,
    .confirmTimeout = CFM_TIMEOUT_VALUE,
    .maxRetries = 3
};

// Instância do handler de comunicação (pode ser trocada por WiFiHandler, etc)
LoRaHandler* commHandler = nullptr;

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

// ---------------------------------------------------------------------------
// Protótipos - funções auxiliares (assinam com as implementações abaixo)
// ---------------------------------------------------------------------------
void ToggleLed(void);
void exception_handling(int Exception_code);
uint8_t Validate_Cycle_Time(uint8_t ct);

// Ponteiro para a função de reset (software)
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
      LOGW("SYSTEM", "Error Code: %d", Exception_code);
      err_count++;
      // Caso o contador de erros exceder o limite de erros consecutivos, força reinício
      if (err_count > ERROR_MAX_SEQ) {
        LOGE("SYSTEM", "Forced Reset in 30s due to repeated LoRa errors");
        delay(30000);
        reset_function();
      }
      break;

    case RESTART_REQUEST:
      LOGW("SYSTEM", "Immediate Reset Requested - rebooting in 30s");
      delay(30000);
      reset_function();
      break;

    default:
      LOGW("SYSTEM", "Undefined Exception - ignored (%d)", Exception_code);
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
  // 1. Inicialização do Hardware Básico

  // Configura os pinos (HW.cpp)
  iniHW();                                   

  // Inicializa o LED da placa LoRaWAN
  pinMode(MODULE_LED_PIN,OUTPUT); 
  ToggleLed();

  // 2. Inicialização das Interfaces Seriais

  // Inicializa logger (Serial)
  Logger::begin(115200);
  
  // Comunicação UART para o módulo LoRa
  loraSerial.begin(9600, SERIAL_8N1, RXD1_LoRa, TXD1_LoRa);

  // Comunicação UART para os sensores SPendio (RS485)
  Serial2.begin(4800, SERIAL_8N1, RXD2_RS485, TXD2_RS485); // RS485
  Serial2.setRxBufferSize(64);
  Serial2.setTimeout(100);

  // 3. Inicialização dos Sensores I2C
  
  // Sensor AHT (Temp/Umid)
  if (!aht.begin()) LOGE("SENSOR", "AHT10/20 não encontrado. Verifique conexões.");
  else LOGI("SENSOR", "AHT10/20 detectado");

  // Sensor BMP (Pressão)
  if (!bmp.begin(END_BMP)) {
    LOGE("SENSOR", "BMP280 não encontrado");
    g_bBMPPresente = false;
  } else {
    LOGI("SENSOR", "BMP280 detectado");
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
  LOGI("SYSTEM", "=== PENDIO SERVIDOR - INICIANDO ===");
  LOGI("SYSTEM", "Versão: %s", Versao);
  LOGI("SYSTEM", "Data: %s", Data);
  
  // Inicializa estruturas de dados dos sensores
  iniSensores(CPendio_LoRa_Sensor_Data.d);

  // 5. Configuração do Handler de Comunicação
  LOGI("COMM", "Inicializando handler de comunicação...");
  LOGI("COMM", "Frame size: %u", (unsigned)sizeof(CPendio_LoRa_Sensor_Data));

  // Carrega informações da EEPROM
  #ifdef USE_EEPROM
    NVM_LoRaWAN_Cycle_Time = EEPROM.read(0);
    NVM_LoRaWAN_Use_Cfm = (NVM_SETTINGS_CFM_BIT == (EEPROM.read(1) & NVM_SETTINGS_CFM_BIT));
  #else
    NVM_LoRaWAN_Cycle_Time = 0;
    NVM_LoRaWAN_Use_Cfm = true;
  #endif
  NVM_LoRaWAN_Cycle_Time = Validate_Cycle_Time(NVM_LoRaWAN_Cycle_Time);

  // Atualizar configuração com valores da EEPROM
  loraConfig.useConfirmation = NVM_LoRaWAN_Use_Cfm;

  // Criar instância do handler LoRa
  commHandler = new LoRaHandler(loraConfig);

  // Inicializar handler
  if (!commHandler->begin()) {
    LOGE("COMM", "Falha ao inicializar handler de comunicação");
    while(1) { delay(1000); }
  }

  // Obter DevEUI
  char deveui[16];
  if (commHandler->getDevEUI(deveui)) {
    // Format DevEUI as hex string for readable log
    char hexstr[33];
    for (int i = 0; i < 16; ++i) {
      sprintf(&hexstr[i*2], "%02X", (uint8_t)deveui[i]);
    }
    hexstr[32] = '\0';
    LOGI("COMM", "DevEUI: %s", hexstr);
  }

  // Inicia JOIN
  delay(500);
  ToggleLed();
  LOGI("COMM", "Primeira tentativa de conexão à rede (JOIN)...");
  commHandler->connect();

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
  DownlinkMessage downlink;
  uint8_t port;

  timenow = millis();     // sample running time only here for all uses (including future calculations)
  if(((unsigned long)(timeout - timenow))>((unsigned long)(-timecycle)))                    // compare if time has come, but also during passage through zero (each ~49..50 days)
  {
    switch(State)
    {
      case STATE_NOT_JOINED:          // IF NOT JOINED YET...
#ifdef FAKE_JOIN
        if(true) {
#else
        if(commHandler->isConnected()) {
#endif
          if(!joined){ LOGI("COMM", "Joined network"); joined = true; }                        // print the "joined" message & set first message after Join to be sent
          State = STATE_READY;
        } else {
          LOGI("COMM", "Another attempt to Join the network");
          commHandler->connect();
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

        // Enviar dados através do handler de comunicação
        {
          LOGD("COMM", "Data payload (len=%u)", (unsigned)sizeof(CPendio_LoRa_Sensor_Data));

          SendResult sendResult = commHandler->send(1, (const uint8_t*)CPendio_LoRa_Sensor_Data.Bytes,
                                                    sizeof(CPendio_LoRa_Sensor_Data));

          if(sendResult == SendResult::SUCCESS) {
            State = STATE_WAIT_CFM;                                                           // Aguarda confirmação
            timecycle = CFM_TIMEOUT_VALUE;                                                    // After a message has been accepted, wait for some time.
            timenow = millis();                                                               // for TX resample running time
            LOGI("COMM", "Tx accepted (port=%d, len=%u)", 1, (unsigned)sizeof(CPendio_LoRa_Sensor_Data));
            exception_handling(ERROR_RESTART);                                                // Clear Error counter
          }
          else if(sendResult == SendResult::PENDING) {
            // Envio ainda pendente, manter estado
            LOGW("COMM", "Tx pending");
          }
          else {
            State = STATE_NOT_JOINED;                                                         // This should not happen... Go back to start
            timecycle = JOIN_TIMEOUT_VALUE;
            LOGE("COMM", "Tx denied - restarting join");
            exception_handling(ERROR_LORAWAN);
          }
        }
      break;
      case STATE_WAIT_CFM:                                                                  // After TX gets here to check what else to do
        if(true == NVM_LoRaWAN_Use_Cfm) {                                                   // If confirmation was expected...
          if (commHandler->isConfirmed()) {                                                 // ...and message has been confirmed...
            LOGI("COMM", "Acknowledgement received");
            exception_handling(ERROR_RESTART);                                              // Clear Error counter
          }
          else {
            LOGW("COMM", "No acknowledgement received");
            exception_handling(ERROR_LORAWAN);                                              // Otherwise report error
          }
          
          // Tentar ler mensagem downlink
          if(commHandler->receive(downlink) == ReceiveResult::MESSAGE_RECEIVED) {
            LOGI("COMM", "Rx message received (port=%u, len=%u)", (unsigned)downlink.port, (unsigned)downlink.length);
            
            // Processar downlink (exemplo hardcoded)
            if (downlink.length >= 5) {
              // Hardcoded evaluation of downlink messages... 
              if (downlink.data[0] == '8') {                                                // 0x8n - Hardcoded - downlink message to evaluate
                if ((downlink.data[1] == '0') && (downlink.data[4]==0x0)) {                // 0x81 0xCT - update of the LoRaWAN Cycle Time
                  NVM_LoRaWAN_Cycle_Time = (downlink.data[2]-'0')*16 + (downlink.data[3]-'0');
                  NVM_LoRaWAN_Cycle_Time = Validate_Cycle_Time(NVM_LoRaWAN_Cycle_Time);
                  LOGI("COMM", "New Cycle Time: %u", (unsigned)NVM_LoRaWAN_Cycle_Time);
                }
                if ((downlink.data[1] == '2') && (downlink.data[2]==0x0)) {                // 0x82 - Restart Request
                  exception_handling(RESTART_REQUEST); 
                }
                if ((downlink.data[1] == '4') && (downlink.data[4]==0x0)) {                // 0x84 0xNN - Confirmation required?
                  NVM_LoRaWAN_Use_Cfm = (NVM_SETTINGS_CFM_BIT == ((downlink.data[3]-'0') & NVM_SETTINGS_CFM_BIT));
                  LOGI("COMM", "New CFM: %s", (true == NVM_LoRaWAN_Use_Cfm) ? "true" : "false");
                }
              }
            }
            ToggleLed();                                                                    // Signal through LED message received
          } else {
            LOGI("COMM", "No downlink message arrived");
          }
          
          State = STATE_READY;                                                              // Go back to restart the whole process
          timecycle = 20000;                                                                // CCS - Hardcoded 20s
        } else {
          timecycle = NEXT_MSG_TIMEOUT_VALUE;                                               // ...next message in a shorter time
          LOGW("COMM", "No Ack - will retry");
          nack_count++;
          if (nack_count++ > LORA_MAX_NACK_RETRIES) {
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
