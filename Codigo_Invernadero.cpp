#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

#include <DHT.h>
#include <IRremote.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeSans9pt7b.h>

// Conexion WiFi
const char *ssid = "Pan con queso";
const char *password = "";

// Telegram
#define BOT_TOKEN "8924730197:AAGIl-a-kboaMJKGDBqiQO70fMNGrNuRxhk"
#define CHAT_ID "8960196922"


// Codigos IR
#define COD1 0xBA45FF00
#define COD2 0xB946FF00
#define COD3 0xBB44FF00
#define COD4 0xBF40FF00
#define COD5 0xF807FF00
#define COD6 0xEA15FF00
#define COD7 0xE916FF00
#define COD8 0xE619FF00
#define COD9 0xF30CFF00
#define COD10 0xE718FF00

// Pines
#define INTERRUPTOR_PIN 18

// Simulación puerta
#define MOTOR_ABRIR 17
#define MOTOR_CERRAR 16

#define IMP1_1 26
#define IMP1_2 27

#define VENTILADOR 33 // VENTILADOR
#define BOMBA 32      // BOMBA

#define LUCES 13
#define IR_RECEIVER_PIN 34
// DHT22
#define DHTPIN 4
#define DHTTYPE DHT22

// OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SCREEN_ADDRESS 0x3C
#define OLED_RESET -1

// Sensores
#define MQ135 39
#define MQ135D 5

#define SENS_H 35
#define LDRA 36

#define IR_sensor_pin 19

// Modos del sistema
enum ModoSistema
{
    AUTOMATICO,
    MANUAL
};

enum TipoControl
{
    CONTROL_CERCANO,
    CONTROL_LEJANO
};

ModoSistema modoSistema = AUTOMATICO;
TipoControl tipoControl = CONTROL_LEJANO;

// Telegram
WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

DHT dht(DHTPIN, DHTTYPE);

Adafruit_SSD1306 display(
    SCREEN_WIDTH,
    SCREEN_HEIGHT,
    &Wire,
    OLED_RESET);

unsigned long lastTimeBotRan = 0;
const int botRequestDelay = 1000;

// Variables globales
int humedadSuelo = 0;
int porcentajeHumedadSuelo = 0;
int valorLDR = 0;
int porcentajeLuz = 0;
int valorAire = 0;
bool obstaculoAnterior = false;
bool puertaAbiertaIR = false;

unsigned long tiempoSinObstaculo = 0;

bool contandoCierre = false;

// Conexion WiFi
void conectarWiFi()
{
    Serial.print("Conectando WiFi");

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nWiFi conectado");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
}

// Puerta (abrir/cerrar)
void abrirPuerta()
{
    Serial.println("PUERTA ABIERTA");

    digitalWrite(MOTOR_CERRAR, LOW);
    digitalWrite(MOTOR_ABRIR, HIGH);

    delay(1000);

    digitalWrite(MOTOR_ABRIR, LOW);
}

void cerrarPuerta()
{
    Serial.println("PUERTA CERRADA");

    digitalWrite(MOTOR_ABRIR, LOW);
    digitalWrite(MOTOR_CERRAR, HIGH);

    delay(1000);

    digitalWrite(MOTOR_CERRAR, LOW);
}

void encenderLuces()
{
    digitalWrite(LUCES, HIGH);

    //Serial.println("LUCES ENCENDIDAS");
}

void apagarLuces()
{
    digitalWrite(LUCES, LOW);

    //Serial.println("LUCES APAGADAS");
}

void abrirTecho()
{
    Serial.println("TECHO ABIERTO");

    digitalWrite(IMP1_1, LOW);
    digitalWrite(IMP1_2, HIGH);

    delay(1000);

    digitalWrite(IMP1_1, LOW);
    digitalWrite(IMP1_2, LOW);
}

void cerrarTecho()
{
    Serial.println("TECHO CERRADO");

    digitalWrite(IMP1_1, HIGH);
    digitalWrite(IMP1_2, LOW);

    delay(1000);

    digitalWrite(IMP1_1, LOW);
    digitalWrite(IMP1_2, LOW);
}

void encenderVentilador()
{
    Serial.println("VENTILADOR ON");

    digitalWrite(VENTILADOR, LOW);
}

void apagarVentilador()
{
    //Serial.println("VENTILADOR OFF");

    digitalWrite(VENTILADOR, HIGH);
}

void encenderBomba()
{
    digitalWrite(BOMBA, HIGH);

    Serial.println("BOMBA ENCENDIDA");
}

