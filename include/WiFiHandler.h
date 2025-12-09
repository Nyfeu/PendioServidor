/**
 * @file WiFiHandler.h
 * @brief Handler de comunicação para Wi-Fi (exemplo de implementação)
 * @details Implementa a interface CommunicationHandler para comunicação via Wi-Fi
 * @note Esta é uma estrutura de exemplo - implementação completa requer biblioteca Wi-Fi
 * @copyright Copyright (c) 2025
 */

#ifndef _WIFI_HANDLER_H
#define _WIFI_HANDLER_H

#include "CommunicationHandler.h"

/**
 * @struct WiFiConfig
 * @brief Configuração do handler Wi-Fi
 */
struct WiFiConfig {
    const char* ssid;                       // SSID da rede Wi-Fi
    const char* password;                   // Senha da rede
    const char* serverAddr;                 // Endereço do servidor
    uint16_t serverPort;                    // Porta do servidor
    unsigned long connectTimeout;           // Timeout de conexão (ms)
    unsigned long sendTimeout;              // Timeout de envio (ms)
};

/**
 * @class WiFiHandler
 * @brief Handler de comunicação para Wi-Fi
 * 
 * Implementação de exemplo da interface CommunicationHandler para comunicação
 * via Wi-Fi. Pode ser facilmente intercambiada com LoRaHandler, mantendo
 * a mesma interface.
 */
class WiFiHandler : public CommunicationHandler {
private:
    WiFiConfig config;                      // Configuração
    ConnectionState currentState;           // Estado atual
    bool confirmed;                         // Flag de confirmação
    DownlinkMessage lastDownlink;           // Última mensagem recebida

public:
    /**
     * @brief Construtor do WiFiHandler
     * @param cfg Configuração inicial
     */
    explicit WiFiHandler(const WiFiConfig& cfg);

    /**
     * @brief Destrutor
     */
    ~WiFiHandler() override = default;

    /**
     * @brief Inicializa o handler Wi-Fi
     * @return bool true se sucesso
     */
    bool begin() override;

    /**
     * @brief Finaliza o handler Wi-Fi
     */
    void end() override;

    /**
     * @brief Conecta à rede Wi-Fi
     * @return bool true se conectado
     */
    bool connect() override;

    /**
     * @brief Verifica se está conectado
     * @return bool true se conectado
     */
    bool isConnected() override;

    /**
     * @brief Envia dados via Wi-Fi
     * @param port Port/Canal para envio (pode ser mapeado para canais HTTP)
     * @param data Dados a enviar
     * @param length Tamanho dos dados
     * @return SendResult Resultado
     */
    SendResult send(uint8_t port, const uint8_t* data, uint16_t length) override;

    /**
     * @brief Verifica confirmação do último envio
     * @return bool true se confirmado
     */
    bool isConfirmed() override;

    /**
     * @brief Recebe mensagem downlink
     * @param message Estrutura para armazenar
     * @return ReceiveResult Resultado
     */
    ReceiveResult receive(DownlinkMessage& message) override;

    /**
     * @brief Obtém estado da conexão
     * @return ConnectionState Estado atual
     */
    ConnectionState getConnectionState() override;

    /**
     * @brief Processa eventos Wi-Fi
     */
    void process() override;

    /**
     * @brief Obtém descrição do estado
     * @return const char* String descritiva
     */
    const char* getStateString() override;

private:
    /**
     * @brief Atualiza o estado da conexão
     */
    void updateState();
};

#endif /* _WIFI_HANDLER_H */
