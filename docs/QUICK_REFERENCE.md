# Quick Reference - Handlers de Comunicação

## 🚀 Iniciar Rapidamente

### LoRa (Produção)

```cpp
#include "LoRaHandler.h"

LoRaConfig cfg = {
    .serial = &Serial1,
    .appEUI = (const uint8_t*)APPEUI,
    .appKey = (const uint8_t*)APPKEY,
    .useConfirmation = true,
    .useADR = true,
    .fixedDR = 5,
    .joinTimeout = 30000,
    .confirmTimeout = 6000,
    .maxRetries = 3
};

CommunicationHandler* handler = new LoRaHandler(cfg);
handler->begin();
handler->connect();
```

### Mock (Testes)

```cpp
#include "MockCommHandler.h"

MockCommConfig cfg = {
    .joinDelay = 1000,
    .sendDelay = 500,
    .autoConfirm = true,
    .simulateErrors = false,
    .errorRate = 0
};

CommunicationHandler* handler = new MockCommHandler(cfg);
handler->begin();
handler->connect();
```

## 📋 Estados

```cpp
ConnectionState state = handler->getConnectionState();

// Possiveis valores:
// - ConnectionState::DISCONNECTED
// - ConnectionState::CONNECTING
// - ConnectionState::CONNECTED
// - ConnectionState::WAITING_CONFIRMATION
// - ConnectionState::ERROR
```

## 📤 Enviar Dados

```cpp
uint8_t data[] = {0x01, 0x02, 0x03};
SendResult result = handler->send(1, data, 3);

switch(result) {
    case SendResult::SUCCESS:
        Serial.println("OK");
        break;
    case SendResult::NOT_CONNECTED:
        Serial.println("Desconectado");
        break;
    case SendResult::FAILED:
        Serial.println("Falha");
        break;
    case SendResult::INVALID_DATA:
        Serial.println("Dados inválidos");
        break;
    case SendResult::PENDING:
        Serial.println("Pendente");
        break;
}
```

## ✅ Verificar Confirmação

```cpp
if (handler->isConfirmed()) {
    Serial.println("Confirmação recebida!");
} else {
    Serial.println("Aguardando...");
}
```

## 📥 Receber Dados

```cpp
DownlinkMessage msg;
ReceiveResult rx = handler->receive(msg);

if (rx == ReceiveResult::MESSAGE_RECEIVED) {
    Serial.print("Porta: ");
    Serial.println(msg.port);
    
    Serial.print("Tamanho: ");
    Serial.println(msg.length);
    
    // Processar dados
    for (int i = 0; i < msg.length; i++) {
        Serial.print(msg.data[i], HEX);
        Serial.print(" ");
    }
}
```

## 🔄 Loop Típico

```cpp
void loop() {
    // 1. Processar eventos
    handler->process();
    
    // 2. Verificar conexão
    if (!handler->isConnected()) {
        handler->connect();
        return;
    }
    
    // 3. Enviar dados (a cada X segundos)
    static unsigned long lastSend = 0;
    if (millis() - lastSend > 10000) {
        lastSend = millis();
        
        uint8_t data[] = {sensor1, sensor2, sensor3};
        SendResult res = handler->send(1, data, 3);
        
        if (res == SendResult::SUCCESS) {
            state = WAITING_ACK;
        }
    }
    
    // 4. Receber downlinks
    if (state == WAITING_ACK) {
        if (handler->isConfirmed()) {
            DownlinkMessage msg;
            if (handler->receive(msg) == ReceiveResult::MESSAGE_RECEIVED) {
                // Processar downlink
            }
            state = READY;
        }
    }
    
    delay(100);
}
```

## ⚙️ Configurações Comuns

### LoRa Padrão (Robocore)
```cpp
LoRaConfig cfg = {
    .serial = &Serial1,
    .appEUI = (const uint8_t*)APPEUI,
    .appKey = (const uint8_t*)APPKEY,
    .useConfirmation = true,    // Exigir ACK
    .useADR = true,              // Adaptar DR automaticamente
    .fixedDR = 5,                // Não usado se ADR=true
    .joinTimeout = 30000,        // 30s
    .confirmTimeout = 6000,      // 6s
    .maxRetries = 3              // 3 tentativas
};
```

### LoRa Sem Confirmação
```cpp
LoRaConfig cfg = {
    // ... igual ...
    .useConfirmation = false,    // Sem ACK
    // ... resto igual ...
};
```

### LoRa com DR Fixo
```cpp
LoRaConfig cfg = {
    // ... igual ...
    .useADR = false,             // Desabilitar ADR
    .fixedDR = 3,                // DR fixo = 3
    // ... resto igual ...
};
```

### Mock com Erros
```cpp
MockCommConfig cfg = {
    .joinDelay = 1000,
    .sendDelay = 500,
    .autoConfirm = true,
    .simulateErrors = true,      // Simular erros
    .errorRate = 30              // 30% de taxa de erro
};
```

## 📊 Enumerações - Valores Numéricos

```cpp
// ConnectionState
0 = DISCONNECTED
1 = CONNECTING
2 = CONNECTED
3 = WAITING_CONFIRMATION
4 = ERROR

// SendResult
0 = SUCCESS
1 = PENDING
2 = FAILED
3 = NOT_CONNECTED
4 = INVALID_DATA

// ReceiveResult
0 = MESSAGE_RECEIVED
1 = NO_MESSAGE
2 = ERROR
```

## 🔍 Debug Útil

```cpp
// Ver estado como string
Serial.println(handler->getStateString());

// Verificar conexão
if (handler->isConnected()) {
    Serial.println("Conectado");
} else {
    Serial.println("Desconectado");
}

// Obter estado detalhado
ConnectionState state = handler->getConnectionState();
Serial.print("Estado: ");
Serial.println((int)state);

// Loop count (em MockCommHandler)
MockCommHandler* mock = (MockCommHandler*)handler;
Serial.print("Mensagens: ");
Serial.println(mock->getMessageCount());
```

## ⏱️ Timeouts Padrão

```cpp
JOIN_TIMEOUT_VALUE       = 30000   // 30 segundos
CFM_TIMEOUT_VALUE        = 6000    // 6 segundos
NXTMSG_TIMEOUT_VALUE     = 3000    // 3 segundos
```

## 💾 Memory

```cpp
sizeof(LoRaConfig)        ≈ 40 bytes
sizeof(LoRaHandler)       ≈ 150 bytes
sizeof(MockCommHandler)   ≈ 100 bytes
sizeof(DownlinkMessage)   ≈ 270 bytes
```

--- 