void apagarBomba()
{
    digitalWrite(BOMBA, LOW);

    Serial.println("BOMBA APAGADA");
}
// Estado del invernadero
String obtenerEstado()
{
    float humedad = dht.readHumidity();
    float temperatura = dht.readTemperature();

    valorAire = analogRead(MQ135);

    humedadSuelo = analogRead(SENS_H);

    // Ajustar según tu sensor
    porcentajeHumedadSuelo =
        map(humedadSuelo,
            0,
            2000,
            0,
            100);

    porcentajeHumedadSuelo =
        constrain(
            porcentajeHumedadSuelo,
            0,
            100);

    valorLDR = analogRead(LDRA);

    porcentajeLuz =
        map(valorLDR,
            0,
            4095,
            0,
            100);

    bool obstaculoActual =
        (digitalRead(IR_sensor_pin) == LOW);

    String estadoAire;

    if (valorAire < 200)
    {
        estadoAire = "✅ Aire normal";
    }
    else if (valorAire <= 400)
    {
        estadoAire = "⚠️ Aire contaminado";
    }
    else
    {
        estadoAire = "☠️ PELIGRO";
    }

    String mensaje;

    mensaje = "📊 ESTADO DEL INVERNADERO\n\n";

    mensaje += "🌡️ Temp: ";
    mensaje += String(temperatura, 1);
    mensaje += " C\n\n";

    mensaje += "💧 Humedad: ";
    mensaje += String(humedad, 1);
    mensaje += " %\n\n";

    mensaje += "🌫️ ";
    mensaje += estadoAire;
    mensaje += "\n\n";

    mensaje += "🌱 Humedad suelo: ";
    mensaje += String(porcentajeHumedadSuelo);
    mensaje += "%";
    mensaje += "\n\n";

    mensaje += "🌕 OSCURIDAD: ";
    mensaje += String(porcentajeLuz);
    mensaje += "%\n\n";

    mensaje += "💡 Luces: ";

    if (digitalRead(LUCES))
    {
        mensaje += "ENCENDIDAS\n\n";
    }
    else
    {
        mensaje += "APAGADAS\n\n";
    }

    mensaje += "\n\n🌀 Ventilador: ";

    if (digitalRead(VENTILADOR) == LOW)
    {
        mensaje += "ENCENDIDO";
    }
    else
    {
        mensaje += "APAGADO";
    }

    mensaje += "\n\n💧 Bomba: ";

    if (digitalRead(BOMBA) == HIGH)
    {
        mensaje += "ENCENDIDA\n";
    }
    else
    {
        mensaje += "APAGADA\n";
    }

    if (digitalRead(IR_sensor_pin) == LOW)
    {
        mensaje += "🚧 Obstaculo detectado\n";
    }
    else
    {
        mensaje += "✅ Camino libre\n";
    }

    mensaje += "\n";

    if (modoSistema == AUTOMATICO)
    {
        mensaje += "🤖 Modo: AUTOMATICO";
    }
    else
    {
        mensaje += "🖐️ Modo: MANUAL\n";

        if (tipoControl == CONTROL_CERCANO)
        {
            mensaje += "📡 Control: IR";
        }
        else
        {
            mensaje += "🌎 Control: TELEGRAM";
        }
    }

    return mensaje;
}

// Comandos disponibles
String obtenerComandos()
{
    String comandos;

    comandos += "COMANDOS DISPONIBLES\n\n";
    comandos += "/estado\n";
    comandos += "/control_cercano\n";
    comandos += "/control_lejano\n";
    comandos += "/abrir_puerta\n";
    comandos += "/cerrar_puerta\n";
    comandos += "/abrir_techo\n";
    comandos += "/cerrar_techo\n";
    comandos += "/encender_ventilador\n";
    comandos += "/apagar_ventilador\n";
    comandos += "/encender_bomba\n";
    comandos += "/apagar_bomba\n";
    comandos += "/encender_luces\n";
    comandos += "/apagar_luces\n";

    return comandos;
}

