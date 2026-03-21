// PBRecords.cpp
// Record CRUD methods, URL helpers, and file URL builder.

#include "PocketbaseExtended.h"

// ---------------------------------------------------------------------------
// Private URL helpers
// ---------------------------------------------------------------------------

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
// File URL builder (no HTTP request)
// ---------------------------------------------------------------------------

String PocketbaseExtended::getFileUrl(const char* recordId,
                                       const char* filename,
                                       const char* thumb) {
    String url = _baseUrl + "files/" + _collection + "/" + recordId + "/" + filename;
    url = _appendParam(url, "thumb", thumb);
    return url;
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
// PBQuery overloads (delegate to positional methods above)
// ---------------------------------------------------------------------------

PBResponse PocketbaseExtended::getListEx(const PBQuery& q) {
    return getListEx(q.page, q.perPage, q.sort, q.filter, q.skipTotal, q.expand, q.fields);
}

PBResponse PocketbaseExtended::getOneEx(const char* recordId, const PBQuery& q) {
    return getOneEx(recordId, q.expand, q.fields);
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

String PocketbaseExtended::getList(const PBQuery& q) {
    return getListEx(q).body;
}

String PocketbaseExtended::getOne(const char* recordId, const PBQuery& q) {
    return getOneEx(recordId, q).body;
}
