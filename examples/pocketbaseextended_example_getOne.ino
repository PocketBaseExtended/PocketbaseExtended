/*
    pocketbaseextended_example_getOne.ino

    Demonstrates getOneEx() with structured error handling.
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

    // Structured response with full error info
    PBResponse resp = pb.collection("notes").getOneEx("RECORD_ID");

    if (resp.ok) {
        Serial.println("Success (" + String(resp.statusCode) + "):");
        Serial.println(resp.body);
    } else {
        Serial.println("Error (" + String(resp.statusCode) + "): " + resp.error);
    }

    // With optional expand and fields
    PBResponse resp2 = pb.collection("notes").getOneEx(
        "RECORD_ID",
        "author",          // expand this relation
        "id,title,author"  // return only these fields
    );
    Serial.println(resp2.body);
}

void loop() {
    PBResponse resp = pb.collection("notes").getOneEx("RECORD_ID");
    if (resp.ok) {
        Serial.println(resp.body);
    } else {
        Serial.println("Fetch failed: " + resp.error);
    }
    delay(10000);
}
