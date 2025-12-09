# Exemplos de Uso - Handlers de Comunicação

## Exemplo 1: Usar LoRaHandler (Padrão)

```cpp
#include "CommunicationHandler.h"
#include "LoRaHandler.h"

HardwareSerial loraSerial(1);

void setup() {
    Serial.begin(115200);
    
    // Configurar LoRa
    LoRaConfig loraConfig = {
        .serial = &loraSerial,
        .appEUI = (const uint8_t*)APPEUI,
        .appKey = (const uint8_t*)APPKEY,
        .useConfirmation = true,
        .useADR = true,
        .fixedDR = 5,
        .joinTimeout = 30000,
        .confirmTimeout = 6000,
        .maxRetries = 3
    };
    
    // Criar handler
    CommunicationHandler* handler = new LoRaHandler(loraConfig);
    
    // Inicializar
    if (!handler->begin()) {
        Serial.println("Falha ao inicializar LoRa");
        return;
    }
    
    // Conectar
    if (!handler->connect()) {
        Serial.println("Falha ao conectar");
        return;
    }
    
    Serial.println("LoRa inicializado com sucesso");
}

void loop() {
    handler->process();
    
    if (handler->isConnected()) {
        uint8_t data[] = {0x01, 0x02, 0x03, 0x04};
        SendResult result = handler->send(1, data, 4);
        
        if (result == SendResult::SUCCESS) {
            Serial.println("Mensagem enfileirada");
        }
    }
    
    delay(100);
}
```

## Exemplo 2: Alternar entre LoRa e Wi-Fi com Compilação Condicional

```cpp
#include "CommunicationHandler.h"

// Descomente o que deseja usar
#define USE_LORA
// #define USE_WIFI
// #define USE_MOCK

#ifdef USE_LORA
    #include "LoRaHandler.h"
#endif

#ifdef USE_WIFI
    #include "WiFiHandler.h"
#endif

#ifdef USE_MOCK
    #include "MockCommHandler.h"
#endif

CommunicationHandler* handler = nullptr;

void setup() {
    Serial.begin(115200);
    
    #ifdef USE_LORA
    {
        LoRaConfig config = { /* ... */ };
        handler = new LoRaHandler(config);
        Serial.println("Usando LoRaHandler");
    }
    #endif
    
    #ifdef USE_WIFI
    {
        WiFiConfig config = {
            .ssid = "MyNetwork",
            .password = "password",
            .serverAddr = "192.168.1.100",
            .serverPort = 8080,
            .connectTimeout = 15000,
            .sendTimeout = 5000
        };
        handler = new WiFiHandler(config);
        Serial.println("Usando WiFiHandler");
    }
    #endif
    
    #ifdef USE_MOCK
    {
        MockCommConfig config = {
            .joinDelay = 2000,
            .sendDelay = 1000,
            .autoConfirm = true,
            .simulateErrors = false,
            .errorRate = 0
        };
        handler = new MockCommHandler(config);
        Serial.println("Usando MockCommHandler");
    }
    #endif
    
    // Resto do código é idêntico
    if (handler) {
        handler->begin();
        handler->connect();
    }
}

void loop() {
    if (!handler) return;
    
    handler->process();
    
    static unsigned long lastSend = 0;
    if (millis() - lastSend > 10000) {
        lastSend = millis();
        
        if (handler->isConnected()) {
            uint8_t data[] = {0xDE, 0xAD, 0xBE, 0xEF};
            handler->send(1, data, 4);
        }
    }
    
    delay(100);
}
```

## Exemplo 3: Máquina de Estados Robusta

