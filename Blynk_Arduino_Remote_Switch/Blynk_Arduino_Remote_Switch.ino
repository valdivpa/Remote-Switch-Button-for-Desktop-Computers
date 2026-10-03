// Must come before BlynkSimpleWifi.h (defines BLYNK_TEMPLATE_ID)
#include "arduino_secrets.h"

#define BLYNK_PRINT Serial

#include <WiFiS3.h>
#include <BlynkSimpleWifi.h>

#include "Arduino_LED_Matrix.h"

#include "on_bitmap.h"
#include "off_bitmap.h"
#include "no_key.h"

#include "wifi_10.h"
#include "wifi_20.h"
#include "wifi_50.h"
#include "wifi_100.h"

#include "wifi_animation.h"

#include "blynk_animation.h"
#include "blynk_unlocked.h"

#include "cloud_animation.h"

ArduinoLEDMatrix matrix;

// --------------------------------------------------
// Cloud connection animation
// --------------------------------------------------

// Connecting to blynk.cloud blocks inside WiFiClient::connect(), so
// cloud_animation is played by the matrix timer interrupt instead of
// the loop. Frame duration is the 4th value of each frame in
// cloud_animation.h. Any later loadFrame() call stops it.
class CloudAnimatedClient : public WiFiClient
{
public:
    int connect(IPAddress ip, uint16_t port) override
    {
        startCloudAnimation();

        return WiFiClient::connect(ip, port);
    }

    int connect(const char* host, uint16_t port) override
    {
        startCloudAnimation();

        return WiFiClient::connect(host, port);
    }

private:
    void startCloudAnimation()
    {
        matrix.loadSequence(cloud_animation);
        matrix.play(true);
    }
};

CloudAnimatedClient cloudClient;

#define RELAY_PIN 2

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

unsigned long lastSignalUpdate = 0;

// --------------------------------------------------
// WiFi status helper
// --------------------------------------------------

const char* wifiStatusToString(int status)
{
    switch (status)
    {
        case WL_IDLE_STATUS:
            return "IDLE";

        case WL_NO_SSID_AVAIL:
            return "NO_SSID_AVAILABLE";

        case WL_SCAN_COMPLETED:
            return "SCAN_COMPLETED";

        case WL_CONNECTED:
            return "CONNECTED";

        case WL_CONNECT_FAILED:
            return "CONNECT_FAILED";

        case WL_CONNECTION_LOST:
            return "CONNECTION_LOST";

        case WL_DISCONNECTED:
            return "DISCONNECTED";

        default:
            return "UNKNOWN";
    }
}

// --------------------------------------------------
// Display WiFi signal strength
// --------------------------------------------------

void showWiFiStrength()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        return;
    }

    long rssi = WiFi.RSSI();

    Serial.print("RSSI: ");
    Serial.println(rssi);

    if (rssi >= -55)
    {
        matrix.loadFrame(WiFi_100[0]);
    }
    else if (rssi >= -67)
    {
        matrix.loadFrame(WiFi_50[0]);
    }
    else if (rssi >= -75)
    {
        matrix.loadFrame(WiFi_20[0]);
    }
    else
    {
        matrix.loadFrame(WiFi_10[0]);
    }
}

// --------------------------------------------------
// Successful connection animation
// --------------------------------------------------

void connectionSuccessAnimation()
{
    for (int i = 0; i < 2; i++)
    {
        matrix.loadFrame(WiFi_100[0]);
        delay(250);

        matrix.loadFrame(OFF_BITMAP);
        delay(250);
    }

    showWiFiStrength();
}

// --------------------------------------------------
// Fatal Blynk auth/configuration failure
// --------------------------------------------------

void fatalBlynkError()
{
    Serial.println("Invalid Blynk configuration");

    matrix.loadFrame(no_key[0]);

    while (true)
    {
        delay(1000);
    }
}

// --------------------------------------------------
// WiFi connection
// --------------------------------------------------

void connectWiFi()
{
    Serial.println("Connecting to WiFi...");

    WiFi.disconnect();

    delay(1000);

    unsigned long lastAttempt = 0;

    while (WiFi.status() != WL_CONNECTED)
    {
        // Start a new connection attempt every 10 seconds
        if (millis() - lastAttempt >= 10000)
        {
            lastAttempt = millis();

            Serial.println("Starting new connection attempt");

            WiFi.begin(ssid, pass);
        }

        // Run animation continuously
        for (int i = 0; i < 4; i++)
        {
            matrix.loadFrame(WiFi_Animation[i]);

            delay(250);

            if (WiFi.status() == WL_CONNECTED)
            {
                Serial.println("WiFi connected");

                connectionSuccessAnimation();

                return;
            }
        }

        // Debug output
        Serial.print("WiFi status: ");
        Serial.println(
            wifiStatusToString(
                WiFi.status()
            )
        );
    }
}

