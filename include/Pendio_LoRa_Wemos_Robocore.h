/**
 * @file Pendio_LoRa_Wemos_Robocore.h
 * @brief Constantes de hardware do sistema Pendio
 * @details Define pinos e mapeamento de hardware
 * @author Eng. Nuncio Perrella, MSc
 * @copyright Copyright (c) 2025
 *
 * @note Configurações de software estão em config.h
 */

#ifndef _PENDIO_LORA_WEMOS_ROBOCORE_H
#define _PENDIO_LORA_WEMOS_ROBOCORE_H

#include <Arduino.h>

// ============================================================================
// HARDWARE - Mapeamento de Pinos (ESP32 DOIT DEVKIT V1)
// ============================================================================

/**
 * @brief LED do módulo
 * @details Usado para indicação visual de status do sistema
 */
#define MODULE_LED_PIN                  2

// ============================================================================
// Macros Utilitárias
// ============================================================================

/**
 * @brief Converte número (0-15) para dígito hexadecimal (0-9, A-F)
 * @param x Valor 0-15
 * @return Caractere '0'-'9' ou 'A'-'F'
 *
 * @note Útil para conversão de arrays de bytes para string hex
 */
#if !defined(Nib)
#define Nib(x)  ((x > 9) ? ('A' + x - 0x0A) : ('0' + x))
#endif

#endif // _PENDIO_LORA_WEMOS_ROBOCORE_H


