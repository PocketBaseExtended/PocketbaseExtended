/*
    pocketbaseextended_example_deleteRecord.ino

    Demonstrates authentication followed by an authorized delete.
    Works on ESP8266 and ESP32.

    https://github.com/jeoooo/PocketbaseExtended
*/

#include <PocketbaseExtended.h>

// ---- ESP8266 ----
#include <ESP8266WiFi.h>

// ---- ESP32 (comment out the ESP8266 line above and uncomment this) ----
// #include <WiFi.h>

const char* ssid     = "YOUR_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

PocketbaseExtended pb("https://YOUR_POCKETBASE_HOST");

void setup() {
    Serial.begin(115200);

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected");

    // Authenticate first (token is stored automatically on success)
    PBResponse auth = pb.collection("users").authWithPassword("user@example.com", "yourpassword");

    if (!auth.ok) {
        Serial.println("Auth failed: " + auth.error);
        return;
    }
    Serial.println("Authenticated. Token: " + pb.getAuthToken());

    // Now delete a record — the bearer token is sent automatically
    PBResponse del = pb.collection("notes").deleteRecordEx("RECORD_ID");

    if (del.ok) {
        Serial.println("Deleted (status " + String(del.statusCode) + ")");
    } else {
        Serial.println("Delete failed (" + String(del.statusCode) + "): " + del.error);
    }
}

void loop() {}
