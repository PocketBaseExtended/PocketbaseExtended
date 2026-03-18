/*
    pocketbaseextended_example.ino

    Quick-start: CRUD operations with PocketbaseExtended.
    Works on both ESP8266 and ESP32 — uncomment the correct WiFi headers below.

    https://github.com/jeoooo/PocketbaseExtended
*/

#include <PocketbaseExtended.h>

// ---- ESP8266 ----
#include <ESP8266WiFi.h>

// ---- ESP32 (comment out the ESP8266 lines above and uncomment these) ----
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

    // Optional: enable debug logging during development
    // pb.setDebug(true);

    // ----- getOne -----
    String rec = pb.collection("notes").getOne("RECORD_ID");
    Serial.println("getOne: " + rec);

    // ----- getList -----
    String list = pb.collection("notes").getList("1", "10", "-created", nullptr, nullptr, nullptr, nullptr);
    Serial.println("getList: " + list);

    // ----- create -----
    String created = pb.collection("notes").create("{\"title\":\"Hello\",\"body\":\"from ESP\"}");
    Serial.println("create: " + created);

    // ----- update -----
    String updated = pb.collection("notes").update("RECORD_ID", "{\"title\":\"Updated\"}");
    Serial.println("update: " + updated);

    // ----- delete -----
    String deleted = pb.collection("notes").deleteRecord("RECORD_ID");
    Serial.println("delete: " + deleted);
}

void loop() {
    // Poll every 10 seconds
    String list = pb.collection("notes").getList(nullptr, nullptr, "-created");
    Serial.println(list);
    delay(10000);
}