// Telegram
void handleNewMessages(int numNewMessages)
{
    for (int i = 0; i < numNewMessages; i++)
    {
        String chat_id = bot.messages[i].chat_id;
        String text = bot.messages[i].text;

        Serial.println(text);

        // ==========================
        // ESTADO
        // ==========================
        if (text == "/estado")
        {
            bot.sendMessage(chat_id,
                            obtenerEstado(),
                            "");
        }

        if (text == "/comandos")
        {
            bot.sendMessage(chat_id,
                            obtenerComandos(),
                            "");
        }

        // ==========================
        // CAMBIO DE CONTROL
        // ==========================
        if (text == "/control_cercano")
        {
            if (modoSistema == MANUAL)
            {
                
                tipoControl = CONTROL_CERCANO;

                while (IrReceiver.decode())
                {
                    IrReceiver.resume();
                }

                bot.sendMessage(chat_id,
                                "📡 Control cercano activado (IR)",
                                "");
            }
            else
            {
                bot.sendMessage(chat_id,
                                "❌ Debe estar en modo MANUAL",
                                "");
            }
        }

        if (text == "/control_lejano")
        {
            if (modoSistema == MANUAL)
            {
                tipoControl = CONTROL_LEJANO;

                while (IrReceiver.decode())
                {
                    IrReceiver.resume();
                }

                bot.sendMessage(chat_id,
                                "🌎 Control lejano activado (Telegram)",
                                "");
            }
            else
            {
                bot.sendMessage(chat_id,
                                "❌ Debe estar en modo MANUAL",
                                "");
            }
        }

        // ==========================
        // PUERTA DESDE TELEGRAM
        // ==========================
        if (text == "/abrir_puerta")
        {
            if (modoSistema == MANUAL &&
                tipoControl == CONTROL_LEJANO)
            {
                abrirPuerta();

                bot.sendMessage(chat_id,
                                "🚪 Puerta abierta",
                                "");
            }
            else
            {
                bot.sendMessage(chat_id,
                                "❌ Telegram deshabilitado",
                                "");
            }
        }

        if (text == "/cerrar_puerta")
        {
            if (modoSistema == MANUAL &&
                tipoControl == CONTROL_LEJANO)
            {
                cerrarPuerta();

                bot.sendMessage(chat_id,
                                "🚪 Puerta cerrada",
                                "");
            }
            else
            {
                bot.sendMessage(chat_id,
                                "❌ Telegram deshabilitado",
                                "");
            }
        }

        if (text == "/abrir_techo")
        {
            if (modoSistema == MANUAL &&
                tipoControl == CONTROL_LEJANO)
            {
                abrirTecho();

                bot.sendMessage(
                    chat_id,
                    "🏠 Techo abierto",
                    "");
            }
            else
            {
                bot.sendMessage(
                    chat_id,
                    "❌ Telegram deshabilitado",
                    "");
            }
        }

        if (text == "/cerrar_techo")
        {
            if (modoSistema == MANUAL &&
                tipoControl == CONTROL_LEJANO)
            {
                cerrarTecho();

                bot.sendMessage(
                    chat_id,
                    "🏠 Techo cerrado",
                    "");
            }
            else
            {
                bot.sendMessage(
                    chat_id,
                    "❌ Telegram deshabilitado",
                    "");
            }
        }

        if (text == "/encender_ventilador")
        {
            if (modoSistema == MANUAL &&
                tipoControl == CONTROL_LEJANO)
            {
                encenderVentilador();

                bot.sendMessage(
                    chat_id,
                    "🌀 Ventilador encendido",
                    "");
            }
            else
            {
                bot.sendMessage(
                    chat_id,
                    "❌ Telegram deshabilitado",
                    "");
            }
        }

        if (text == "/apagar_ventilador")
        {
            if (modoSistema == MANUAL &&
                tipoControl == CONTROL_LEJANO)
            {
                apagarVentilador();

                bot.sendMessage(
                    chat_id,
                    "🛑 Ventilador apagado",
                    "");
            }
            else
            {
                bot.sendMessage(
                    chat_id,
                    "❌ Telegram deshabilitado",
                    "");
            }
        }

        if (text == "/encender_bomba")
        {
            if (modoSistema == MANUAL &&
                tipoControl == CONTROL_LEJANO)
            {
                encenderBomba();

                bot.sendMessage(
                    chat_id,
                    "💧 Bomba encendida",
                    "");
            }
            else
            {
                bot.sendMessage(
                    chat_id,
                    "❌ Telegram deshabilitado",
                    "");
            }
        }

        if (text == "/apagar_bomba")
        {
            if (modoSistema == MANUAL &&
                tipoControl == CONTROL_LEJANO)
            {
                apagarBomba();

                bot.sendMessage(
                    chat_id,
                    "💧 Bomba apagada",
                    "");
            }
            else
            {
                bot.sendMessage(
                    chat_id,
                    "❌ Telegram deshabilitado",
                    "");
            }
        }

        if (text == "/encender_luces")
        {
            if (modoSistema == MANUAL &&
                tipoControl == CONTROL_LEJANO)
            {
                encenderLuces();

                bot.sendMessage(
                    chat_id,
                    "💡 Luces encendidas",
                    "");
            }
            else
            {
                bot.sendMessage(
                    chat_id,
                    "❌ Telegram deshabilitado",
                    "");
            }
        }

        if (text == "/apagar_luces")
        {
            if (modoSistema == MANUAL &&
                tipoControl == CONTROL_LEJANO)
            {
                apagarLuces();

                bot.sendMessage(
                    chat_id,
                    "💡 Luces apagadas",
                    "");
            }
            else
            {
                bot.sendMessage(
                    chat_id,
                    "❌ Telegram deshabilitado",
                    "");
            }
        }
    }
}

