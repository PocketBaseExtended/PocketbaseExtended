// PBAuth.cpp
// Authentication, token management, and health check.

#include "PocketbaseExtended.h"

// ---------------------------------------------------------------------------
// Token management
// ---------------------------------------------------------------------------

void   PocketbaseExtended::setAuthToken(const String& token) { _authToken = token; }
String PocketbaseExtended::getAuthToken() const              { return _authToken;  }
void   PocketbaseExtended::clearAuthToken()                  { _authToken = "";    }

// ---------------------------------------------------------------------------
// Auth endpoints
// ---------------------------------------------------------------------------

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

    // Auto-extract token from response body (no ArduinoJson dependency)
    if (resp.ok) {
        int idx = resp.body.indexOf("\"token\":\"");
        if (idx != -1) {
            idx += 9; // skip past `"token":"`
            int end = resp.body.indexOf("\"", idx);
            if (end != -1) {
                _authToken = resp.body.substring(idx, end);
            }
        }
    }

    return resp;
}

PBResponse PocketbaseExtended::authRefresh() {
    if (_collection.length() == 0) {
        PBResponse r; r.ok = false; r.statusCode = 0;
        r.error = "No collection set. Call collection() first.";
        return r;
    }
    if (_authToken.length() == 0) {
        PBResponse r; r.ok = false; r.statusCode = 0;
        r.error = "No token to refresh. Call authWithPassword() first.";
        return r;
    }

    String url = _baseUrl + "collections/" + _collection + "/auth-refresh";
    PBResponse resp = _request("POST", url);

    // Auto-update stored token on success
    if (resp.ok) {
        int idx = resp.body.indexOf("\"token\":\"");
        if (idx != -1) {
            idx += 9;
            int end = resp.body.indexOf("\"", idx);
            if (end != -1) {
                _authToken = resp.body.substring(idx, end);
            }
        }
    }

    return resp;
}

// ---------------------------------------------------------------------------
// Health check
// ---------------------------------------------------------------------------

PBResponse PocketbaseExtended::checkHealth() {
    String url = _baseUrl + "health";
    return _request("GET", url);
}
