/*
    pocketbaseextended_example_auth.ino

    Demonstrates the full auth workflow:
      1. Login with email + password
      2. Token is stored automatically
      3. Subsequent requests carry the Bearer token

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

    // --- Login ---
    // authWithPassword() posts to /api/collections/{collection}/auth-with-password
    // and automatically extracts and stores the returned token.
    PBResponse auth = pb.collection("users").authWithPassword(
        "user@example.com",
        "yourpassword"
    );

    if (!auth.ok) {
        Serial.println("Login failed (" + String(auth.statusCode) + "): " + auth.error);
        return;
    }

    Serial.println("Logged in. Token: " + pb.getAuthToken());

    // --- Authorized GET ---
    // The stored token is automatically sent as Authorization: Bearer <token>
    PBResponse resp = pb.collection("private_notes").getListEx("1", "5");
    if (resp.ok) {
        Serial.println("Private notes: " + resp.body);
    } else {
        Serial.println("Fetch failed: " + resp.error);
    }

    // --- Create with auth ---
    PBResponse created = pb.collection("private_notes").createEx(
        "{\"title\":\"Secure note\",\"content\":\"Only visible when logged in\"}"
    );
    Serial.println("Created: " + created.body);

    // --- Logout (clear token) ---
    pb.clearAuthToken();
    Serial.println("Logged out");

    // --- Manual token management ---
    // pb.setAuthToken("previously_saved_token");
    // pb.getAuthToken(); // retrieve current token
}

void loop() {}