void funcionesAutomaticas()
{
    float temperatura =
        dht.readTemperature();

    valorAire =
        analogRead(MQ135);

    humedadSuelo =
        analogRead(SENS_H);

    porcentajeHumedadSuelo =
        map(humedadSuelo,
            0,
            2000,
            0,
            100);

    porcentajeHumedadSuelo =
        constrain(
            porcentajeHumedadSuelo,
            0,
            100);

    // VENTILADOR
    if (temperatura > 35 ||
        valorAire > 650)
    {
        encenderVentilador();
    }
    else if (temperatura < 30 || valorAire < 350)
    {
        apagarVentilador();
    }

    Serial.print("ADC: ");
    Serial.print(humedadSuelo);

    Serial.print("  Porcentaje: ");
    Serial.println(porcentajeHumedadSuelo);

    // BOMBA
    if (porcentajeHumedadSuelo < 45 && modoSistema == AUTOMATICO )
    {
        encenderBomba();
    }
    else if (porcentajeHumedadSuelo > 55)
    {
        apagarBomba();
    }
}

// Configuracion inicial
void setup()
{

    // ==========================
    // CAMBIO AUTOMATICO/MANUAL
    // ==========================
    bool interruptorActivo =
        digitalRead(INTERRUPTOR_PIN);

    if (interruptorActivo &&
        modoSistema == AUTOMATICO)
    {
        modoSistema = MANUAL;

        tipoControl = CONTROL_LEJANO;

        Serial.println("MANUAL");

        bot.sendMessage(
            CHAT_ID,
            "🖐️ Modo MANUAL\n🌎 Control TELEGRAM",
            "");
    }

    else if (!interruptorActivo &&
             modoSistema == MANUAL)
    {
        modoSistema = AUTOMATICO;

        Serial.println("AUTOMATICO");

        bot.sendMessage(
            CHAT_ID,
            "🤖 Modo AUTOMATICO",
            "");
    }

    IrReceiver.begin(IR_RECEIVER_PIN, ENABLE_LED_FEEDBACK);

    Serial.println("Receptor IR listo");

    Serial.begin(115200);
    dht.begin();

    pinMode(INTERRUPTOR_PIN, INPUT_PULLDOWN);

    pinMode(MOTOR_ABRIR, OUTPUT);
    pinMode(MOTOR_CERRAR, OUTPUT);

    digitalWrite(MOTOR_ABRIR, LOW);
    digitalWrite(MOTOR_CERRAR, LOW);

    pinMode(MQ135, INPUT);
    pinMode(MQ135D, INPUT);

    pinMode(SENS_H, INPUT);
    pinMode(LDRA, INPUT);

    pinMode(IMP1_1, OUTPUT);
    pinMode(IMP1_2, OUTPUT);

    pinMode(VENTILADOR, OUTPUT);

    digitalWrite(VENTILADOR, HIGH);

    pinMode(BOMBA, OUTPUT);

    digitalWrite(BOMBA,LOW);

    digitalWrite(IMP1_1, LOW);
    digitalWrite(IMP1_2, LOW);

    pinMode(IR_sensor_pin, INPUT);

    pinMode(LUCES, OUTPUT);

    digitalWrite(LUCES, LOW);

    // OLED
    if (!display.begin(
            SSD1306_SWITCHCAPVCC,
            SCREEN_ADDRESS))
    {
        while (true)
            ;
    }

    display.clearDisplay();
    display.display();

    display.setFont(
        &FreeSans9pt7b);

    conectarWiFi();

    client.setCACert(TELEGRAM_CERTIFICATE_ROOT);

    bot.sendMessage(
        CHAT_ID,
        "🤖 ESP32 CONECTADO",
        "");

    bot.sendMessage(CHAT_ID, obtenerEstado(), "");

    bot.sendMessage(
        CHAT_ID,
        obtenerComandos(),
        "");
}