```cpp
enum class AppState {
    INIT,
    CONNECTING,
    READY,
    SENDING,
    WAITING_ACK,
    ERROR
};

CommunicationHandler* handler = nullptr;
AppState appState = AppState::INIT;
unsigned long stateTimeout = 0;

void setup() {
    // Inicializar handler...
    appState = AppState::INIT;
}

void updateAppState() {
    unsigned long now = millis();
    
    switch (appState) {
        case AppState::INIT:
            if (handler->begin()) {
                appState = AppState::CONNECTING;
                Serial.println("Estado: Conectando...");
            }
            break;
            
        case AppState::CONNECTING:
            if (handler->isConnected()) {
                appState = AppState::READY;
                Serial.println("Estado: Pronto para enviar");
            } else if (now - stateTimeout > 60000) {
                appState = AppState::ERROR;
                Serial.println("Estado: Timeout na conexão");
            }
            break;
            
        case AppState::READY:
            // Aqui coleta sensores e muda para SENDING
            appState = AppState::SENDING;
            break;
            
        case AppState::SENDING: {
            uint8_t data[] = {0x01, 0x02, 0x03};
            SendResult result = handler->send(1, data, 3);
            
            if (result == SendResult::SUCCESS) {
                appState = AppState::WAITING_ACK;
                stateTimeout = now;
                Serial.println("Estado: Aguardando ACK");
            } else if (result == SendResult::FAILED) {
                appState = AppState::ERROR;
            }
            break;
        }
            
        case AppState::WAITING_ACK:
            if (handler->isConfirmed()) {
                appState = AppState::READY;
                Serial.println("Estado: Confirmação recebida");
            } else if (now - stateTimeout > 10000) {
                appState = AppState::ERROR;
                Serial.println("Estado: Timeout no ACK");
            }
            break;
            
        case AppState::ERROR:
            Serial.print("Estado: Erro - ");
            Serial.println(handler->getStateString());
            
            if (now - stateTimeout > 30000) {
                handler->connect();
                appState = AppState::CONNECTING;
                Serial.println("Estado: Reconectando...");
            }
            break;
    }
    
    handler->process();
}

void loop() {
    updateAppState();
    delay(100);
}
```

## Exemplo 4: Testes com Mock Handler

```cpp
#include "MockCommHandler.h"

void testBasicSend() {
    Serial.println("\n=== Teste 1: Envio básico ===");
    
    MockCommConfig config = {
        .joinDelay = 1000,
        .sendDelay = 500,
        .autoConfirm = true,
        .simulateErrors = false,
        .errorRate = 0
    };
    
    MockCommHandler handler(config);
    handler.begin();
    handler.connect();
    
    // Esperar conexão
    delay(1500);
    
    if (handler.isConnected()) {
        uint8_t data[] = {0xAA, 0xBB, 0xCC};
        SendResult result = handler.send(1, data, 3);
        
        Serial.print("Resultado: ");
        Serial.println(result == SendResult::SUCCESS ? "OK" : "FALHA");
    }
}

void testWithDownlink() {
    Serial.println("\n=== Teste 2: Com downlink ===");
    
    MockCommConfig config = {
        .joinDelay = 1000,
        .sendDelay = 500,
        .autoConfirm = true,
        .simulateErrors = false,
        .errorRate = 0
    };
    
    MockCommHandler handler(config);
    handler.begin();
    handler.connect();
    
    // Simular recebimento
    uint8_t downlinkData[] = {0x80, 0x00, 0x05, 0x10};
    handler.injectDownlink(1, downlinkData, 4);
    
    // Aguardar
    delay(1500);
    
    // Verificar recebimento
    DownlinkMessage msg;
    ReceiveResult result = handler.receive(msg);
    
    Serial.print("Downlink recebido: ");
    Serial.println(result == ReceiveResult::MESSAGE_RECEIVED ? "SIM" : "NÃO");
    
    if (result == ReceiveResult::MESSAGE_RECEIVED) {
        Serial.print("Porta: ");
        Serial.println(msg.port);
        Serial.print("Tamanho: ");
        Serial.println(msg.length);
    }
}

void testErrorSimulation() {
    Serial.println("\n=== Teste 3: Simulação de erros ===");
    
    MockCommConfig config = {
        .joinDelay = 1000,
        .sendDelay = 500,
        .autoConfirm = true,
        .simulateErrors = true,  // Ativar erros
        .errorRate = 50          // 50% de erro
    };
    
    MockCommHandler handler(config);
    handler.begin();
    handler.connect();
    
    delay(1500);
    
    int successCount = 0;
    int failCount = 0;
    
    for (int i = 0; i < 10; i++) {
        uint8_t data[] = {0x01};
        SendResult result = handler.send(1, data, 1);
        
        if (result == SendResult::SUCCESS) {
            successCount++;
        } else if (result == SendResult::FAILED) {
            failCount++;
        }
        
        delay(100);
    }
    
    Serial.print("Sucessos: ");
    Serial.print(successCount);
    Serial.print(" | Falhas: ");
    Serial.println(failCount);
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    testBasicSend();
    testWithDownlink();
    testErrorSimulation();
    
    Serial.println("\n=== Testes Concluídos ===");
}

void loop() {
    delay(1000);
}
```