// --------------------------------------------------
// Blynk animations
// --------------------------------------------------

void animateBlynk()
{
    static unsigned long lastFrame = 0;
    static int frame = 0;

    const int frameCount =
        sizeof(blynk_animation) / sizeof(blynk_animation[0]);

    if (millis() - lastFrame < 250)
    {
        return;
    }

    lastFrame = millis();

    matrix.loadFrame(
        blynk_animation[frame]
    );

    frame++;

    if (frame >= frameCount)
    {
        frame = 0;
    }
}

void blynkUnlockedAnimation()
{
    matrix.loadFrame(blynk_unlocked[0]);
    delay(500);

    matrix.loadFrame(blynk_unlocked[1]);
    delay(1000);

    showWiFiStrength();
}

// --------------------------------------------------
// Blynk authentication
// --------------------------------------------------

// Non-blocking: Blynk.run() retries the connection on its own
// every 5 seconds while the authentication animation plays.
void updateBlynk()
{
    static bool wasConnected = false;

    Blynk.run();

    if (Blynk.isTokenInvalid())
    {
        fatalBlynkError();
    }

    if (Blynk.connected())
    {
        if (!wasConnected)
        {
            wasConnected = true;

            Serial.println("Blynk connected");

            blynkUnlockedAnimation();

            lastSignalUpdate = millis();
        }

        return;
    }

    if (wasConnected)
    {
        wasConnected = false;

        Serial.println("Blynk disconnected, authenticating...");
    }

    animateBlynk();
}

void connectBlynk()
{
    Serial.println("Authenticating with Blynk...");

    while (!Blynk.connected() && WiFi.status() == WL_CONNECTED)
    {
        updateBlynk();
    }
}

// --------------------------------------------------
// WiFi animation
// --------------------------------------------------

void animateWiFi()
{
    static unsigned long lastFrame = 0;
    static int frame = 0;

    if (millis() - lastFrame < 250)
    {
        return;
    }

    lastFrame = millis();

    matrix.loadFrame(
        WiFi_Animation[frame]
    );

    frame++;

    if (frame >= 4)
    {
        frame = 0;
    }
}

// --------------------------------------------------
// Relay action
// --------------------------------------------------

void pressPowerButton()
{
    if (!Blynk.connected())
    {
        Serial.println("Blynk offline");
        return;
    }

    Serial.println("Power button pressed");

    matrix.loadFrame(ON_BITMAP);

    digitalWrite(RELAY_PIN, HIGH);

    delay(500);

    digitalWrite(RELAY_PIN, LOW);

    showWiFiStrength();
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

// --------------------------------------------------
// Setup
// --------------------------------------------------

void setup()
{
    Serial.begin(115200);

    matrix.begin();

    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);

    // WiFiS3's WiFi.begin() blocks up to this timeout waiting for a
    // connection, freezing the animation. 0 makes it return immediately;
    // the connection is tracked by polling WiFi.status() instead.
    WiFi.setTimeout(0);

    connectWiFi();

    _blynkTransport.setClient(&cloudClient);

    // An invalid auth token stops the program forever
    // (handled in updateBlynk()).
    Blynk.config(BLYNK_AUTH_TOKEN);

    connectBlynk();
}

// --------------------------------------------------
// Main loop
// --------------------------------------------------

void loop()
{
    static unsigned long debugTimer = 0;
    static unsigned long lastWifiAttempt = 0;

    // Debug output every 5 seconds
    if (millis() - debugTimer > 5000)
    {
        debugTimer = millis();

        Serial.print("WiFi Status = ");
        Serial.println(
            wifiStatusToString(
                WiFi.status()
            )
        );

        Serial.print("Blynk Status = ");
        Serial.println(
            Blynk.connected()
                ? "CONNECTED"
                : "DISCONNECTED"
        );
    }

    // WiFi disconnected
    if (WiFi.status() != WL_CONNECTED)
    {
        // Blynk.connected() is only a flag updated inside Blynk.run(),
        // which doesn't run while WiFi is down. connect(0) closes the
        // socket and marks the session as connecting without blocking,
        // so it re-authenticates once WiFi is back.
        if (Blynk.connected())
        {
            Blynk.connect(0);
        }

        animateWiFi();

        // New connection attempt every 10 seconds
        if (millis() - lastWifiAttempt >= 10000)
        {
            lastWifiAttempt = millis();

            Serial.println(
                "Starting new connection attempt"
            );

            WiFi.disconnect();

            WiFi.begin(
                ssid,
                pass
            );
        }

        return;
    }

    // Blynk (re)authentication with animation
    updateBlynk();

    // Refresh RSSI display only while the Blynk animation isn't playing
    if (Blynk.connected() && millis() - lastSignalUpdate >= 10000)
    {
        lastSignalUpdate = millis();

        showWiFiStrength();
    }
}