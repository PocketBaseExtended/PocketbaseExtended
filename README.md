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
- [PocketBase API Coverage](#pocketbase-api-coverage)
- [API Reference](#api-reference)
  - [Constructor](#constructor)
  - [collection()](#collection)
  - [Record Methods](#record-methods)
    - [getOneEx / getOne](#getoneex--getone)
    - [getListEx / getList](#getlistex--getlist)
    - [createEx / create](#createex--create)
    - [updateEx / update](#updateex--update)
    - [deleteRecordEx / deleteRecord](#deleterecordex--deleterecord)
  - [Authentication](#authentication)
  - [checkHealth](#checkhealth)
  - [getFileUrl](#getfileurl)
  - [Configuration](#configuration)
  - [PBResponse](#pbresponse)
- [Examples](#examples)
- [Error Handling](#error-handling)
- [Migration from v0.x](#migration-from-v0x)
- [Contributing](#contributing)
- [License](#license)

---

## PocketBase API Coverage

> Endpoints marked **—** are not applicable or not feasible on ESP8266/ESP32 and are intentionally excluded.

| Category | PocketBase Endpoint | Library Method | Notes |
|----------|---------------------|----------------|-------|
| **Records** | `GET /api/collections/{c}/records` | `getList` / `getListEx` | Pagination, sort, filter, expand, fields |
| | `GET /api/collections/{c}/records/{id}` | `getOne` / `getOneEx` | Expand, fields |
| | `POST /api/collections/{c}/records` | `create` / `createEx` | JSON body |
| | `PATCH /api/collections/{c}/records/{id}` | `update` / `updateEx` | Partial update |
| | `DELETE /api/collections/{c}/records/{id}` | `deleteRecord` / `deleteRecordEx` | |
| **Auth** | `POST .../auth-with-password` | `authWithPassword` | Token stored automatically |
| | `POST .../auth-refresh` | `authRefresh` | Token updated automatically |
| | `GET .../auth-methods` | — | Low value on headless IoT |
| | `POST .../auth-with-oauth2` | — | Requires browser redirect |
| | `POST .../request-password-reset` | — | User-facing email flow |
| | `POST .../confirm-password-reset` | — | User-facing email flow |
| | `POST .../request-verification` | — | User-facing email flow |
| | `POST .../confirm-verification` | — | User-facing email flow |
| | `POST .../request-email-change` | — | User-facing email flow |
| | `POST .../confirm-email-change` | — | User-facing email flow |
| **Files** | `GET /api/files/{c}/{recordId}/{file}` | `getFileUrl` | URL builder only — no HTTP call |
| | Multipart file upload | — | Deferred to v1.1 |
| **Health** | `GET /api/health` | `checkHealth` | Connectivity probe at startup |
| **Realtime** | `GET /api/realtime` (SSE) | — | Server-sent events not feasible on ESP |
| **Admin** | All `/api/admins/...` | — | Out of scope for IoT client |

---

## Installation

### Arduino IDE — Library Manager

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
// ESP32 — swap the line above for:
// #include <WiFi.h>

const char* ssid     = "YOUR_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

PocketbaseExtended pb("https://your-pocketbase-host.com");

void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) { delay(500); }

    String rec  = pb.collection("notes").getOne("RECORD_ID");
    String list = pb.collection("notes").getList("1", "10", "-created");
    String newR = pb.collection("notes").create("{\"title\":\"Hello\"}");
    String upd  = pb.collection("notes").update("RECORD_ID", "{\"title\":\"Updated\"}");
    String del  = pb.collection("notes").deleteRecord("RECORD_ID");
}
```

---

## API Reference

Every public method maps directly to a PocketBase REST endpoint.
All record methods exist in two forms:

| Variant | Return type | Use when |
|---------|-------------|----------|
| `methodEx()` | `PBResponse` | You need the status code or structured error |
| `method()` | `String` (body only) | Quick access; backward-compatible with v0.x |

---

### Constructor

```cpp
PocketbaseExtended pb(const char* baseUrl);
```

| Parameter | Type | Description |
|-----------|------|-------------|
| `baseUrl` | `const char*` | Root URL of the PocketBase instance, e.g. `"https://my-pb-host.com"`. Trailing slashes are normalised automatically. |

---

### collection()

```cpp
PocketbaseExtended& pb.collection(const char* name);
```

Sets the active collection for all subsequent requests. Returns `*this` for chaining.

```cpp
pb.collection("notes").getOne("RECORD_ID");
```

---

### Record Methods

#### getOneEx / getOne

> `GET /api/collections/{collection}/records/{id}`

```cpp
PBResponse getOneEx(const char* recordId,
                    const char* expand = nullptr,
                    const char* fields = nullptr);

String     getOne  (const char* recordId,
                    const char* expand = nullptr,
                    const char* fields = nullptr);
```

| Parameter  | Description |
|------------|-------------|
| `recordId` | **Required.** ID of the record to fetch. |
| `expand`   | Comma-separated relation fields to auto-expand, e.g. `"author,tags.name"`. Pass `nullptr` to skip. |
| `fields`   | Comma-separated fields to include in the response, e.g. `"id,title,created"`. Pass `nullptr` for all fields. |

```cpp
PBResponse r = pb.collection("notes").getOneEx("abc123", "author", "id,title,author");
```

---

#### getListEx / getList

> `GET /api/collections/{collection}/records`

```cpp
PBResponse getListEx(const char* page      = nullptr,
                     const char* perPage   = nullptr,
                     const char* sort      = nullptr,
                     const char* filter    = nullptr,
                     const char* skipTotal = nullptr,
                     const char* expand    = nullptr,
                     const char* fields    = nullptr);

String     getList  (const char* page      = nullptr,
                     const char* perPage   = nullptr,
                     const char* sort      = nullptr,
                     const char* filter    = nullptr,
                     const char* skipTotal = nullptr,
                     const char* expand    = nullptr,
                     const char* fields    = nullptr);
```

| Parameter   | Description |
|-------------|-------------|
| `page`      | Page number (1-based). Server default: `1`. |
| `perPage`   | Records per page (max `500`). Server default: `30`. |
| `sort`      | Sort expression. Prefix field with `-` for DESC, no prefix for ASC. Example: `"-created,id"`. |
| `filter`    | PocketBase filter expression. Example: `"active = true && views > 100"`. |
| `skipTotal` | Pass `"1"` to omit `totalItems`/`totalPages` from the response for faster queries. |
| `expand`    | Comma-separated relation fields to auto-expand. |
| `fields`    | Comma-separated fields to include in each record. |

Pass `nullptr` for any parameter you want to omit.

```cpp
// Page 1, 20 records, newest first, only active
PBResponse r = pb.collection("notes").getListEx("1", "20", "-created", "active = true");
```

---

#### createEx / create

> `POST /api/collections/{collection}/records`

```cpp
PBResponse createEx(const String& requestBody);
String     create  (const String& requestBody);
```

| Parameter     | Description |
|---------------|-------------|
| `requestBody` | JSON string of record fields to set, e.g. `"{\"title\":\"Hello\"}"`. |

```cpp
PBResponse r = pb.collection("notes").createEx("{\"title\":\"Hello\",\"active\":true}");
```

---

#### updateEx / update

> `PATCH /api/collections/{collection}/records/{id}`

```cpp
PBResponse updateEx(const char* recordId, const String& requestBody);
String     update  (const char* recordId, const String& requestBody);
```

| Parameter     | Description |
|---------------|-------------|
| `recordId`    | **Required.** ID of the record to update. |
| `requestBody` | JSON string of fields to change. Omitted fields retain their current values. |

```cpp
PBResponse r = pb.collection("notes").updateEx("abc123", "{\"title\":\"Updated\"}");
```

---

#### deleteRecordEx / deleteRecord

> `DELETE /api/collections/{collection}/records/{id}`

```cpp
PBResponse deleteRecordEx(const char* recordId);
String     deleteRecord  (const char* recordId);
```

| Parameter  | Description |
|------------|-------------|
| `recordId` | **Required.** ID of the record to delete. |

A successful delete returns HTTP `204` with an empty body (`PBResponse::ok == true`, `body == ""`).

```cpp
PBResponse r = pb.collection("notes").deleteRecordEx("abc123");
```

---

### Authentication

> `POST /api/collections/{collection}/auth-with-password`

```cpp
PBResponse authWithPassword(const char* identity, const char* password);
void       setAuthToken    (const String& token);
String     getAuthToken    () const;
void       clearAuthToken  ();
```

| Method | Description |
|--------|-------------|
| `authWithPassword(identity, password)` | Authenticate with email/username and password. On success, the JWT token is extracted and stored automatically. All subsequent requests include `Authorization: Bearer <token>`. |
| `authRefresh()` | Exchange the current token for a new one with a renewed expiry. Token is updated automatically on success. Call periodically on long-running devices. |
| `setAuthToken(token)` | Manually set a token (e.g. restored from persistent storage). |
| `getAuthToken()` | Return the currently stored token, or an empty `String` if not authenticated. |
| `clearAuthToken()` | Remove the stored token (effective logout). |

```cpp
// Login — token stored automatically
PBResponse auth = pb.collection("users").authWithPassword("user@example.com", "password");
if (auth.ok) {
    Serial.println("Token: " + pb.getAuthToken());
}

// All subsequent requests carry the token automatically
pb.collection("private_notes").getListEx();

// Refresh before expiry (e.g. call every few hours)
pb.collection("users").authRefresh();

// Logout
pb.clearAuthToken();
```

---

---

### checkHealth

> `GET /api/health`

```cpp
PBResponse checkHealth();
```

Does not require a collection to be set. Useful as a connectivity probe at device startup.

```cpp
PBResponse h = pb.checkHealth();
if (h.ok) Serial.println("PocketBase is reachable");
```

---

### getFileUrl

```cpp
String getFileUrl(const char* recordId,
                  const char* filename,
                  const char* thumb = nullptr);
```

URL builder only — **no HTTP request is made**. Returns the full URL for a file attachment on the active collection.

| Parameter  | Description |
|------------|-------------|
| `recordId` | ID of the record that owns the file. |
| `filename` | Exact filename as stored in the record field. |
| `thumb`    | Optional thumbnail size, e.g. `"100x100"` or `"0x50"`. Pass `nullptr` for the original file. |

```cpp
// Get file URL
String url = pb.collection("notes").getFileUrl("abc123", "photo.jpg");

// Get a thumbnail
String thumb = pb.collection("notes").getFileUrl("abc123", "photo.jpg", "100x100");
```

---

### Configuration

| Method | Default | Description |
|--------|---------|-------------|
| `setTimeout(ms)` | `10000` | HTTP request timeout in milliseconds. |
| `setInsecureTLS(enabled)` | `true` | When `true`, TLS certificate verification is skipped. Set to `false` in production with a valid certificate. |
| `setDebug(enabled)` | `false` | When `true`, request URLs, status codes, and response bodies are printed to `Serial`. |

```cpp
pb.setTimeout(15000);
pb.setInsecureTLS(false);
pb.setDebug(true);   // useful during development; disable before shipping
```

---

### PBResponse

Returned by all `Ex` methods.

| Field | Type | Description |
|-------|------|-------------|
| `ok` | `bool` | `true` when the HTTP status is 2xx. |
| `statusCode` | `int` | HTTP status code. `0` if the connection itself failed. |
| `body` | `String` | Raw JSON response body. |
| `error` | `String` | Human-readable error. Equals `body` on 4xx/5xx responses; connection error message when `statusCode == 0`. |

---

## Examples

| Sketch | Description |
|--------|-------------|
| `pocketbaseextended_example` | Quick-start CRUD |
| `pocketbaseextended_example_getOne` | `getOneEx()` with structured error handling |
| `pocketbaseextended_example_getList` | `getListEx()` with pagination, sorting, and filtering |
| `pocketbaseextended_example_deleteRecord` | Auth + authorized delete |
| `pocketbaseextended_example_auth` | Full auth workflow |

---

## Error Handling

```cpp
PBResponse resp = pb.collection("notes").getOneEx("BAD_ID");

if (resp.ok) {
    Serial.println(resp.body);
} else {
    // resp.statusCode == 404; resp.error contains the server error JSON
    Serial.println("HTTP " + String(resp.statusCode) + ": " + resp.error);
}
```

---

## Migration from v0.x

| v0.x | v1.0 |
|------|------|
| `PocketbaseArduino pb(...)` | `PocketbaseExtended pb(...)` — `PocketbaseArduino` still compiles as a typedef alias |
| `#include <BearSSLHelpers.h>` in sketch | No longer needed — included internally |
| `#include <ESP8266HTTPClient.h>` in sketch | No longer needed — included internally |
| `getList` wrote `skipTotal=<filter>` (bug) | Fixed — all query params map correctly |
| No `update()` | `update()` / `updateEx()` added |
| No auth | `authWithPassword()`, `setAuthToken()`, `clearAuthToken()` |
| Empty string on all errors | `PBResponse` with `ok`, `statusCode`, `body`, `error` |
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
