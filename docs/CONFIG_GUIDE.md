# ⚙️ Guia de Configuração - Pendio

Referência completa das configurações do sistema.

---

## 📋 Localização das Configurações

```
include/config.h                    ← PRINCIPAL (edite aqui)
include/Pendio_LoRa_Wemos_Robocore.h  ← Hardware (raramente muda)
include/credentials.h               ← AppEUI, AppKey
platformio.ini                      ← Ambiente de build
```

---

## 🎯 Configurações Principais (config.h)

### Logging

```cpp
ENABLE_LOGGING      1              // Ativo
LOG_LEVEL_DEFAULT   LOG_LEVEL_INFO // INFO, WARN, ERROR, DEBUG
SERIAL_BAUDRATE     115200         // Não altere (padrão ESP32)
```

**Resultado**:
```
[00:01:23.456] [INFO][SYSTEM] Sistema iniciado
[00:02:45.789] [WARN][COMM] Tentando rejoin...
```

---

### LoRaWAN - Timeouts

| Config | Valor | Nota |
|--------|-------|------|
| `JOIN_TIMEOUT_VALUE` | 10000 ms | OTAA Join |
| `CFM_TIMEOUT_VALUE` | 180000 ms | Aguardar ACK (3 min) |
| `NEXT_MSG_TIMEOUT_VALUE` | 20000 ms | Entre mensagens (teste) |

**Cenários**:
- **Teste** (desenvolvimento): 20s entre mensagens
- **Produção** (duty cycle): 1800s (30 min)

```cpp
// Para produção, comentar/descomentar:
// #define NEXT_MSG_TIMEOUT_VALUE   20000      // Teste
#define NEXT_MSG_TIMEOUT_VALUE   1800000     // Produção
```

---

### LoRaWAN - Transmissão

| Config | Valor | Significado |
|--------|-------|-------------|
| `LORA_FIXED_DR` | 0-12 | Data Rate (se ADR off) |
| `LORA_ADR_ON` | 1 | Adaptive Data Rate |
| `LORA_USE_CONFIRMATION` | 0 | Mensagens confirmadas |
| `LORA_MAX_PAYLOAD` | 100 | Tamanho max [bytes] |
| `LORA_MAX_NACK_RETRIES` | 9 | Retentativas |

**Data Rates**:
```
DR 0  → SF12, BW=125kHz  (melhor alcance, mais lento)
DR 2  → SF10, BW=125kHz  (padrão)
DR 5  → SF7,  BW=125kHz  (mais rápido, menor alcance)
```

---

### Sensores

```cpp
SENSOR_AHT_ENABLED      1    // Temperatura/Umidade
SENSOR_BMP_ENABLED      1    // Pressão
SENSOR_SPENDIO_ENABLED  1    // RS485
SENSOR_RAIN_ENABLED     1    // Chuva
SENSOR_BATTERY_ENABLED  1    // Bateria
```

**Desabilitar sensor**: Mude para `0` se não estiver instalado.

---

### Pinos (Hardware)

```cpp
PIN_LED              2        // ESP32 GPIO2 (LED interno)
LORA_SERIAL_PORT     1        // Serial1 (TX=GPIO17, RX=GPIO16)
LORA_TX_POWER        20       // dBm (2-20)
```

**Atenção**: Não altere sem revisar `docs/HARDWARE.md`.

---

## 🔧 Casos Comuns de Ajuste

### 1️⃣ Modo Teste (Desenvolvimento)

```cpp
ENABLE_LOGGING             1
LOG_LEVEL_DEFAULT          LOG_LEVEL_DEBUG
NEXT_MSG_TIMEOUT_VALUE     20000         // 20s
LORA_ADR_ON                1
LORA_USE_CONFIRMATION      0             // Sem ACK
DEBUG_MODE                 1
```

✅ Logs verbosos, mensagens frequentes, sem confirmação.

---

### 2️⃣ Modo Produção (Campo)

```cpp
ENABLE_LOGGING             1
LOG_LEVEL_DEFAULT          LOG_LEVEL_INFO
NEXT_MSG_TIMEOUT_VALUE     1800000       // 30min
LORA_ADR_ON                1
LORA_USE_CONFIRMATION      1             // Com ACK
DEBUG_MODE                 0
```

✅ Logs econômicos, mensagens espaçadas, com confirmação.

---

### 3️⃣ Sensor Específico Ausente

Se o **BMP280 não está instalado**:

```cpp
SENSOR_BMP_ENABLED         0
```

O sistema ignora erros de inicialização do sensor.

---

### 4️⃣ Aumentar Comunicação (CFM)

Se precisa garantir entrega:

```cpp
LORA_USE_CONFIRMATION      1        // Pedir ACK
CFM_TIMEOUT_VALUE          180000   // Aguardar 3 min
LORA_MAX_NACK_RETRIES      9        // 9 tentativas
```

⚠️ Aumenta consumo de energia e uso de airtime.

---

## 📊 Comparação: Teste vs Produção

| Aspecto | Teste | Produção |
|---------|-------|----------|
| Log Level | DEBUG | INFO |
| Intervalo Mensagens | 20s | 1800s (30min) |
| CFM (ACK) | Não | Sim |
| Watchdog | Desabilitado | Habilitado |
| TX Power | 20 dBm | 14-20 dBm |

---

## ✅ Validação

Ao compilar, o sistema valida:

```cpp
#if LORA_FIXED_DR < 0 || LORA_FIXED_DR > 12
    #error "LORA_FIXED_DR inválido"
#endif
```

**Se erro**: Ajuste `config.h` e recompile.

---

## 🔑 Credenciais (credentials.h)

**Nunca** coloque credenciais em `config.h`. Use arquivo separado:

```cpp
// include/credentials.h
const char APPEUI[] = "26e7cc9af428bec1";
const char APPKEY[] = "cfeebad46ac8638d69fa23c5789926f3";
```

---
