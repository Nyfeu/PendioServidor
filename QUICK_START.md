# 🚀 Quick Start - Pendio

Guia rápido para começar em 5 minutos.

## 1. Clonar e Preparar

```bash
git clone https://github.com/Nyfeu/PendioServidor.git
cd PendioServidor
cp include/credentials.example.h include/credentials.h
```

## 2. Configurar Credenciais LoRa

Editar `include/credentials.h`:
```cpp
const char APPEUI[] = "seu_appeui_aqui";
const char APPKEY[] = "sua_appkey_aqui";
```

## 3. Compilar

```bash
platformio run
```

Ou via VS Code: `Ctrl+Alt+B`

## 4. Upload

```bash
platformio run --target upload
```

Ou via VS Code: `Ctrl+Alt+U`

## 5. Monitor

```bash
platformio device monitor
```

---

## ✅ Esperado na Saída

```
[00:00:01.234] [INFO][SYSTEM] Logger initialized
[00:00:02.456] [INFO][COMM] LoRa handler starting...
[00:00:05.789] [INFO][COMM] Joined LoRaWAN network
[00:00:06.012] [INFO][SENSOR] Reading sensors...
```

---
