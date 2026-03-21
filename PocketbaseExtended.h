/**
 * @file    PocketbaseExtended.h
 * @brief   PocketBase client library for ESP8266 and ESP32.
 *
 * Provides full CRUD, authentication, structured error responses,
 * and optional debug logging for PocketBase (https://pocketbase.io).
 * No heavy dependencies — uses only the platform's built-in HTTP stack.
 *
 * Supported platforms: ESP8266, ESP32
 * PocketBase API compatibility: v0.16+
 *
 * @version 1.0.0
 * @author  Jeo Carlo Lubao
 * @license GPL-3.0
 */

#ifndef PocketbaseExtended_h
#define PocketbaseExtended_h

#include "Arduino.h"

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
// Response type
// ---------------------------------------------------------------------------

/**
 * @brief Structured result returned by all @c Ex record and auth methods.
 *
 * Convenience methods (those without the @c Ex suffix) return only
 * @c body as a @c String; use the @c Ex variants when you need the
 * status code or a machine-readable error.
 */
struct PBResponse {
    bool   ok;          ///< @c true when the HTTP status is 2xx.
    int    statusCode;  ///< HTTP status code; @c 0 when the connection itself failed.
    String body;        ///< Raw JSON response body.
    String error;       ///< Human-readable error; equals @c body on 4xx/5xx responses.
};

// ---------------------------------------------------------------------------
// Query options
// ---------------------------------------------------------------------------

/**
 * @brief  Named query parameters for getList and getOne requests.
 *
 * Use this struct instead of positional @c nullptr placeholders:
 * @code
 * PBQuery q;
 * q.filter = "active=true";
 * q.sort   = "-created";
 * pb.collection("notes").getList(q);
 * @endcode
 */
struct PBQuery {
    const char* page      = nullptr; ///< Page number (1-based). Default: 1.
    const char* perPage   = nullptr; ///< Records per page (max 500). Default: 30.
    const char* sort      = nullptr; ///< Sort expression, e.g. @c "-created,id".
    const char* filter    = nullptr; ///< Filter expression, e.g. @c "active=true".
    const char* skipTotal = nullptr; ///< Set to @c "1" to omit totalItems/totalPages.
    const char* expand    = nullptr; ///< Comma-separated relation fields to expand.
    const char* fields    = nullptr; ///< Comma-separated fields to include in response.
};

// ---------------------------------------------------------------------------
// Client class
// ---------------------------------------------------------------------------

/**
 * @brief PocketBase REST client for ESP8266 / ESP32.
 *
 * @par Basic usage
 * @code
 * PocketbaseExtended pb("https://my-pb-host.com");
 * pb.collection("notes").getOne("RECORD_ID");
 * @endcode
 *
 * @par Auth usage
 * @code
 * pb.collection("users").authWithPassword("user@example.com", "password");
 * // token is stored automatically; all subsequent requests carry it
 * pb.collection("notes").getListEx("1", "10");
 * @endcode
 */
class PocketbaseExtended {
public:

    // -----------------------------------------------------------------------
    // Constructor
    // -----------------------------------------------------------------------

    /**
     * @brief  Construct a new PocketbaseExtended client.
     *
     * Trailing slashes in @p baseUrl are normalised automatically.
     *
     * @param[in] baseUrl  Root URL of the PocketBase instance,
     *                     e.g. @c "https://my-pb-host.com".
     */
    explicit PocketbaseExtended(const char* baseUrl);

    // -----------------------------------------------------------------------
    // Collection selector
    // -----------------------------------------------------------------------

    /**
     * @brief  Set the active collection for subsequent requests.
     *
     * Returns a reference to @c *this so calls can be chained:
     * @code
     * pb.collection("notes").getOne("id");
     * @endcode
     *
     * @param[in] name  PocketBase collection name.
     * @return Reference to this instance (chainable).
     */
    PocketbaseExtended& collection(const char* name);

    // -----------------------------------------------------------------------
    // Record methods — extended (return PBResponse)
    // -----------------------------------------------------------------------

    /**
     * @brief  Fetch a single record by ID.
     *
     * PocketBase endpoint: @c GET /api/collections/{collection}/records/{id}
     *
     * @param[in] recordId  ID of the record to fetch. Must not be empty.
     * @param[in] expand    Comma-separated relation fields to auto-expand,
     *                      e.g. @c "author,tags.name". Pass @c nullptr to skip.
     * @param[in] fields    Comma-separated fields to include in the response,
     *                      e.g. @c "id,title,created". Pass @c nullptr for all fields.
     * @return PBResponse
     */
    PBResponse getOneEx(const char* recordId,
                        const char* expand = nullptr,
                        const char* fields = nullptr);

    /** @brief  PBQuery overload — avoids nullptr placeholders.
     *
     * @code
     * PBQuery q;
     * q.expand = "author";
     * PBResponse r = pb.collection("posts").getOneEx("RECORD_ID", q);
     * @endcode
     */
    PBResponse getOneEx(const char* recordId, const PBQuery& q);

