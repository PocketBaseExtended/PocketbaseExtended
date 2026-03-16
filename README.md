# PocketbaseExtended

<div align="center">

![splash image](splash.png)

[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg)](CONTRIBUTING.md)
[![Open Source Love](https://badges.frapsoft.com/os/v1/open-source.svg?v=103)](https://opensource.org/)
![Static Badge](https://img.shields.io/badge/version-v1.0.0-%2300969B?color=%2300969B)
![Static Badge](https://img.shields.io/badge/Pocketbase-Pocketbase?logo=pocketbase&logoColor=%23000&labelColor=%23fff&color=%23fff)
![Static Badge](https://img.shields.io/badge/Arduino-Arduino?logo=arduino&logoColor=%23fff&labelColor=%2300969B&color=%2300969B)

</div>

A lightweight PocketBase client library for **ESP8266** and **ESP32** Arduino projects.
Supports full CRUD, authentication, structured error responses, and optional debug logging — with no heavy dependencies.

## Table of Contents

- [Installation](#installation)
- [Quick Start](#quick-start)
- [API Reference](#api-reference)
  - [Constructor & Collection](#constructor--collection)
  - [Record Methods](#record-methods)
  - [Authentication](#authentication)
  - [Configuration](#configuration)
  - [Structured Response (PBResponse)](#structured-response-pbresponse)
- [Examples](#examples)
- [Error Handling](#error-handling)
- [Migration from v0.x](#migration-from-v0x)
- [Contributing](#contributing)
- [License](#license)

---

## Installation

### Arduino IDE (Library Manager)
1. Open **Sketch → Include Library → Manage Libraries...**
2. Search for `PocketbaseExtended` and click **Install**.

### Manual
1. Download or clone this repository.
2. Copy the folder into your Arduino `libraries/` directory.
3. Restart the Arduino IDE.

---

## Quick Start

```cpp
#include <PocketbaseExtended.h>

// ESP8266
#include <ESP8266WiFi.h>
// ESP32 — comment the line above and uncomment:
// #include <WiFi.h>

const char* ssid     = "YOUR_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

PocketbaseExtended pb("https://your-pocketbase-host.com");

void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) { delay(500); }

    // Simple string convenience methods
    String rec  = pb.collection("notes").getOne("RECORD_ID");
    String list = pb.collection("notes").getList("1", "10", "-created");
    String newR = pb.collection("notes").create("{\"title\":\"Hello\"}");
    String upd  = pb.collection("notes").update("RECORD_ID", "{\"title\":\"Updated\"}");
    String del  = pb.collection("notes").deleteRecord("RECORD_ID");
}
```

---

## API Reference

### Constructor & Collection

```cpp
PocketbaseExtended pb(const char* baseUrl);
pb.collection(const char* name);  // chainable, returns *this
```

### Record Methods

All methods have two variants:
- **`Ex` variant** — returns a `PBResponse` struct with status code and error info.
- **Convenience variant** — returns the response body as a `String` (backward compatible).

```cpp
// Extended (recommended)
PBResponse getOneEx(const char* recordId,
                    const char* expand    = nullptr,
                    const char* fields    = nullptr);

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

// Convenience (returns body String)
String getOne(const char* recordId, const char* expand = nullptr, const char* fields = nullptr);
String getList(const char* page = nullptr, ...);  // same params as getListEx
String create(const String& requestBody);
String update(const char* recordId, const String& requestBody);
String deleteRecord(const char* recordId);
```

### Authentication

```cpp
// POST /api/collections/{collection}/auth-with-password
// Token is extracted and stored automatically on success.
PBResponse authWithPassword(const char* identity, const char* password);

void   setAuthToken(const String& token);  // manually set a stored token
String getAuthToken() const;               // retrieve current token
void   clearAuthToken();                   // logout / clear token
```

Once a token is set (via `authWithPassword` or `setAuthToken`), every subsequent request automatically includes:
```
Authorization: Bearer <token>
```

### Configuration

```cpp
pb.setTimeout(10000);       // request timeout in ms (default: 10000)
pb.setInsecureTLS(true);    // skip TLS certificate verification (default: true)
pb.setDebug(true);          // print request/response info to Serial (default: false)
```

### Structured Response (PBResponse)

```cpp
struct PBResponse {
    bool   ok;          // true if HTTP 2xx
    int    statusCode;  // HTTP status code (0 if connection failed)
    String body;        // raw response body
    String error;       // error message or response body on failure
};
```

---

## Examples

| Sketch | Description |
|--------|-------------|
| `pocketbaseextended_example` | Quick-start CRUD |
| `pocketbaseextended_example_getOne` | getOneEx with structured error handling |
| `pocketbaseextended_example_getList` | getListEx with pagination, sorting, and filtering |
| `pocketbaseextended_example_deleteRecord` | Auth + authorized delete |
| `pocketbaseextended_example_auth` | Full auth workflow |

---

## Error Handling

```cpp
PBResponse resp = pb.collection("notes").getOneEx("BAD_ID");

if (resp.ok) {
    Serial.println(resp.body);
} else {
    // resp.statusCode == 404, resp.error contains the server error body
    Serial.println("HTTP " + String(resp.statusCode) + ": " + resp.error);
}
```

---

## Migration from v0.x

| v0.x | v1.0 |
|------|------|
| `PocketbaseArduino pb(...)` | `PocketbaseExtended pb(...)` — `PocketbaseArduino` still works as a typedef alias |
| `#include <BearSSLHelpers.h>` in sketch | No longer needed — handled internally |
| `#include <ESP8266HTTPClient.h>` in sketch | No longer needed — handled internally |
| `getList` filter bug (wrote `skipTotal=<filter>`) | Fixed — all query params now map correctly |
| No `update()` method | `update()` / `updateEx()` now available |
| No auth support | `authWithPassword()`, `setAuthToken()`, `clearAuthToken()` |
| No structured errors — empty string on failure | `PBResponse` with `ok`, `statusCode`, `body`, `error` |
| Debug logging always on | Off by default; enable with `pb.setDebug(true)` |

---

## Contributing

1. [Fork](https://github.com/jeoooo/PocketbaseArduino/fork) this repository.
2. Create a feature branch: `git checkout -b my-feature`.
3. Commit your changes: `git commit -am 'Add feature'`.
4. Push and open a pull request.

---

## License

GPL-3.0 license
