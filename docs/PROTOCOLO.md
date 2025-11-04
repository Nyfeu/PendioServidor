# Documentação do Protocolo - Payload LoRaWAN

Este documento descreve a estrutura da mensagem (payload) enviada pela rede LoRaWAN. O payload é uma string ASCII que representa valores hexadecimais concatenados.

A estrutura é baseada na union `CPendio_LoRa_Sensor_Data_Type` definida em `include/Sensores.h`.

- Tamanho Total do Payload: 61 caracteres (Bytes)

- Interface do Payload (ASCII String): 
    - Formato: 
        ```
        01<SensorB_data(14)><SensorM_data(14)><SensorT_data(14)><Temp(2)><Umid(2)><Pressao(5)><Pluv(4)><Bat(3)><Final(1)>
        ```
    - Exemplo:
        ```
        011FB1FF25603A7E1F81F2276F423F1F41F8269F423F212B15BC600260B50
        ```

## Estrutura Detalhada do Payload

| Campo | Tamanho (caracteres) | Descrição | Exemplo (Valor) |
|---|:-:|---|-:|
| `frmFmtV` | 2 | Versão do Formato do Frame (ASCII) | `01` |
| `Sensor B | 14 | Dados do Sensor SPendio 'B' (Base) | |
| `acx[3]` | 3 | Acelerômetro X (Hex ASCII) | `1FB` | 
| `acy[3]` | 3| Acelerômetro Y (Hex ASCII)| `1FF` | 
| `acz[3]` | 3 | Acelerômetro Z (Hex ASCII) | `256` | 
| `solo[5]` | 5 | Sensor de Solo (Hex ASCII) | `03A7E` |
| `Sensor M | 14 | Dados do Sensor SPendio 'M' (Meio) | |
| `acx[3]` | 3 | Acelerômetro X (Hex ASCII) | `1F8` |
| `acy[3]` | 3  | Acelerômetro Y (Hex ASCII) | `1F2` |
| `acz[3]` | 3 | Acelerômetro Z (Hex ASCII) | `276` |
| `solo[5]` | 5 | Sensor de Solo (Hex ASCII) | `F423F` |
| `Sensor T | 14 | Dados do Sensor SPendio 'T' (Topo) | |
| `acx[3]` | 3 | Acelerômetro X (Hex ASCII) | `1F4` | 
| `acy[3]` | 3 | Acelerômetro Y (Hex ASCII) | `1F8` |
| `acz[3]` | 3 | Acelerômetro Z (Hex ASCII) | `269` |
| `solo[5]` | 5 | Sensor de Solo (Hex ASCII) | `F423F` | 
| `temp` | 2 | Temperatura (AHT) (Hex ASCII) | `21` (33°C) |
| `umid` | 2 | Umidade (AHT) (Hex ASCII) | `2B` (43%) | 
| `pressao` | 5 | Pressão Atmosférica (BMP) (Hex ASCII) | `15BC6` (89030 Pa) | 
| `pluv` | 4 | Contagem Sensor de Chuva (Hex ASCII) | `0026` (38 contagens) |
| `bat` | 3 | Tensão da Bateria (Hex ASCII) | `0B5` | 
| `final` | 1 | Caractere Finalizador (ASCII) | `0` |

--- 