// Bucle principal
void loop()
{

    // ==========================
    // CAMBIO AUTOMATICO/MANUAL
    // ==========================
    bool interruptorActivo =
        digitalRead(INTERRUPTOR_PIN);

    if (interruptorActivo &&
        modoSistema == AUTOMATICO)
    {
        modoSistema = MANUAL;

        tipoControl = CONTROL_LEJANO;

        Serial.println("MANUAL");

        bot.sendMessage(
            CHAT_ID,
            "🖐️ Modo MANUAL\n🌎 Control TELEGRAM",
            "");
    }

    else if (!interruptorActivo &&
             modoSistema == MANUAL)
    {
        modoSistema = AUTOMATICO;

        Serial.println("AUTOMATICO");

        bot.sendMessage(
            CHAT_ID,
            "🤖 Modo AUTOMATICO",
            "");
    }

    if (IrReceiver.decode())
    {
        Serial.print("Codigo HEX: 0x");
        Serial.println( IrReceiver.decodedIRData.decodedRawData, HEX);

        IrReceiver.resume(); // Espera siguiente codigo
    }

    bool obstaculoActual =
        (digitalRead(IR_sensor_pin) == LOW);

    float humedad =
        dht.readHumidity();

    float temperatura =
        dht.readTemperature();

    valorAire =
        analogRead(MQ135);

    humedadSuelo =
        analogRead(SENS_H);

    valorLDR =
        analogRead(LDRA);

    porcentajeLuz =
        map(valorLDR,
            0,
            4095,
            0,
            100);

    display.clearDisplay();

    // ==========================
    // CONTROL AUTOMATICO LUCES
    // ==========================
    if (modoSistema == AUTOMATICO)
    {
        if (porcentajeLuz > 60)
        {
            encenderLuces();
        }
        else
        {
            apagarLuces();
        }
    }

    display.setTextSize(1);
    display.setTextColor(
        SSD1306_WHITE);

    display.setCursor(0, 12);
    display.printf(
        "T: %.1fC",
        temperatura);

    display.setCursor(0, 28);
    display.printf(
        "H: %.1f%%",
        humedad);

    display.setCursor(0, 44);

    if (modoSistema == AUTOMATICO)
    {
        funcionesAutomaticas();
    }

    if (valorAire < 200)
    {
        display.print("Aire normal");
    }
    else if (valorAire <= 400)
    {
        display.print("Aire contam.");
    }
    else
    {
        display.print("PELIGRO");
    }

    display.display();

    

    // =====================================
    // PUERTA AUTOMATICA CON SENSOR IR
    // =====================================

    if (obstaculoActual)
    {
        // Si la puerta está cerrada la abre
        if (!puertaAbiertaIR)
        {
            abrirPuerta();
            puertaAbiertaIR = true;

            Serial.println("Puerta abierta por sensor IR");
        }

        // Reinicia temporizador de cierre
        contandoCierre = false;
    }
    else
    {
        // Si ya no hay obstáculo
        if (puertaAbiertaIR)
        {
            // Inicia conteo una sola vez
            if (!contandoCierre)
            {
                tiempoSinObstaculo = millis();
                contandoCierre = true;
            }

            // Espera 3 segundos sin detectar nada
            if (millis() - tiempoSinObstaculo >= 3000)
            {
                cerrarPuerta();

                puertaAbiertaIR = false;
                contandoCierre = false;

                Serial.println("Puerta cerrada por sensor IR");
            }
        }
    }

    obstaculoAnterior =
        obstaculoActual;

    // ==========================
    // TELEGRAM
    // ==========================
    if (millis() - lastTimeBotRan >
        botRequestDelay)
    {
        int numNewMessages =
            bot.getUpdates(
                bot.last_message_received + 1);

        while (numNewMessages)
        {
            handleNewMessages(
                numNewMessages);

            numNewMessages =
                bot.getUpdates(
                    bot.last_message_received + 1);
        }

        lastTimeBotRan = millis();
    }

    
    // ==========================
    // CONTROL IR
    // ==========================
    if (modoSistema == MANUAL &&
        tipoControl == CONTROL_CERCANO)
    {
        if (IrReceiver.decode())
        {
            uint32_t codigo =
                IrReceiver.decodedIRData.decodedRawData;

            switch (codigo)
            {
            case COD1: abrirPuerta();
                
                break;
            
            case COD2: cerrarPuerta();
                
                break; 
            
            case COD3: abrirTecho();
                
                break;
            
            case COD4: cerrarTecho();
                
                break;
            
            case COD5:encenderVentilador();
                
                break;
            
            case COD6: apagarVentilador();
                
                break;
            
            case COD7: encenderBomba();
                
                break;
            
            case COD8: apagarBomba();
                
                break;
            
            case COD9:encenderLuces();
                
                break;
            
            case COD10:apagarLuces();
                
                break;
            
            default:
                break;
            }

            IrReceiver.resume();
        }
    }


    
}