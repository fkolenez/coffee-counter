#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <Preferences.h>

const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;
const char *statePath = "/state.json";
const char *dataPath = "/data.ndjson";
unsigned long lastMenuAt = 0;
Preferences preferences;

struct State
{
    int today;
    int record;
    time_t lastCoffee;
    time_t lastSync;
};

State state = {0, 0, 0, 0};

String getMetrics()
{
    JsonDocument doc;

    int sketchSize = ESP.getSketchSize();
    int flashSize = ESP.getFreeSketchSpace();
    int availableSize = flashSize - sketchSize;

    doc["esp32_uptime"] = millis();
    doc["esp32_wifi_rssi"] = WiFi.RSSI();
    doc["esp32_heap_size"] = ESP.getHeapSize();
    doc["esp32_free_heap"] = ESP.getFreeHeap();
    doc["esp32_sketch_size"] = sketchSize;
    doc["esp32_flash_size"] = flashSize;
    doc["esp32_available_size"] = availableSize;
    doc["esp32_temperature"] = temperatureRead();
    doc["esp32_boot_counter"] = preferences.getInt("boot", 0);

    String metrics;
    serializeJson(doc, metrics);
    return metrics;
}

void menu()
{
    Serial.println("\n===== MENU =====");
    Serial.println("1 - Cadastrar café");
    Serial.println("2 - Mostrar timestamp");
    Serial.println("3 - Visualizar state");
    Serial.println("4 - Visualizar registros (logs)");
    Serial.println("m - Mostrar métricas");
    Serial.println("t - teste timestamp");
    Serial.println("r - Reiniciar");
    Serial.print("> ");
    lastMenuAt = millis();
}

void connectWifi()
{
    Serial.println("> Conectando na rede wifi");

    WiFi.begin(ssid, password);

    unsigned long startedAt = millis();

    while (WiFi.status() != WL_CONNECTED && millis() - startedAt < 15000)
    {
        delay(500);
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("\n> WiFi conectado");
        Serial.print("> IP: ");
        Serial.println(WiFi.localIP());
        return;
    }

    Serial.println("\n> Não foi possivel conectar ao WiFi.");
    Serial.println("> O menu vai continuar disponivel.");
}

void startLittleFS()
{
    if (!LittleFS.begin(true))
    {
        Serial.println("> Erro ao montar LittleFS");
        return;
    }
}

time_t getTimestamp()
{
    configTime(-3 * 3600, 0, "pool.ntp.org"); // UTC-3

    struct tm timeinfo;
    unsigned long startedAt = millis();

    while (!getLocalTime(&timeinfo))
    {
        Serial.println("> Aguardando sincronização...");
        delay(1000);

        if (millis() - startedAt > 10000)
        {
            Serial.println("> Não foi possivel sincronizar a hora.");
            return 0;
        }
    }

    return time(nullptr);
}

void saveState()
{
    JsonDocument doc;

    doc["today"] = state.today;
    doc["record"] = state.record;
    doc["lastCoffee"] = (long long)state.lastCoffee;
    doc["lastSync"] = (long long)state.lastSync;

    File file = LittleFS.open(statePath, "w");
    if (!file)
    {
        Serial.printf("Erro ao abrir %s para escrita\n", statePath);
        return;
    }

    if (serializeJson(doc, file) == 0)
    {
        Serial.printf("Erro ao salvar %s\n", statePath);
    }

    file.close();
}

void appendHistory(time_t now) {
//     JsonDocument doc;
//     doc["time"]
}

void showFile(String path)
{
    File file = LittleFS.open(path, "r");
    if (!file)
    {
        Serial.printf("Erro ao abrir %s para leitura\n", path);
        return;
    }

    Serial.printf("%s:\n\n", path);
    while (file.available())
    {
        Serial.write(file.read());
    }
    Serial.println();

    file.close();
}

void timeStampFormmatedTest()
{
    time_t timestamp = getTimestamp();

    if (timestamp == 0)
    {
        Serial.println("> Erro: timestamp nao sincronizado. Usando fallback fixo para teste.");
        timestamp = 1785294000; // 2026-07-29 00:00:00 UTC-3
    }

    // Converte o timestamp para o horário local
    struct tm *info_tempo = localtime(&timestamp);

    // Mostra a data em formato normal de texto
    Serial.printf("Timestamp Unix: %lld\n", (long long)timestamp);
    Serial.printf("Data local: %s", asctime(info_tempo));
} 

void setup()
{
    Serial.println("> Iniciando setup...");
    Serial.println("> Conectando na porta serial 115200");
    Serial.begin(115200);
    delay(1000);

    preferences.begin("storage", false);
    preferences.putInt("boot", preferences.getInt("boot", 0) + 1);

    Serial.println("> Conectando Wifi");
    connectWifi();
    delay(1000);

    Serial.println("> Iniciando LittleFS");
    startLittleFS();
    delay(1000);

    Serial.println("> Setup completo!");

    menu();
}

void loop()
{    
    if (!Serial.available())
    {
        if (millis() - lastMenuAt > 10000)
        {
            menu();
        }

        return;
    }

    char option = Serial.read();

    if (option == '\n' || option == '\r')
    {
        return;
    }

    switch (option)
    {
    /*
     *  Cadastrar café
     */
    case '1':
    {
        state.today++;

        time_t now = getTimestamp();

        // Atualiza o record salvo na variavel state
        if (state.today > state.record)
            state.record = state.today;

        state.lastCoffee = now;

        Serial.print("\n> Café registrado com sucesso! \n");

        saveState();
        appendHistory(now);
        break;
    }

    case '2':
    {
        time_t now = getTimestamp();

        if (now != 0)
        {
            Serial.printf("\n> Timestamp Unix: %lld\n", (long long)now);
        }

        break;
    }

    case '3':
    {
        showFile(statePath);
        break;
    }

    case '4':
    {
        showFile(dataPath);
        break;
    }

    case 'm':
    {
        Serial.println();
        Serial.println(getMetrics());
        break;
    }

    case 'r':
    {
        Serial.println("\n> Reiniciando...");
        ESP.restart();
        break;
    }

    case 't':
    {
        timeStampFormmatedTest();
        break;
    }

    default:
    {
        Serial.println("\n> Opcao invalida");
        break;
    }
    }

    while (Serial.available())
        Serial.read();

    menu();
}