## Exemplo 5: Logging Centralizado

```cpp
class CommLogger {
private:
    CommunicationHandler* handler;
    unsigned long lastStateCheck = 0;
    
public:
    CommLogger(CommunicationHandler* h) : handler(h) {}
    
    void logStatus() {
        if (millis() - lastStateCheck > 5000) {
            lastStateCheck = millis();
            
            Serial.print("[COMM] Estado: ");
            Serial.println(handler->getStateString());
            
            Serial.print("[COMM] Conectado: ");
            Serial.println(handler->isConnected() ? "SIM" : "NÃO");
            
            ConnectionState state = handler->getConnectionState();
            Serial.print("[COMM] ConnectionState: ");
            Serial.println((int)state);
        }
    }
    
    void logSendResult(SendResult result, uint16_t size) {
        Serial.print("[SEND] Resultado: ");
        
        switch (result) {
            case SendResult::SUCCESS:
                Serial.print("SUCESSO");
                break;
            case SendResult::PENDING:
                Serial.print("PENDENTE");
                break;
            case SendResult::FAILED:
                Serial.print("FALHA");
                break;
            case SendResult::NOT_CONNECTED:
                Serial.print("NÃO_CONECTADO");
                break;
            case SendResult::INVALID_DATA:
                Serial.print("DADOS_INVÁLIDOS");
                break;
        }
        
        Serial.print(" | Tamanho: ");
        Serial.println(size);
    }
    
    void logReceiveResult(ReceiveResult result, const DownlinkMessage* msg) {
        Serial.print("[RX] Resultado: ");
        
        switch (result) {
            case ReceiveResult::MESSAGE_RECEIVED:
                Serial.println("MENSAGEM_RECEBIDA");
                if (msg) {
                    Serial.print("    Porta: ");
                    Serial.print(msg->port);
                    Serial.print(" | Tamanho: ");
                    Serial.println(msg->length);
                }
                break;
            case ReceiveResult::NO_MESSAGE:
                Serial.println("SEM_MENSAGEM");
                break;
            case ReceiveResult::ERROR:
                Serial.println("ERRO");
                break;
        }
    }
};

CommunicationHandler* handler = nullptr;
CommLogger* logger = nullptr;

void setup() {
    Serial.begin(115200);
    
    // Inicializar handler...
    handler = new LoRaHandler(/* config */);
    logger = new CommLogger(handler);
    
    handler->begin();
    handler->connect();
}

void loop() {
    handler->process();
    logger->logStatus();
    
    if (handler->isConnected()) {
        static unsigned long lastSend = 0;
        if (millis() - lastSend > 10000) {
            lastSend = millis();
            
            uint8_t data[] = {0x01, 0x02, 0x03};
            SendResult result = handler->send(1, data, 3);
            logger->logSendResult(result, 3);
            
            DownlinkMessage msg;
            ReceiveResult rxResult = handler->receive(msg);
            logger->logReceiveResult(rxResult, &msg);
        }
    }
    
    delay(100);
}
```

## Exemplo 6: Factory Pattern (Construir handler dinamicamente)

```cpp
class CommHandlerFactory {
public:
    enum class HandlerType {
        LORA,
        WIFI,
        MOCK
    };
    
    static CommunicationHandler* create(HandlerType type) {
        switch (type) {
            case HandlerType::LORA: {
                LoRaConfig config = { /* ... */ };
                return new LoRaHandler(config);
            }
            
            case HandlerType::WIFI: {
                WiFiConfig config = { /* ... */ };
                return new WiFiHandler(config);
            }
            
            case HandlerType::MOCK: {
                MockCommConfig config = { /* ... */ };
                return new MockCommHandler(config);
            }
            
            default:
                return nullptr;
        }
    }
};

void setup() {
    // Criar handler baseado em define
    #ifdef PRODUCTION
        CommunicationHandler* handler = CommHandlerFactory::create(
            CommHandlerFactory::HandlerType::LORA
        );
    #else
        CommunicationHandler* handler = CommHandlerFactory::create(
            CommHandlerFactory::HandlerType::MOCK
        );
    #endif
    
    handler->begin();
}
```

---
