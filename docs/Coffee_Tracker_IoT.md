# Coffee Tracker IoT

## Objetivo

Desenvolver um dispositivo IoT utilizando um **ESP32** para registrar
automaticamente (via botão) a quantidade de cafés consumidos ao longo do
dia, mantendo histórico local persistente e, futuramente, sincronizando
os dados com uma API para geração de estatísticas.

## Arquitetura

``` text
               Wi-Fi (NTP)

                  │
                  ▼
             Servidor NTP

                  ▲
                  │
+-----------------------------------+
|              ESP32                |
|                                   |
|  OLED                             |
|  LED RGB                          |
|  Botão Café                       |
|  Botão Sync                       |
|                                   |
| LittleFS                          |
| ├── state.json                    |
| ├── history.ndjson                |
| └── config.json                   |
+-----------------------------------+
                  │
                  ▼
              API REST
                  │
            PostgreSQL
                  │
             Dashboard
```

## Requisitos Funcionais

-   Registrar um café ao pressionar um botão.
-   Persistir dados em memória Flash utilizando LittleFS.
-   Registrar timestamp de cada café utilizando NTP.
-   Manter histórico completo dos registros.
-   Exibir informações em um display OLED.
-   Indicar estados através de LED RGB.
-   Permitir sincronização futura com uma API REST.

## Requisitos Não Funcionais

-   Inicialização inferior a 5 segundos.
-   Funcionamento offline após sincronização inicial do horário.
-   Persistência local sem perda de dados.
-   Código modular e de fácil expansão.

## Persistência

### state.json

``` json
{
  "today": 5,
  "record": 11,
  "lastCoffee": 1753528000,
  "lastSync": 1753520000
}
```

### history.ndjson

``` text
{"timestamp":1753521000}
{"timestamp":1753524500}
{"timestamp":1753528200}
```

## Componentes

-   ESP32 Dev Module
-   Display OLED SSD1306 I²C
-   LED RGB (WS2812B)
-   Botão de registro
-   Botão de sincronização (futuro)

## Fluxo

1.  Ligar o ESP32.
2.  Conectar ao Wi-Fi.
3.  Sincronizar horário via NTP.
4.  Carregar estado salvo.
5.  Registrar cafés.
6.  Persistir localmente.
7.  Sincronizar com API quando solicitado.

## Roadmap

### MVP

-   Contador por botão.
-   Persistência local.
-   Testes via Serial Monitor.

### Evolução

-   OLED e LED RGB.
-   Histórico em LittleFS.
-   Sincronização NTP.
-   API REST.
-   PostgreSQL.
-   Dashboard.

## Tecnologias

### Firmware

-   ESP32
-   Arduino Framework
-   LittleFS
-   ArduinoJson
-   WiFi
-   NTP (`configTime`)

### Backend (futuro)

-   FastAPI ou Flask
-   PostgreSQL
-   Docker
-   Grafana ou dashboard próprio.
