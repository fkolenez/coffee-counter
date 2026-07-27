#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;
unsigned long lastMenuAt = 0;

struct State
{
    int today;
    int record;
    time_t lastCoffee;
    time_t lastSync;
};

State state = {0, 0, 0, 0};

void menu()
{
    Serial.println("\n===== MENU =====");
    Serial.println("1 - Cadastrar café");
    Serial.println("2 - Mostrar uptime");
    Serial.println("3 - Visualizar LitteFS");
    Serial.println("r - Reiniciar");
    Serial.print("> ");
    lastMenuAt = millis();
}

void connectWifi()
{
    Serial.println("Conectando na rede wifi");

    WiFi.begin(ssid, password);

    unsigned long startedAt = millis();

    while (WiFi.status() != WL_CONNECTED && millis() - startedAt < 15000)
    {
        delay(500);
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("\nWiFi conectado");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
        return;
    }

    Serial.println("\nNao foi possivel conectar ao WiFi.");
    Serial.println("O menu vai continuar disponivel.");
}

void startLittleFS()
{
    if (!LittleFS.begin(true))
    {
        Serial.println("Erro ao montar LittleFS");
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
        Serial.println("Aguardando sincronização...");
        delay(1000);

        if (millis() - startedAt > 10000)
        {
            Serial.println("Nao foi possivel sincronizar a hora.");
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

    File file = LittleFS.open("/state.json", "w");
    if (!file)
    {
        Serial.println("Erro ao abrir /state.json para escrita");
        return;
    }

    if (serializeJson(doc, file) == 0)
    {
        Serial.println("Erro ao salvar /state.json");
    }

    file.close();
}

void setup()
{
    Serial.println("Conectando na porta serial 115200");
    Serial.begin(115200);
    delay(1000);

    Serial.println("Conectando Wifi");
    connectWifi();
    delay(1000);

    Serial.println("Iniciando LittleFS");
    startLittleFS();
    delay(1000);

    Serial.println("Setup completo!");

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
     *. Cadastrar café
     */
    case '1':
        state.today++;

       time_t now = getTimestamp();

       // Atualiza o record salvo na variavel state
        if (state.today > state.record)
            state.record = state.today;

        state.lastCoffee = now;

        saveState();
        // appendHistory(now);
        break;

    case '2':
    {
        time_t now = getTimestamp();

        if (now != 0)
        {
            Serial.printf("Timestamp Unix: %lld\n", (long long)now);
        }

        break;
    }

    case 'r':
        Serial.println("Reiniciando...");
        ESP.restart();
        break;

    default:
        Serial.println("Opcao invalida");
        break;
    }

    while (Serial.available())
        Serial.read();

    menu();
}
