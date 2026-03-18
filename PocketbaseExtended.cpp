// PocketbaseExtended.cpp
// Core: constructor, collection selector, configuration, debug output.

#include "PocketbaseExtended.h"

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

void PocketbaseExtended::setTimeout(uint32_t ms)      { _timeout     = ms;      }
void PocketbaseExtended::setInsecureTLS(bool enabled) { _insecureTLS = enabled; }
void PocketbaseExtended::setDebug(bool enabled)       { _debug       = enabled; }

// ---------------------------------------------------------------------------
// Internal: debug output
// ---------------------------------------------------------------------------

void PocketbaseExtended::_debugPrint(const String& msg) {
    if (_debug) Serial.println(msg);
}
