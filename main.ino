/////////////////////////////////////////////////////////////////
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "ESPRotary.h";

/////////////////////////////////////////////////////////////////

#define ROTARY_PIN1	12
#define ROTARY_PIN2	13

/////////////////////////////////////////////////////////////////

void showDirection(ESPRotary& r);
void rotate(ESPRotary& r);

/////////////////////////////////////////////////////////////////
WiFiClient mainESP;
PubSubClient MQTT(mainESP);
ESPRotary r = ESPRotary(ROTARY_PIN1, ROTARY_PIN2, 4, 0, 40);

const char *ssid = "TP-Link_1526"; // WiFi SSID
const char *password = "00102698"; // WiFi password
const char *mqtt_server = "192.168.1.25";

int position = 0;
/////////////////////////////////////////////////////////////////

void setup() {
    Serial.begin(115200);
    delay(50);
    pinMode(5, OUTPUT);
    analogWriteFreq(30000);
    Serial.println("\n\nSimple Counter");
    r.setChangedHandler(rotate);

    WiFi.begin(ssid, password);

    int _try = 0;
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print("..");
        delay(500);
        _try++;
    }
    Serial.println("\n[WiFi] Connected to the WiFi network");
    Serial.print("[WiFi] IP address: ");
    Serial.println(WiFi.localIP());

    MQTT.setServer(mqtt_server, 1883);
    MQTT.setCallback(callback);

    while (!MQTT.connected())
    {
        Serial.println("[MQTT] Connecting to MQTT...");

        if (MQTT.connect("ESP32Client"))
        {

            Serial.println("[MQTT] connected");
        }
        else
        {

            Serial.print("failed with state ");
            Serial.print(MQTT.state());
            delay(2000);
        }
    }

    Serial.println("[MQTT] Sub to cmnd/CuisineExterieur/dimmer");
    Serial.println("[MQTT] Sub to cmnd/CuisineExterieur/power");
    MQTT.subscribe("cmnd/CuisineExterieur/dimmer");
    MQTT.subscribe("cmnd/CuisineExterieur/power");

}

void loop() {
    r.loop();
    MQTT.loop();
}

/////////////////////////////////////////////////////////////////

// on change
void rotate(ESPRotary& r) {

    char charPos[10];

    position = 25*r.getPosition();

    itoa(position, charPos, 10);

    if (position == 0)
    {
        Serial.println("[Rotary] Turn Off LED Strip");
        analogWrite(5, 0);
        MQTT.publish("stat/CuisineExterieur/dimmer", charPos);
        MQTT.publish("stat/CuisineExterieur/power", "OFF");

    }
    else
    {
        // Serial.println(position);
        analogWrite(5, position);
        Serial.print("[Rotary] PWM set to : ");
        Serial.println(position);
        MQTT.publish("stat/CuisineExterieur/dimmer", charPos);
        MQTT.publish("stat/CuisineExterieur/power", "ON");
    }
}

// on left or right rotattion
void showDirection(ESPRotary& r) {
    Serial.println(r.directionToString(r.getDirection()));
}

/////////////////////////////////////////////////////////////////

void callback(char *topic, byte *payload, unsigned int length)
{
    char msg[10];
    int i = 0;
    int newPos = 0;

    Serial.print("[MQTT Callback] Message arrived in topic: ");
    Serial.println(topic);

    Serial.print("[MQTT Callback] Message : ");
    for (i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
        msg[i] = (char)payload[i];
    }
    Serial.println("");

    msg[i] = '\0';

    if (strcmp(topic, "cmnd/CuisineExterieur/dimmer") == 0)
    {
        newPos = map(atoi(msg), 0, 100, 0, 40);

        Serial.println(newPos);

        r.setPositon(newPos);

        Serial.print("[Callback] New position of ESPRotary : ");
        Serial.println(r.getPosition());

        // position = 25*r.getPosition();
        // analogWrite(5, position);

        // Serial.print("[Callback] PWM set to : ");
        // Serial.println(position);




    }

    else
    {
        if (strcmp(msg, "OFF") == 0)
        {
            analogWrite(5, 0);
            Serial.println("Light turned off");
        }

        else
        {
            analogWrite(5, position);
            Serial.println("Light turned on");
            Serial.print("PWM set to : ");
            Serial.println(position);


        }

    }





}