/*
    pocketbaseextended_example_getList.ino

    Demonstrates getListEx() with pagination, sorting, and filtering.
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

    // Page 1, 10 per page, sorted newest first
    PBResponse resp = pb.collection("notes").getListEx(
        "1",        // page
        "10",       // perPage
        "-created", // sort: newest first
        nullptr,    // filter
        nullptr,    // skipTotal
        nullptr,    // expand
        nullptr     // fields
    );

    if (resp.ok) {
        Serial.println(resp.body);
    } else {
        Serial.println("Error: " + resp.error);
    }

    // With filter — only records where active = true
    PBResponse filtered = pb.collection("notes").getListEx(
        "1", "20", nullptr,
        "active = true",  // filter expression
        nullptr, nullptr, nullptr
    );
    Serial.println(filtered.body);

    // Skip total count for faster queries
    PBResponse fast = pb.collection("notes").getListEx(
        "1", "30", "-created",
        nullptr, "1",  // skipTotal=1
        nullptr, nullptr
    );
    Serial.println(fast.body);
}

void loop() {
    PBResponse resp = pb.collection("notes").getListEx("1", "10", "-created");
    if (resp.ok) {
        Serial.println(resp.body);
    }
    delay(10000);
}
