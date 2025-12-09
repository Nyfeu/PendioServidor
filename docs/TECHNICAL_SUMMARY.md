# Resumo Técnico - Isolamento de Lógica LoRa

## 🎯 Objetivo

Isolar a lógica de comunicação LoRa em um módulo independente e intercambiável, permitindo fácil substituição por outros métodos de comunicação sem alterar a lógica principal da aplicação.

## 🔧 Implementação

### 1. Interface Abstrata (`CommunicationHandler`)

**Localização**: `include/CommunicationHandler.h`

**Propósito**: Define contrato que todos os handlers devem implementar.

**Estrutura**:
```cpp
class CommunicationHandler {
    // Inicialização
    virtual bool begin() = 0;
    virtual void end() = 0;
    
    // Conexão
    virtual bool connect() = 0;
    virtual bool isConnected() = 0;
    
    // Comunicação
    virtual SendResult send(uint8_t port, const uint8_t* data, uint16_t length) = 0;
    virtual bool isConfirmed() = 0;
    virtual ReceiveResult receive(DownlinkMessage& message) = 0;
    
    // Estado
    virtual ConnectionState getConnectionState() = 0;
    virtual void process() = 0;
    virtual const char* getStateString() = 0;
};
```

**Enumerações**:

1. **ConnectionState**
   - `DISCONNECTED`: Não conectado
   - `CONNECTING`: Em processo de conexão
   - `CONNECTED`: Conectado e pronto
   - `WAITING_CONFIRMATION`: Aguardando ACK/resposta
   - `ERROR`: Erro na comunicação

2. **SendResult**
   - `SUCCESS`: Mensagem aceita para envio
   - `PENDING`: Envio ainda não processado
   - `FAILED`: Falha no envio
   - `NOT_CONNECTED`: Não está conectado
   - `INVALID_DATA`: Dados inválidos

3. **ReceiveResult**
   - `MESSAGE_RECEIVED`: Mensagem foi recebida
   - `NO_MESSAGE`: Nenhuma mensagem disponível
   - `ERROR`: Erro ao tentar receber

**Estrutura de Dados**:

```cpp
struct DownlinkMessage {
    uint8_t port;                      // FPort (1-223)
    uint8_t data[256];                 // Payload
    uint16_t length;                   // Tamanho do payload
    uint32_t timestamp;                // Quando foi recebido
};
```

### 2. Implementação LoRa (`LoRaHandler`)

**Localização**: 
- Header: `include/LoRaHandler.h`
- Implementação: `src/LoRaHandler.cpp`

**Componentes**:

```cpp
struct LoRaConfig {
    HardwareSerial* serial;         // Serial UART (padrão: Serial1)
    const uint8_t* appEUI;          // 8 bytes
    const uint8_t* appKey;          // 16 bytes
    bool useConfirmation;           // CFM ON/OFF
    bool useADR;                    // Adaptive Data Rate
    uint8_t fixedDR;                // Data Rate 0-7
    unsigned long joinTimeout;      // Timeout JOIN (ms)
    unsigned long confirmTimeout;   // Timeout CFM (ms)
    uint8_t maxRetries;             // Máximo de tentativas
};
```

**Método de Operação**:

1. **Inicialização** (`begin()`)
   - Reset do módulo SMW_SX1262M0
   - Configuração de AppEUI e AppKey
   - Configuração de OTAA Join
   - Configuração de CFM, ADR, DR
   - Salvar configurações

2. **Conexão** (`connect()`)
   - Enviar comando JOIN
   - Aguardar por `joinTimeout`
   - Retornar sucesso/falha

3. **Envio** (`send()`)
   - Validar dados (1-242 bytes)
   - Verificar conexão
   - Enviar via `lorawan.sendX(port, data)`
   - Retornar resultado

4. **Confirmação** (`isConfirmed()`)
   - Se CFM ativo: verificar `lorawan.isConfirmed()`
   - Se CFM inativo: retornar true após envio aceito
   - Atualizar estado

5. **Recebimento** (`receive()`)
   - Ler buffer via `lorawan.readX()`
   - Copiar dados para `DownlinkMessage`
   - Retornar resultado

6. **Processamento** (`process()`)
   - Atualizar estado
   - Verificar timeouts
   - Gerenciar retentativas

**Máquina de Estados Interna**:

```
┌─────────────────┐
│  DISCONNECTED   │◄───────────────────────────┐
└────────┬────────┘                            │
         │ connect()                           │
         ▼                                     │
┌─────────────────┐                          │
│   CONNECTING    │                          │
└────────┬────────┘                          │
         │ (joinTimeout)                     │
         ▼                                   │
┌─────────────────┐                        │
│   CONNECTED     │◄──────────┐           │
└────────┬────────┘           │           │
         │ send()             │           │
         ▼                    │           │
┌─────────────────┐          │           │
│ WAITING_CFM     │──────────┼────────►  │
└────────┬────────┘    (timeout)        │
         │ isConfirmed()                │
         └──────────────────────────────┘
```

