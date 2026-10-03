// Must come before BlynkSimpleWifi.h (defines BLYNK_TEMPLATE_ID)
#include "arduino_secrets.h"

#define BLYNK_PRINT Serial

#include <WiFiS3.h>
#include <BlynkSimpleWifi.h>

#define RELAY_PIN 2

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

// --------------------------------------------------
// WiFi connection
// --------------------------------------------------

void connectWiFi()
{
    Serial.println("Connecting to WiFi...");

    while (WiFi.status() != WL_CONNECTED)
    {
        WiFi.begin(ssid, pass);

        if (WiFi.status() != WL_CONNECTED)
        {
            Serial.println("Retrying in 5 seconds...");
            delay(5000);
        }
    }

    Serial.println("WiFi connected");
}

// --------------------------------------------------
// Relay action
// --------------------------------------------------

void pressPowerButton()
{
    Serial.println("Power button pressed");

    digitalWrite(RELAY_PIN, HIGH);
    delay(500);
    digitalWrite(RELAY_PIN, LOW);
}

// --------------------------------------------------
// Blynk V0 handler
// --------------------------------------------------

BLYNK_WRITE(V0)
{
    if (param.asInt() == 1)
    {
        pressPowerButton();
    }
}

BLYNK_CONNECTED()
{
    Serial.println("Blynk connected");
}

BLYNK_DISCONNECTED()
{
    Serial.println("Blynk disconnected");
}

// --------------------------------------------------
// Setup
// --------------------------------------------------

void setup()
{
    Serial.begin(115200);

    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);

    connectWiFi();

    Blynk.config(BLYNK_AUTH_TOKEN);
    Blynk.connect();
}

// --------------------------------------------------
// Main loop
// --------------------------------------------------

void loop()
{
    // BlynkSimpleWifi only connects WiFi once, so reconnection
    // after a WiFi drop has to be handled here.
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("WiFi lost");

        // Marks the Blynk session as disconnected so it
        // re-authenticates once WiFi is back.
        Blynk.connect(0);

        connectWiFi();
    }

    if (Blynk.isTokenInvalid())
    {
        Serial.println("Invalid Blynk auth token");

        while (true)
        {
            delay(1000);
        }
    }

    // Also retries the Blynk connection on its own if it drops
    Blynk.run();
}
