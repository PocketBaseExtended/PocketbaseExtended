# Changelog

## [1.0.0] - 2026-03-18

### Overview

v1.0.0 is a complete rewrite of the library, previously known as `PocketbaseArduino`. The API has been significantly expanded with a structured response type, a full authentication system, new endpoints, and a multi-file architecture. Existing v0.x sketches remain source-compatible via a typedef alias.

---

### Breaking Changes

- **Library renamed** from `PocketbaseArduino` to `PocketbaseExtended`. Update your `#include` and constructor. A `PocketbaseArduino` typedef alias is retained — existing sketches still compile without changes.
- **Header includes removed from sketch requirement.** `<ESP8266HTTPClient.h>`, `<BearSSLHelpers.h>`, and `<WiFiClientSecure.h>` are now included internally. Remove them from your sketches.
- **Debug logging is now off by default.** Call `pb.setDebug(true)` to re-enable it.

---

### New Features

#### `PBResponse` — Structured Response Type

All `Ex` methods return a `PBResponse` struct instead of a raw `String`:

```cpp
struct PBResponse {
    bool   ok;         // true when HTTP status is 2xx
    int    statusCode; // 0 if the connection itself failed
    String body;       // raw JSON response body
    String error;      // human-readable error description on failure
};
```

#### Authentication

| Method | Description |
|--------|-------------|
| `authWithPassword(identity, password)` | Authenticate with email/username + password. JWT stored automatically. |
| `authRefresh()` | Exchange the current token for a fresh one. Useful on long-running devices. |
| `setAuthToken(token)` | Manually restore a token (e.g. from flash storage). |
| `getAuthToken()` | Read the currently stored token. |
| `clearAuthToken()` | Remove the stored token (effective logout). |

All requests automatically include `Authorization: Bearer <token>` once authenticated.

#### `Ex` Variants for All Record Methods

Every record method now has an `Ex` counterpart returning `PBResponse`:

| Convenience (`String`) | Extended (`PBResponse`) |
|------------------------|-------------------------|
| `getOne()` | `getOneEx()` |
| `getList()` | `getListEx()` |
| `create()` | `createEx()` |
| `update()` | `updateEx()` |
| `deleteRecord()` | `deleteRecordEx()` |

#### New Methods

- **`update()` / `updateEx()`** — `PATCH` partial record update. Was missing in v0.x.
- **`authRefresh()`** — Token refresh without re-entering credentials.
- **`checkHealth()`** — `GET /api/health` connectivity probe. No collection needed.
- **`getFileUrl(recordId, filename, thumb?)`** — Builds a PocketBase file attachment URL without making an HTTP request.

#### New Query Parameters

`getOne`/`getOneEx` and `getList`/`getListEx` now accept:

| Parameter | Description |
|-----------|-------------|
| `expand` | Comma-separated relation fields to auto-expand, e.g. `"author,tags.name"` |
| `fields` | Comma-separated fields to return, e.g. `"id,title,created"` |
| `skipTotal` | Pass `"1"` to omit `totalItems`/`totalPages` for faster list queries |

#### Configuration

| Method | Default | Description |
|--------|---------|-------------|
| `setTimeout(ms)` | `10000` | HTTP request timeout in milliseconds |
| `setInsecureTLS(bool)` | `true` | Skip TLS cert verification. Set `false` in production. |
| `setDebug(bool)` | `false` | Print URLs, status codes, and bodies to `Serial` |

---

### Bug Fixes

- **`getList` query parameter mapping fixed.** In v0.x, `skipTotal` was incorrectly written as the filter value, corrupting query strings when both were used. All parameters now map correctly.

---

### Internal Changes

- Split into dedicated source files: `PocketbaseTransport.cpp`, `PocketbaseAuth.cpp`, `PocketbaseRecords.cpp`
- Single unified HTTP dispatcher handles GET, POST, PATCH, and DELETE
- `baseUrl` trailing slash normalisation handled internally

---

### Migration from v0.x

| v0.x | v1.0.0 |
|------|--------|
| `#include <PocketbaseArduino.h>` | `#include <PocketbaseExtended.h>` |
| `PocketbaseArduino pb(...)` | `PocketbaseExtended pb(...)` — old name still compiles as alias |
| Manual platform header includes | Remove — included internally |
| All methods return `String` | Convenience variants still return `String`; use `Ex` variants for full response |
| No `update()` | `update()` / `updateEx()` added |
| No authentication | Full auth system added |
| Debug always on | Off by default — `pb.setDebug(true)` to enable |
| `getList` skipTotal/filter bug | Fixed |
