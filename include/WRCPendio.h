#ifndef _WCPENDIO_H
#define _WCPENDIO_H

//-------------------------------------------------------------------------------------------
//   WCPendio.h - Wemos-D1-R32/Robocore LoRaWAN ESP32 GPIOs
//
#define RXD2_RS485  16              // Serial2 RS485
#define TXD2_RS485  17
#define WLED         2              // WLED Led Wemos
#define nRE         27              // Pinos nRE RS485 - Habilita Recepção
#define pDE         19              // Pinos pDE RS485 - Habilita Transmissão
#define LLED        18              // LLED Led Robocore LoRaWAN
#define pSCL_SHTU   22              // SCL Sensor temp/umidade
#define pSDA_SHTU   21              // SDA Sensor temp/umidade
#define nChuva       4
#define RXD1_LoRa    5              // Serial1 Robocore LoRaWAN
#define TXD1_LoRa   23
#define aVBat       39              // tensão da bateria

#define END_BMP    0x76             // Endereço I2C BMP280

//------------------------------------------------------------------------------
//  Macros
//
#define ligWLED() digitalWrite(WLED, HIGH )
#define desWLED() digitalWrite(WLED, LOW)
#define invWLED() digitalWrite(WLED, !digitalRead(WLED))

#define ligLLED() digitalWrite(LLED, HIGH )
#define desLLED() digitalWrite(LLED, LOW)
#define invLLED() digitalWrite(LLED, !digitalRead(LLED))

#define leChuva() digitalRead(nChuva)
#define temChuva() (!digitalRead(nChuva))

#endif /*_WCPENDIO_H */
