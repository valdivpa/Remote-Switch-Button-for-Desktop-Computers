// Must come before BlynkSimpleWifi.h (defines BLYNK_TEMPLATE_ID)
#include "arduino_secrets.h"

#define BLYNK_PRINT Serial

#include <WiFiS3.h>
#include <BlynkSimpleWifi.h>

#define RELAY_PIN 2

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

BLYNK_WRITE(V0)
{
    if (param.asInt() == 1)
    {
        digitalWrite(RELAY_PIN, HIGH);

        delay(500);

        digitalWrite(RELAY_PIN, LOW);
    }
}

void setup()
{
    Serial.begin(115200);

    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);

    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
}

void loop()
{
    Blynk.run();
}
