// PocketbaseExtended.cpp

#include "PocketbaseExtended.h"

#if defined(ESP8266)
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <BearSSLHelpers.h>
#elif defined(ESP32)
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#endif

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------

PocketbaseExtended::PocketbaseExtended(const char* baseUrl)
    : _timeout(10000), _insecureTLS(true), _debug(false)
{
    _baseUrl = baseUrl;

    // Normalise trailing slash then append /api/
    if (_baseUrl.endsWith("/")) {
        _baseUrl.remove(_baseUrl.length() - 1);
    }
    _baseUrl += "/api/";
}

// ---------------------------------------------------------------------------
// Collection selector
// ---------------------------------------------------------------------------

PocketbaseExtended& PocketbaseExtended::collection(const char* name) {
    _collection = name;
    return *this;
}

// ---------------------------------------------------------------------------
// Configuration
// ---------------------------------------------------------------------------

void PocketbaseExtended::setTimeout(uint32_t ms)      { _timeout      = ms;      }
void PocketbaseExtended::setInsecureTLS(bool enabled) { _insecureTLS  = enabled; }
void PocketbaseExtended::setDebug(bool enabled)       { _debug        = enabled; }

// ---------------------------------------------------------------------------
// Auth
// ---------------------------------------------------------------------------

void PocketbaseExtended::setAuthToken(const String& token) { _authToken = token; }
String PocketbaseExtended::getAuthToken() const            { return _authToken;  }
void PocketbaseExtended::clearAuthToken()                  { _authToken = "";    }

PBResponse PocketbaseExtended::authWithPassword(const char* identity, const char* password) {
    if (_collection.length() == 0) {
        PBResponse r; r.ok = false; r.statusCode = 0;
        r.error = "No collection set. Call collection() first.";
        return r;
    }

    String url  = _baseUrl + "collections/" + _collection + "/auth-with-password";
    String body = "{\"identity\":\"";
    body += identity;
    body += "\",\"password\":\"";
    body += password;
    body += "\"}";

    PBResponse resp = _request("POST", url, body);

    // Auto-extract token from response body without ArduinoJson
    if (resp.ok) {
        int idx = resp.body.indexOf("\"token\":\"");
        if (idx != -1) {
            idx += 9; // skip past "token":"
            int end = resp.body.indexOf("\"", idx);
            if (end != -1) {
                _authToken = resp.body.substring(idx, end);
            }
        }
    }

    return resp;
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

void PocketbaseExtended::_debugPrint(const String& msg) {
    if (_debug) Serial.println(msg);
}

String PocketbaseExtended::_buildRecordsUrl(const char* recordId) {
    String url = _baseUrl + "collections/" + _collection + "/records";
    if (recordId != nullptr && strlen(recordId) > 0) {
        url += "/";
        url += recordId;
    }
    return url;
}

String PocketbaseExtended::_appendParam(const String& url, const char* key, const char* value) {
    if (value == nullptr || strlen(value) == 0) return url;
    String result = url;
    result += (result.indexOf('?') == -1) ? "?" : "&";
    result += key;
    result += "=";
    result += value;
    return result;
}

// ---------------------------------------------------------------------------
// Unified HTTP request (ESP8266 + ESP32, HTTP + HTTPS)
// ---------------------------------------------------------------------------

PBResponse PocketbaseExtended::_request(const char* method, const String& url, const String& body) {
    PBResponse resp;
    resp.ok         = false;
    resp.statusCode = 0;
    resp.body       = "";
    resp.error      = "";

    if (_collection.length() == 0 &&
        url.indexOf("/auth-with-password") == -1 &&
        url.indexOf("/collections/") == -1) {
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

// ---------------------------------------------------------------------------
// Extended record methods (return PBResponse)
// ---------------------------------------------------------------------------

PBResponse PocketbaseExtended::getOneEx(const char* recordId,
                                         const char* expand,
                                         const char* fields) {
    if (recordId == nullptr || strlen(recordId) == 0) {
        PBResponse r; r.ok = false; r.statusCode = 0;
        r.error = "recordId is required";
        return r;
    }
    String url = _buildRecordsUrl(recordId);
    url = _appendParam(url, "expand", expand);
    url = _appendParam(url, "fields", fields);
    return _request("GET", url);
}

PBResponse PocketbaseExtended::getListEx(const char* page,
                                          const char* perPage,
                                          const char* sort,
                                          const char* filter,
                                          const char* skipTotal,
                                          const char* expand,
                                          const char* fields) {
    String url = _buildRecordsUrl();
    url = _appendParam(url, "page",      page);
    url = _appendParam(url, "perPage",   perPage);
    url = _appendParam(url, "sort",      sort);
    url = _appendParam(url, "filter",    filter);
    url = _appendParam(url, "skipTotal", skipTotal);
    url = _appendParam(url, "expand",    expand);
    url = _appendParam(url, "fields",    fields);
    return _request("GET", url);
}

PBResponse PocketbaseExtended::createEx(const String& requestBody) {
    String url = _buildRecordsUrl();
    return _request("POST", url, requestBody);
}

PBResponse PocketbaseExtended::updateEx(const char* recordId, const String& requestBody) {
    if (recordId == nullptr || strlen(recordId) == 0) {
        PBResponse r; r.ok = false; r.statusCode = 0;
        r.error = "recordId is required";
        return r;
    }
    String url = _buildRecordsUrl(recordId);
    return _request("PATCH", url, requestBody);
}

PBResponse PocketbaseExtended::deleteRecordEx(const char* recordId) {
    if (recordId == nullptr || strlen(recordId) == 0) {
        PBResponse r; r.ok = false; r.statusCode = 0;
        r.error = "recordId is required";
        return r;
    }
    String url = _buildRecordsUrl(recordId);
    return _request("DELETE", url);
}

// ---------------------------------------------------------------------------
// Convenience wrappers (return body String for backward compatibility)
// ---------------------------------------------------------------------------

String PocketbaseExtended::getOne(const char* recordId,
                                   const char* expand,
                                   const char* fields) {
    return getOneEx(recordId, expand, fields).body;
}

String PocketbaseExtended::getList(const char* page,
                                    const char* perPage,
                                    const char* sort,
                                    const char* filter,
                                    const char* skipTotal,
                                    const char* expand,
                                    const char* fields) {
    return getListEx(page, perPage, sort, filter, skipTotal, expand, fields).body;
}

String PocketbaseExtended::create(const String& requestBody) {
    return createEx(requestBody).body;
}

String PocketbaseExtended::update(const char* recordId, const String& requestBody) {
    return updateEx(recordId, requestBody).body;
}

String PocketbaseExtended::deleteRecord(const char* recordId) {
    return deleteRecordEx(recordId).body;
}
