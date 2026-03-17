// PBTransport.cpp
// HTTP transport layer: unified _request() dispatcher for ESP8266 and ESP32.

#include "PocketbaseExtended.h"

#if defined(ESP8266)
#  include <BearSSLHelpers.h>
#  include <ESP8266HTTPClient.h>
#  include <ESP8266WiFi.h>
#elif defined(ESP32)
#  include <HTTPClient.h>
#  include <WiFi.h>
#  include <WiFiClientSecure.h>
#endif

// ---------------------------------------------------------------------------
// Unified HTTP request (GET / POST / PATCH / DELETE)
// ---------------------------------------------------------------------------

PBResponse PocketbaseExtended::_request(const char* method,
                                         const String& url,
                                         const String& body) {
    PBResponse resp;
    resp.ok         = false;
    resp.statusCode = 0;
    resp.body       = "";
    resp.error      = "";

    if (_collection.length() == 0 &&
        url.indexOf("/collections/") == -1 &&
        url.indexOf("/health")       == -1) {
        resp.error = "No collection set.";
        return resp;
    }

    _debugPrint(String("[PB] ") + method + " " + url);

    bool isHttps = url.startsWith("https");
    int  httpCode = 0;

#if defined(ESP8266)
    HTTPClient              http;
    BearSSL::WiFiClientSecure secureClient;
    WiFiClient              plainClient;
    bool begun = false;

    if (isHttps) {
        if (_insecureTLS) secureClient.setInsecure();
        begun = http.begin(secureClient, url);
    } else {
        begun = http.begin(plainClient, url);
    }

    if (!begun) {
        resp.error = "Connection failed";
        _debugPrint("[PB] Connection failed");
        return resp;
    }

    http.setTimeout(_timeout);

    if (_authToken.length() > 0) {
        http.addHeader("Authorization", "Bearer " + _authToken);
    }

    if (strcmp(method, "GET") == 0) {
        httpCode = http.GET();
    } else if (strcmp(method, "DELETE") == 0) {
        http.addHeader("Content-Length", "0");
        httpCode = http.sendRequest("DELETE");
    } else if (strcmp(method, "POST") == 0) {
        http.addHeader("Content-Type", "application/json");
        httpCode = http.POST(body);
    } else if (strcmp(method, "PATCH") == 0) {
        http.addHeader("Content-Type", "application/json");
        httpCode = http.sendRequest("PATCH", body);
    } else {
        resp.error = "Unknown HTTP method";
        http.end();
        return resp;
    }

    if (httpCode > 0) {
        resp.statusCode = httpCode;
        resp.body       = http.getString();
        resp.ok         = (httpCode >= 200 && httpCode < 300);
        if (!resp.ok) resp.error = resp.body;
        _debugPrint(String("[PB] ") + httpCode + " " + resp.body);
    } else {
        resp.error = http.errorToString(httpCode);
        _debugPrint(String("[PB] Error: ") + resp.error);
    }

    http.end();

#elif defined(ESP32)
    HTTPClient       http;
    WiFiClientSecure secureClient;
    WiFiClient       plainClient;
    bool begun = false;

    if (isHttps) {
        if (_insecureTLS) secureClient.setInsecure();
        begun = http.begin(secureClient, url);
    } else {
        begun = http.begin(plainClient, url);
    }

    if (!begun) {
        resp.error = "Connection failed";
        _debugPrint("[PB] Connection failed");
        return resp;
    }

    http.setTimeout(_timeout);

    if (_authToken.length() > 0) {
        http.addHeader("Authorization", "Bearer " + _authToken);
    }

    if (strcmp(method, "GET") == 0) {
        httpCode = http.GET();
    } else if (strcmp(method, "DELETE") == 0) {
        http.addHeader("Content-Length", "0");
        httpCode = http.sendRequest("DELETE");
    } else if (strcmp(method, "POST") == 0) {
        http.addHeader("Content-Type", "application/json");
        httpCode = http.POST(body);
    } else if (strcmp(method, "PATCH") == 0) {
        http.addHeader("Content-Type", "application/json");
        httpCode = http.sendRequest("PATCH", body);
    } else {
        resp.error = "Unknown HTTP method";
        http.end();
        return resp;
    }

    if (httpCode > 0) {
        resp.statusCode = httpCode;
        resp.body       = http.getString();
        resp.ok         = (httpCode >= 200 && httpCode < 300);
        if (!resp.ok) resp.error = resp.body;
        _debugPrint(String("[PB] ") + httpCode + " " + resp.body);
    } else {
        resp.error = http.errorToString(httpCode);
        _debugPrint(String("[PB] Error: ") + resp.error);
    }

    http.end();

#else
    resp.error = "Unsupported platform";
#endif

    return resp;
}