### 3. Implementação Wi-Fi (`WiFiHandler`) - Exemplo

**Localização**:
- Header: `include/WiFiHandler.h`
- Implementação: `src/WiFiHandler.cpp`

**Status**: Estrutura de exemplo - implementação completa requer:
- Biblioteca `WiFi.h` (ESP32)
- Biblioteca `HTTPClient.h` (para HTTP)
- Ou `WiFiClient.h` (para TCP)

**Configuração**:
```cpp
struct WiFiConfig {
    const char* ssid;           // SSID da rede
    const char* password;       // Senha
    const char* serverAddr;     // IP/hostname do servidor
    uint16_t serverPort;        // Porta (ex: 8080)
    unsigned long connectTimeout;
    unsigned long sendTimeout;
};
```

### 4. Mock Handler (`MockCommHandler`) - Testes

**Localização**:
- Header: `include/MockCommHandler.h`
- Implementação: `src/MockCommHandler.cpp`

**Propósito**: Simular comportamento de handler real sem hardware.

**Recursos**:
- Simular delays de join/envio
- Confirmação automática
- Injetar downlinks para testes
- Simular erros aleatórios

**Uso**:
```cpp
MockCommConfig cfg = {
    .joinDelay = 2000,
    .sendDelay = 1000,
    .autoConfirm = true,
    .simulateErrors = false,
    .errorRate = 0
};

MockCommHandler handler(cfg);
```

## 📊 Fluxo de Dados

### Envio

```
main.cpp loop()
    │
    ├─ Ler sensores
    ├─ Montar payload
    │
    ▼
handler->send(port, data, length)
    │
    ▼
LoRaHandler::send()
    ├─ Validar entrada
    ├─ Verificar isConnected()
    │
    ▼
lorawan.sendX(port, data)  ◄─── SMW_SX1262M0
    │
    ▼
SendResult (SUCCESS/PENDING/FAILED/...)
    │
    ▼
main.cpp loop()
    └─ STATE_WAIT_CFM
```

### Recebimento

```
main.cpp loop()
    │
    ▼
handler->receive(downlink)
    │
    ▼
LoRaHandler::receive()
    │
    ├─ lorawan.readX(port, buffer)  ◄─── SMW_SX1262M0
    │
    ├─ Copiar para DownlinkMessage
    │
    ▼
ReceiveResult (MESSAGE_RECEIVED/NO_MESSAGE/ERROR)
    │
    ▼
main.cpp loop()
    └─ Processar dados
```

## 🔄 Sequência de Operações Típica

### Setup

```
LoRaConfig config = {...}
    │
    ▼
handler = new LoRaHandler(config)
    │
    ▼
handler->begin()
    ├─ lorawan.reset()
    ├─ lorawan.set_AppEUI()
    ├─ lorawan.set_AppKey()
    ├─ lorawan.set_JoinMode(OTAA)
    ├─ lorawan.set_CFM()
    ├─ lorawan.set_ADR()
    └─ lorawan.save()
    │
    ▼
handler->connect()
    ├─ lorawan.join()
    └─ Aguardar resposta (joinTimeout)
    │
    ▼
estado = CONNECTED
```

### Loop Cíclico

```
loop() {
    handler->process()  ◄─── Atualizar estados, timeouts
    
    if (handler->isConnected()) {
        SendResult result = handler->send(port, data, len)
        
        if (result == SUCCESS) {
            estado = WAITING_CFM
        }
    }
    
    if (estado == WAITING_CFM) {
        if (handler->isConfirmed()) {
            DownlinkMessage msg;
            ReceiveResult rx = handler->receive(msg)
            
            if (rx == MESSAGE_RECEIVED) {
                Processar downlink
            }
            
            estado = READY
        }
    }
}
```

## 📁 Arquitetura de Arquivos

```
PendioServidor/
│
├─ include/
│  ├─ CommunicationHandler.h    ◄─── Interface abstrata
│  ├─ LoRaHandler.h             ◄─── Implementação LoRa
│  ├─ WiFiHandler.h             ◄─── Estrutura Wi-Fi
│  ├─ MockCommHandler.h         ◄─── Handler para testes
│  │
│  ├─ aplic.h
│  ├─ config.h
│  ├─ credentials.h
│  └─ ... (outros headers)
│
├─ src/
│  ├─ main.cpp                  ◄─── Aplicação principal (refatorada)
│  ├─ LoRaHandler.cpp           ◄─── Implementação LoRa
│  ├─ WiFiHandler.cpp           ◄─── Estrutura Wi-Fi
│  ├─ MockCommHandler.cpp       ◄─── Testes
│  ├─ Sensores.cpp
│  ├─ HW.cpp
│  └─ ... (outras implementações)
│
├─ docs/
│  ├─ COMMUNICATION_HANDLERS.md  ◄─── Guia completo
│  ├─ USAGE_EXAMPLES.md          ◄─── Exemplos práticos
│  └─ TECHNICAL_SUMMARY.md       ◄─── Este arquivo
│
├─ platformio.ini
└─ README.md
```

