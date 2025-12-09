# 🎯 Como Usar as Configurações

Guia rápido para ajustar o Pendio ao seu caso de uso.

---

## 1️⃣ Localizar Configuração

**Onde?**  
→ `include/config.h`

**Nunca editar:**
- `Pendio_LoRa_Wemos_Robocore.h` (hardware apenas)
- `credentials.h` (credenciais, versão controlada)

---

## 2️⃣ Exemplo: Ajustar Intervalo de Mensagens

### Cenário: Teste Rápido

```cpp
// Editar config.h
#define NEXT_MSG_TIMEOUT_VALUE      20000     // 20 segundos ← DESCOMENTE
// #define NEXT_MSG_TIMEOUT_VALUE   1800000    // 30 minutos
```

### Cenário: Produção (Economizar Energia)

```cpp
// Editar config.h
// #define NEXT_MSG_TIMEOUT_VALUE      20000
#define NEXT_MSG_TIMEOUT_VALUE   1800000      // 30 minutos ← DESCOMENTE
```

**Resultado**: Mensagens a cada 30 minutos em vez de 20 segundos.

---

## 3️⃣ Exemplo: Desabilitar Sensor não Instalado

Se o **BMP280 não está conectado**:

```cpp
// Em config.h
// #define SENSOR_BMP_ENABLED      1        // Comentar esta linha
#define SENSOR_BMP_ENABLED         0        // ou mudar para 0
```

✅ Sistema ignora erros de inicialização.

---

## 4️⃣ Exemplo: Modo Debug

Para **desenvolvimento com logs verbosos**:

```cpp
// Em config.h
#define LOG_LEVEL_DEFAULT       LOG_LEVEL_DEBUG  // Mudar de INFO
#define DEBUG_MODE              1                // Ativar
#define NEXT_MSG_TIMEOUT_VALUE  5000             // 5 segundos
```

**Na serial você verá**:
```
[HH:MM:SS.ms] [DEBUG][tag] Mensagem detalhada
[HH:MM:SS.ms] [DEBUG][tag] Outra informação
...
```

---

## 5️⃣ Exemplo: Confirmação de Mensagens

Se quer **garantir entrega** (com custo de energia):

```cpp
// Em config.h
#define LORA_USE_CONFIRMATION    1       // Pedir ACK
#define CFM_TIMEOUT_VALUE        180000  // 3 minutos para esperar
#define LORA_MAX_NACK_RETRIES    9       // 9 tentativas
```

⚠️ Aumenta consumo de energia (~2x).

---

## 6️⃣ Antes de Compilar: Checklist

```
□ Editou config.h (não o .h de hardware)
□ Credenciais em include/credentials.h estão corretas
□ Sensor que está desabilitado? Mude SENSOR_*_ENABLED = 0
□ Pronto para teste ou produção?
  → Teste: NEXT_MSG_TIMEOUT_VALUE = 20000
  → Produção: NEXT_MSG_TIMEOUT_VALUE = 1800000
```

---

## 7️⃣ Compilar e Fazer Upload

```bash
# Compilar
platformio run

# Upload
platformio run --target upload

# Monitor Serial
platformio device monitor
```

**Esperado na saída**:
```
[00:00:01.234] [INFO][SYSTEM] Logger initialized
[00:00:05.678] [INFO][COMM] LoRa connected
[00:00:10.000] [INFO][SENSOR] Reading sensors...
```

---
