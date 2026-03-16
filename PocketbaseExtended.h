// PocketbaseExtended.h

#ifndef PocketbaseExtended_h
#define PocketbaseExtended_h

#include "Arduino.h"

#if defined(ESP8266)
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <BearSSLHelpers.h>
#elif defined(ESP32)
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#endif

// Structured response returned by all Ex methods
struct PBResponse {
    bool ok;
    int statusCode;
    String body;
    String error;
};

class PocketbaseExtended {
public:
    explicit PocketbaseExtended(const char* baseUrl);

    // Select the active collection (chainable)
    PocketbaseExtended& collection(const char* name);

    // ---------- Extended methods (return PBResponse) ----------

    PBResponse getOneEx(const char* recordId,
                        const char* expand = nullptr,
                        const char* fields = nullptr);

    PBResponse getListEx(const char* page      = nullptr,
                         const char* perPage   = nullptr,
                         const char* sort      = nullptr,
                         const char* filter    = nullptr,
                         const char* skipTotal = nullptr,
                         const char* expand    = nullptr,
                         const char* fields    = nullptr);

    PBResponse createEx(const String& requestBody);

    PBResponse updateEx(const char* recordId, const String& requestBody);

    PBResponse deleteRecordEx(const char* recordId);

    // ---------- Convenience methods (return body String) ----------

    String getOne(const char* recordId,
                  const char* expand = nullptr,
                  const char* fields = nullptr);

    String getList(const char* page      = nullptr,
                   const char* perPage   = nullptr,
                   const char* sort      = nullptr,
                   const char* filter    = nullptr,
                   const char* skipTotal = nullptr,
                   const char* expand    = nullptr,
                   const char* fields    = nullptr);

    String create(const String& requestBody);

    String update(const char* recordId, const String& requestBody);

    String deleteRecord(const char* recordId);

    // ---------- Auth ----------

    // POST /api/collections/{collection}/auth-with-password
    // On success, automatically stores the returned token.
    PBResponse authWithPassword(const char* identity, const char* password);

    void   setAuthToken(const String& token);
    String getAuthToken() const;
    void   clearAuthToken();

    // ---------- Configuration ----------

    void setTimeout(uint32_t ms);
    void setInsecureTLS(bool enabled);
    void setDebug(bool enabled);

private:
    String   _baseUrl;      // e.g. "http://host/api/"
    String   _collection;
    String   _authToken;
    uint32_t _timeout;
    bool     _insecureTLS;
    bool     _debug;

    // Build /api/collections/{collection}/records[/recordId]
    String _buildRecordsUrl(const char* recordId = nullptr);

    // Append a key=value query param; skips if value is null/empty
    String _appendParam(const String& url, const char* key, const char* value);

    // Unified HTTP request helper
    PBResponse _request(const char* method,
                        const String& url,
                        const String& body = "");

    void _debugPrint(const String& msg);
};

// Backward-compatibility alias for sketches that used PocketbaseArduino
using PocketbaseArduino = PocketbaseExtended;

#endif
