# Handlers de Comunicação - Documentação

## Visão Geral

O projeto Pendio implementa um padrão de **handlers intercambiáveis** para comunicação. Isso permite alternar entre diferentes métodos de transmissão (LoRa, Wi-Fi, 4G, etc.) sem modificar a lógica principal do programa.

### Benefícios
- ✅ **Modularidade**: Lógica de comunicação isolada em módulos específicos
- ✅ **Reutilização**: Mesmo código de aplicação funciona com diferentes meios de comunicação
- ✅ **Facilidade de teste**: Implementar mock handlers para testes
- ✅ **Extensibilidade**: Adicionar novos meios de comunicação facilmente

## Arquitetura

```
┌─────────────────────────────────┐
│         main.cpp (Aplicação)    │
│    Lógica de sensores, estados  │
│                                 │
│  commHandler->send()            │
│  commHandler->receive()         │
└──────────────┬──────────────────┘
               │
               ├─── Interface Virtual ───┐
               │  CommunicationHandler   │
               └─────────────────────────┤
               │                         │
        ┌──────▼────────────────┐        │
        │   LoRaHandler         │        │
        │  (Implementação)      │        │
        └───────────────────────┘        │
                                         │
                                    ┌────▼──────────┐
                                    │  WiFiHandler  │
                                    │  (Futura)     │
                                    └───────────────┘
```

## Classe Base: CommunicationHandler

Definida em `include/CommunicationHandler.h`, fornece a interface virtual que todos os handlers devem implementar.

### Métodos Principais

```cpp
class CommunicationHandler {
public:
    
    // Inicialização
    virtual bool begin() = 0;                              
    // Inicializa o hardware
    virtual void end() = 0;                               
    // Finaliza o hardware
    
    // Conexão
    virtual bool connect() = 0;                           
    // Conecta à rede/servidor
    virtual bool isConnected() = 0;                       
    // Verifica se conectado
    
    // Envio
    virtual SendResult send(uint8_t port, 
                           const uint8_t* data, 
                           uint16_t length) = 0;          // Envia dados
    virtual bool isConfirmed() = 0;                       
    // Verifica confirmação
    
    // Recebimento
    virtual ReceiveResult receive(DownlinkMessage& msg) = 0; 
    // Recebe dados
    
    // Estado
    virtual ConnectionState getConnectionState() = 0;     
    // Obtém estado
    virtual void process() = 0;                           
    // Processa eventos
    virtual const char* getStateString() = 0;             // Descrição do estado
};
```

### Enumerações Importantes

```cpp
// Estados de conexão
enum class ConnectionState {
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
    WAITING_CONFIRMATION,
    ERROR
};

// Resultado de envio
enum class SendResult {
    SUCCESS,                // Envio aceito
    PENDING,                // Pendente
    FAILED,                 // Falhou
    NOT_CONNECTED,          // Não conectado
    INVALID_DATA            // Dados inválidos
};

// Resultado de recebimento
enum class ReceiveResult {
    MESSAGE_RECEIVED,       // Mensagem recebida
    NO_MESSAGE,             // Nenhuma mensagem
    ERROR                   // Erro
};
```

## Usando na Aplicação Principal

### Instanciação (em main.cpp)

```cpp
// Configurar parâmetros
LoRaConfig loraConfig = {
    .serial = &loraSerial,
    .appEUI = (const uint8_t*)APPEUI,
    .appKey = (const uint8_t*)APPKEY,
    .useConfirmation = false,
    .useADR = LORA_ADR_ON,
    .fixedDR = LORA_FIXED_DR,
    .joinTimeout = JOIN_TIMEOUT_VALUE,
    .confirmTimeout = CFM_TIMEOUT_VALUE,
    .maxRetries = 3
};

// Criar instância (polimorfa)
CommunicationHandler* commHandler = new LoRaHandler(loraConfig);

// Ou para Wi-Fi:
// WiFiConfig wifiConfig = { ... };
// CommunicationHandler* commHandler = new WiFiHandler(wifiConfig);
```

### Inicialização

```cpp
if (!commHandler->begin()) {
    Serial.println(F("Falha ao inicializar comunicação"));
    while(1) delay(1000);
}

// Tentar conectar
if (!commHandler->connect()) {
    Serial.println(F("Falha ao conectar"));
}
```

### Envio de Dados

```cpp
SendResult result = commHandler->send(1, data, length);

switch(result) {
    case SendResult::SUCCESS:
        Serial.println("Envio aceito");
        break;
    case SendResult::NOT_CONNECTED:
        Serial.println("Não conectado");
        break;
    case SendResult::INVALID_DATA:
        Serial.println("Dados inválidos");
        break;
    case SendResult::FAILED:
        Serial.println("Envio recusado");
        break;
    case SendResult::PENDING:
        Serial.println("Envio pendente");
        break;
}
```

### Verificação de Confirmação

```cpp
if (commHandler->isConfirmed()) {
    Serial.println("Mensagem confirmada");
} else {
    Serial.println("Aguardando confirmação...");
}
```

### Recebimento de Dados

```cpp
DownlinkMessage msg;
ReceiveResult result = commHandler->receive(msg);

if (result == ReceiveResult::MESSAGE_RECEIVED) {
    Serial.print("Mensagem na porta: ");
    Serial.println(msg.port);
    Serial.print("Tamanho: ");
    Serial.println(msg.length);
    
    // Processar dados
    for (int i = 0; i < msg.length; i++) {
        Serial.println(msg.data[i]);
    }
}
```

### Processamento Cíclico

```cpp
void loop() {
    // ... lógica de aplicação ...
    
    // Processar eventos de comunicação
    commHandler->process();
    
    // Verificar estado
    Serial.println(commHandler->getStateString());
}
```