## 💾 Ocupação de Memória

**Estimativa por Handler**:
- LoRaHandler: ~2KB RAM, ~15KB Flash
- WiFiHandler: ~1.5KB RAM, ~10KB Flash
- MockCommHandler: ~1KB RAM, ~8KB Flash
- CommunicationHandler interface: ~200 bytes (virtual)

**Total adicional**: ~20-25KB Flash (negligenciável para ESP32)

## ⚡ Performance

| Operação | Tempo | Notas |
|----------|-------|-------|
| `begin()` | ~500ms | Reset + configuração |
| `connect()` | Variável | Até 30s (timeout) |
| `send()` | ~10ms | Apenas enfileiração |
| `isConfirmed()` | <1ms | Leitura de flag |
| `receive()` | <1ms | Se mensagem na fila |
| `process()` | <5ms | Verificação de timeout |

## 🛡️ Tratamento de Erros

### Em LoRaHandler

1. **Erros de Inicialização**
   - Reset falhar → `begin()` retorna false
   - Configuração falhar → `begin()` retorna false

2. **Erros de Conexão**
   - JOIN timeout → estado = `ERROR`
   - JOIN recusado → retenta automaticamente

3. **Erros de Envio**
   - Não conectado → `SendResult::NOT_CONNECTED`
   - Dados inválidos → `SendResult::INVALID_DATA`
   - Módulo recusa → `SendResult::FAILED`

4. **Timeouts**
   - JOIN timeout: `config.joinTimeout`
   - CFM timeout: `config.confirmTimeout`
   - Retentativas: `config.maxRetries`

### Em Aplicação

```cpp
SendResult result = handler->send(1, data, len);

switch(result) {
    case SendResult::SUCCESS:
        // Envio aceito
        break;
    
    case SendResult::NOT_CONNECTED:
        // Tentar reconectar
        handler->connect();
        break;
    
    case SendResult::FAILED:
        // Incrementar contador de erro
        err_count++;
        break;
    
    // ...
}
```

## 🔗 Dependências Externas

### LoRaHandler requer:
- `<RoboCore_SMW_SX1262M0.h>` - Driver do módulo
- `<HardwareSerial.h>` - Comunicação serial
- `<Arduino.h>` - Funções básicas

### WiFiHandler requer (quando completo):
- `<WiFi.h>` - Biblioteca Wi-Fi ESP32
- `<HTTPClient.h>` - Para HTTP (opcional)
- `<WiFiClient.h>` - Para TCP direto

### MockCommHandler requer:
- `<Arduino.h>` - Apenas funções básicas

### main.cpp requer:
- Apenas interface abstrata
- Não requer conhecimento específico de LoRa/WiFi

## 🎓 Padrões de Design Utilizados

1. **Strategy Pattern**
   - CommunicationHandler = estratégia abstrata
   - LoRaHandler, WiFiHandler = estratégias concretas
   - main.cpp = contexto

2. **Factory Pattern** (opcional)
   - Criar handlers baseado em tipo
   - Facilita seleção em tempo de execução

3. **Polymorphism**
   - Mesmo código funciona com qualquer handler
   - Sem conversão de tipos (type-safe)

4. **Encapsulation**
   - Detalhes LoRa encapsulados
   - Interface clara e simples

## 📈 Escalabilidade

Para adicionar novo método (ex: 4G):

1. Criar `include/4GHandler.h`
2. Criar `src/4GHandler.cpp`
3. Herdar de `CommunicationHandler`
4. Implementar 9 métodos virtuais
5. Usar em main.cpp sem mudanças

**Tempo estimado**: 2-4 horas de desenvolvimento

## ✅ Verificação

Para validar implementação:

```cpp
// Testar interface
void test_interface() {
    MockCommConfig cfg = {...};
    CommunicationHandler* h = new MockCommHandler(cfg);
    
    assert(h->begin() == true);
    assert(h->connect() == true);
    
    uint8_t data[] = {1,2,3};
    SendResult r = h->send(1, data, 3);
    assert(r == SendResult::SUCCESS);
    
    assert(h->isConnected() == true);
}

// Testar troca de handler
void test_handler_swap() {
    // Código em main.cpp funciona com qualquer:
    CommunicationHandler* h1 = new LoRaHandler(...);
    CommunicationHandler* h2 = new MockCommHandler(...);
    // Ambos funcionam igualmente
}
```

---