    /**
     * @brief  Fetch a paginated list of records.
     *
     * PocketBase endpoint: @c GET /api/collections/{collection}/records
     *
     * All parameters are optional. Pass @c nullptr or omit to use the
     * server default for that parameter.
     *
     * @param[in] page       Page number (1-based). Default: @c 1.
     * @param[in] perPage    Records per page (max 500). Default: @c 30.
     * @param[in] sort       Sort expression, e.g. @c "-created,id"
     *                       (@c - prefix = DESC, no prefix = ASC).
     * @param[in] filter     PocketBase filter expression,
     *                       e.g. @c "active = true && views > 100".
     * @param[in] skipTotal  Set to @c "1" to omit @c totalItems / @c totalPages
     *                       from the response for faster queries.
     * @param[in] expand     Comma-separated relation fields to auto-expand.
     * @param[in] fields     Comma-separated fields to include in each record.
     * @return PBResponse
     */
    PBResponse getListEx(const char* page      = nullptr,
                         const char* perPage   = nullptr,
                         const char* sort      = nullptr,
                         const char* filter    = nullptr,
                         const char* skipTotal = nullptr,
                         const char* expand    = nullptr,
                         const char* fields    = nullptr);

    /** @brief  PBQuery overload — avoids nullptr placeholders.
     *
     * @code
     * PBQuery q;
     * q.filter = "active=true";
     * q.sort   = "-created";
     * PBResponse r = pb.collection("notes").getListEx(q);
     * @endcode
     */
    PBResponse getListEx(const PBQuery& q);

    /**
     * @brief  Create a new record.
     *
     * PocketBase endpoint: @c POST /api/collections/{collection}/records
     *
     * @param[in] requestBody  JSON string of fields to set,
     *                         e.g. @c "{\"title\":\"Hello\",\"active\":true}".
     * @return PBResponse
     */
    PBResponse createEx(const String& requestBody);

    /**
     * @brief  Update an existing record (partial update).
     *
     * PocketBase endpoint: @c PATCH /api/collections/{collection}/records/{id}
     *
     * Only the fields present in @p requestBody are changed; omitted fields
     * retain their current values.
     *
     * @param[in] recordId     ID of the record to update. Must not be empty.
     * @param[in] requestBody  JSON string of fields to update.
     * @return PBResponse
     */
    PBResponse updateEx(const char* recordId, const String& requestBody);

    /**
     * @brief  Delete a record by ID.
     *
     * PocketBase endpoint: @c DELETE /api/collections/{collection}/records/{id}
     *
     * A successful delete returns HTTP 204 with an empty body;
     * @c PBResponse::ok will be @c true and @c body will be empty.
     *
     * @param[in] recordId  ID of the record to delete. Must not be empty.
     * @return PBResponse
     */
    PBResponse deleteRecordEx(const char* recordId);

    // -----------------------------------------------------------------------
    // Record methods — convenience (return body String)
    // -----------------------------------------------------------------------

    /**
     * @brief  Convenience wrapper for getOneEx(). Returns the response body.
     * @see    getOneEx()
     */
    String getOne(const char* recordId,
                  const char* expand = nullptr,
                  const char* fields = nullptr);

    /** @brief  PBQuery overload — avoids nullptr placeholders. @see getOneEx(const char*, const PBQuery&) */
    String getOne(const char* recordId, const PBQuery& q);

    /**
     * @brief  Convenience wrapper for getListEx(). Returns the response body.
     * @see    getListEx()
     */
    String getList(const char* page      = nullptr,
                   const char* perPage   = nullptr,
                   const char* sort      = nullptr,
                   const char* filter    = nullptr,
                   const char* skipTotal = nullptr,
                   const char* expand    = nullptr,
                   const char* fields    = nullptr);

    /** @brief  PBQuery overload — avoids nullptr placeholders. @see getListEx(const PBQuery&) */
    String getList(const PBQuery& q);

    /**
     * @brief  Convenience wrapper for createEx(). Returns the response body.
     * @see    createEx()
     */
    String create(const String& requestBody);

    /**
     * @brief  Convenience wrapper for updateEx(). Returns the response body.
     * @see    updateEx()
     */
    String update(const char* recordId, const String& requestBody);

    /**
     * @brief  Convenience wrapper for deleteRecordEx(). Returns the response body.
     * @see    deleteRecordEx()
     */
    String deleteRecord(const char* recordId);

    // -----------------------------------------------------------------------
    // Authentication
    // -----------------------------------------------------------------------

    /**
     * @brief  Authenticate with an identity (email/username) and password.
     *
     * PocketBase endpoint:
     * @c POST /api/collections/{collection}/auth-with-password
     *
     * On success, the returned JWT token is automatically extracted from
     * the response body and stored internally. All subsequent requests will
     * include @c Authorization: Bearer <token> until clearAuthToken() is called.
     *
     * @note   Call @c collection() with the auth collection name (e.g. @c "users")
     *         before calling this method.
     *
     * @param[in] identity  Email address or username of the user.
     * @param[in] password  Account password.
     * @return PBResponse   Full auth response body (contains @c token and @c record).
     */
    PBResponse authWithPassword(const char* identity, const char* password);

