# Documentação do Hardware 
## Mapeamento de Pinos (GPIO)

Este documento descreve a ligação dos pinos (GPIOs) do ESP32 (Wemos D1 R32) para os periféricos do sistema Pendio.

A informação foi extraída de `include/WRCPendio.h`.

| Pino (GPIO) | Nome no Código | Periférico | Descrição |
|:---:|---|---|---|
| 16 | `RXD2_RS485` | RS485 | RX da Serial2 para comunicação com sensores SPendio |
| 17 | `TXD2_RS485` | RS485 | TX da Serial2 para comunicação com sensores SPendio |
| 27 | `nRE` | RS485 | Controle do Transceiver RS485 (Habilita Recepção, Ativo Baixo) |
| 19 | `pDE` | RS485 | Controle do Transceiver RS485 (Habilita Transmissão, Ativo Alto) |
| 5 | `RXD1_LoRa` | LoRaWAN | RX da Serial1 para o módulo Robocore LoRaWAN |
| 23 | `TXD1_LoRa` | LoRaWAN | TX da Serial1 para o módulo Robocore LoRaWAN |
| 22 | `pSCL_SHTU` | I2C | SCL (Clock) para sensores AHT, BMP |
| 22 | `pSDA_SHTU` | I2C | SDA (Dados) para sensores AHT, BMP |
| 4 | `nChuva` | Sensor Chuva | Pino de entrada para o sensor de chuva (contato seco) | 
| 39 | `aVBat` | Bateria | Entrada Analógica (ADC) para medição da tensão da bateria | 
| 2 | `WLED` | LED | LED integrado na placa Wemos |
| 18 | `LLED` | LED | LED na placa Robocore LoRaWAN | 

## Endereços I2C 

| Periférico | Endereço | Nome no Código | 
|---|---|---|
|BMP280 | `0x76` | `END_BMP` |

---