## Implementando um Novo Handler

### Passo 1: Criar o Header (.h)

```cpp
#ifndef _MY_COMM_HANDLER_H
#define _MY_COMM_HANDLER_H

#include "CommunicationHandler.h"

struct MyCommConfig {
    // Parâmetros específicos do seu handler
    unsigned long timeout;
    // ...
};

class MyCommHandler : public CommunicationHandler {
private:
    MyCommConfig config;
    ConnectionState currentState;
    bool confirmed;
    DownlinkMessage lastDownlink;

public:
    explicit MyCommHandler(const MyCommConfig& cfg);
    
    // Implementar todos os métodos virtuais
    bool begin() override;
    void end() override;
    bool connect() override;
    bool isConnected() override;
    SendResult send(uint8_t port, const uint8_t* data, uint16_t length) override;
    bool isConfirmed() override;
    ReceiveResult receive(DownlinkMessage& message) override;
    ConnectionState getConnectionState() override;
    void process() override;
    const char* getStateString() override;

private:
    void updateState();
};

#endif
```

### Passo 2: Implementar o .cpp

```cpp
#include "MyCommHandler.h"

MyCommHandler::MyCommHandler(const MyCommConfig& cfg)
    : config(cfg),
      currentState(ConnectionState::DISCONNECTED),
      confirmed(false) {
    // Inicializar lastDownlink
    memset(lastDownlink.data, 0, sizeof(lastDownlink.data));
}

bool MyCommHandler::begin() {
    Serial.println(F("[MyComm] Inicializando..."));
    // Implementar inicialização
    currentState = ConnectionState::DISCONNECTED;
    return true;
}

// ... Implementar demais métodos ...
```

### Passo 3: Usar na Aplicação

```cpp
#include "MyCommHandler.h"

MyCommConfig config = { ... };
CommunicationHandler* commHandler = new MyCommHandler(config);
```

## Exemplo Prático: Alternância LoRa ↔ Wi-Fi

```cpp
// Definir qual meio usar
#define USE_LORA 1
// #define USE_WIFI 1

void setup() {
    #ifdef USE_LORA
        LoRaConfig loraConfig = { /* ... */ };
        commHandler = new LoRaHandler(loraConfig);
    #endif
    
    #ifdef USE_WIFI
        WiFiConfig wifiConfig = { /* ... */ };
        commHandler = new WiFiHandler(wifiConfig);
    #endif
    
    // Resto do código é idêntico!
    commHandler->begin();
    commHandler->connect();
}

void loop() {
    // Código funciona com qualquer handler
    if (commHandler->isConnected()) {
        commHandler->send(1, data, length);
        if (commHandler->isConfirmed()) {
            // Envio confirmado
        }
    }
}
```

## Handlers Disponíveis

### LoRaHandler (`include/LoRaHandler.h`)
- **Arquivo de implementação**: `src/LoRaHandler.cpp`
- **Hardware**: SMW_SX1262M0 via Serial UART
- **Características**:
  - OTAA Join
  - Confirmação de mensagens (CFM)
  - Adaptive Data Rate (ADR)
  - Data Rate fixo configurável

### WiFiHandler (`include/WiFiHandler.h`)
- **Arquivo de implementação**: `src/WiFiHandler.cpp`
- **Hardware**: ESP32 built-in Wi-Fi
- **Status**: Estrutura de exemplo (necessita completar implementação)
- **Características planejadas**:
  - Conexão SSID/Password
  - HTTP POST/GET
  - TCP direto

## Tratamento de Erros

Cada handler gerencia seu próprio estado e erros através de:

1. **ConnectionState**: Indica estado geral da conexão
2. **SendResult**: Indica resultado de cada tentativa de envio
3. **ReceiveResult**: Indica resultado de cada tentativa de recebimento

```cpp
// Exemplo com tratamento robusto
ConnectionState state = commHandler->getConnectionState();
if (state == ConnectionState::ERROR) {
    Serial.println("Erro na comunicação!");
    delay(5000);
    commHandler->connect();  // Tentar reconectar
}
```

## Estrutura de Diretórios

```
include/
├── CommunicationHandler.h       ← Interface abstrata
├── LoRaHandler.h               ← Handler LoRa
└── WiFiHandler.h               ← Handler Wi-Fi

src/
├── main.cpp                    ← Aplicação principal (agnóstica)
├── LoRaHandler.cpp            ← Implementação LoRa
└── WiFiHandler.cpp            ← Implementação Wi-Fi
```

## Boas Práticas

1. **Sempre verificar conexão antes de enviar**
   ```cpp
   if (commHandler->isConnected()) {
       commHandler->send(...);
   }
   ```

2. **Processar eventos regularmente**
   ```cpp
   void loop() {
       commHandler->process();  // Chamar em todo ciclo
       // ... resto da lógica ...
   }
   ```

3. **Usar enumerações para estados**
   ```cpp
   if (result == SendResult::SUCCESS) { ... }
   // Melhor que: if (result == 0) { ... }
   ```

4. **Não fazer suposições sobre timing**
   ```cpp
   // ❌ ERRADO: Assume resposta imediata
   commHandler->send(...);
   Serial.println(commHandler->isConfirmed());
   
   // ✅ CORRETO: Aguarda confirmação em ciclos subsequentes
   if (commHandler->send(...) == SendResult::SUCCESS) {
       state = WAITING_CONFIRMATION;
   }
   ```

## Suporte e Contribuição

Para adicionar um novo método de comunicação:

1. Criar header e implementação seguindo o padrão
2. Implementar todos os métodos virtuais
3. Atualizar este documento
4. Testar com código existente (sem mudanças na aplicação)

---