    /**
     * @brief  Refresh the currently stored auth token.
     *
     * PocketBase endpoint:
     * @c POST /api/collections/{collection}/auth-refresh
     *
     * Exchanges the existing token for a new one with a renewed expiry.
     * The updated token is stored automatically on success.
     * Useful for long-running devices that need to stay authenticated
     * without re-entering credentials.
     *
     * @note   Requires an existing token (via authWithPassword() or setAuthToken())
     *         and the correct collection to be set.
     *
     * @return PBResponse   Full auth response body (contains new @c token and @c record).
     */
    PBResponse authRefresh();

    /**
     * @brief  Manually set the auth token (e.g. a token restored from storage).
     * @param[in] token  JWT token string.
     */
    void setAuthToken(const String& token);

    /**
     * @brief  Retrieve the currently stored auth token.
     * @return JWT token string, or an empty String if not authenticated.
     */
    String getAuthToken() const;

    /**
     * @brief  Clear the stored auth token (effective logout).
     *
     * Subsequent requests will no longer include an Authorization header.
     */
    void clearAuthToken();

    // -----------------------------------------------------------------------
    // Configuration
    // -----------------------------------------------------------------------

    /**
     * @brief  Set the HTTP request timeout.
     * @param[in] ms  Timeout in milliseconds. Default: @c 10000.
     */
    void setTimeout(uint32_t ms);

    /**
     * @brief  Enable or disable TLS certificate verification.
     *
     * When @c true (default), the TLS certificate of the server is not
     * verified — use this for self-signed or development certificates.
     * Set to @c false in production environments with valid certificates.
     *
     * @param[in] enabled  @c true to skip verification (insecure), @c false to verify.
     */
    void setInsecureTLS(bool enabled);

    /**
     * @brief  Enable or disable verbose debug logging to Serial.
     *
     * When enabled, request URLs, HTTP status codes, and response bodies
     * are printed via @c Serial.println(). Disabled by default.
     *
     * @param[in] enabled  @c true to enable logging, @c false to disable.
     */
    void setDebug(bool enabled);

    // -----------------------------------------------------------------------
    // Health
    // -----------------------------------------------------------------------

    /**
     * @brief  Check whether the PocketBase server is healthy.
     *
     * PocketBase endpoint: @c GET /api/health
     *
     * Does not require a collection to be set.
     * Useful as a connectivity probe at device startup.
     *
     * @return PBResponse   Body is @c {"code":200,"message":"API is healthy."} on success.
     */
    PBResponse checkHealth();

    // -----------------------------------------------------------------------
    // Files
    // -----------------------------------------------------------------------

    /**
     * @brief  Build the URL for a record file attachment.
     *
     * PocketBase file URL: @c GET /api/files/{collection}/{recordId}/{filename}
     *
     * This is a URL-construction helper — it does not make an HTTP request.
     * Use the returned URL with an HTTP GET (or pass it to a browser / display).
     *
     * @param[in] recordId  ID of the record that owns the file.
     * @param[in] filename  Exact filename as stored in the record field.
     * @param[in] thumb     Optional thumbnail size string, e.g. @c "100x100" or @c "0x50".
     *                      Pass @c nullptr to get the original file.
     * @return String       Full URL to the file.
     *
     * @see https://pocketbase.io/docs/files-handling/
     */
    String getFileUrl(const char* recordId,
                      const char* filename,
                      const char* thumb = nullptr);

private:
    String   _baseUrl;     ///< Normalised base URL ending with @c "/api/".
    String   _collection;  ///< Active collection name set by collection().
    String   _authToken;   ///< Stored JWT; empty when not authenticated.
    uint32_t _timeout;     ///< Request timeout in milliseconds.
    bool     _insecureTLS; ///< Skip TLS certificate verification when @c true.
    bool     _debug;       ///< Print debug output to Serial when @c true.

    /** @brief Build the records endpoint URL, optionally appending a record ID. */
    String _buildRecordsUrl(const char* recordId = nullptr);

    /** @brief Append @c key=value to a URL query string; no-op if @p value is null or empty. */
    String _appendParam(const String& url, const char* key, const char* value);

    /** @brief Unified HTTP dispatcher for GET, POST, PATCH, and DELETE. */
    PBResponse _request(const char* method,
                        const String& url,
                        const String& body = "");

    /** @brief Print @p msg to Serial when debug mode is enabled. */
    void _debugPrint(const String& msg);
};

// ---------------------------------------------------------------------------
// Backward-compatibility alias
// ---------------------------------------------------------------------------

/** @brief Alias for PocketbaseExtended. Retained for v0.x sketch compatibility. */
using PocketbaseArduino = PocketbaseExtended;

#endif // PocketbaseExtended_h
