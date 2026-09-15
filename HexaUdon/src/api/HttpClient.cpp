#include "api/HttpClient.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>

// Helper: Convert std::string (UTF-8) to std::wstring
static std::wstring toWide(const std::string& str) {
    if (str.empty()) return L"";
    int sz = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), nullptr, 0);
    std::wstring wstr(sz, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &wstr[0], sz);
    return wstr;
}

// Helper: Parse URL into host, port, path, https flag
static void parseUrl(const std::string& url, std::string& host, int& port, bool& https) {
    std::string remainder = url;
    if (remainder.substr(0, 8) == "https://") {
        https = true;
        port = 443;
        remainder = remainder.substr(8);
    } else if (remainder.substr(0, 7) == "http://") {
        https = false;
        port = 80;
        remainder = remainder.substr(7);
    }

    // Check for port
    auto colonPos = remainder.find(':');
    auto slashPos = remainder.find('/');
    if (colonPos != std::string::npos && (slashPos == std::string::npos || colonPos < slashPos)) {
        host = remainder.substr(0, colonPos);
        std::string portStr;
        if (slashPos != std::string::npos) {
            portStr = remainder.substr(colonPos + 1, slashPos - colonPos - 1);
        } else {
            portStr = remainder.substr(colonPos + 1);
        }
        port = std::stoi(portStr);
    } else {
        host = (slashPos != std::string::npos) ? remainder.substr(0, slashPos) : remainder;
    }
}

HttpClient::HttpClient(const std::string& baseUrl) : baseUrl_(baseUrl) {
    parseUrl(baseUrl, host_, port_, useHttps_);

    std::wstring wAgent = L"HexaUdon/2.0";
    hSession_ = InternetOpenW(wAgent.c_str(),
                              INTERNET_OPEN_TYPE_PRECONFIG,
                              NULL,
                              NULL, 0);

    if (hSession_) {
        std::wstring wHost = toWide(host_);
        hConnect_ = InternetConnectW(hSession_, wHost.c_str(),
                                     static_cast<INTERNET_PORT>(port_),
                                     NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    }

    // Default headers
    headers_.push_back({"Content-Type", "application/json"});
    headers_.push_back({"Accept", "application/json"});
}

HttpClient::~HttpClient() {
    if (hConnect_) InternetCloseHandle(hConnect_);
    if (hSession_) InternetCloseHandle(hSession_);
}

void HttpClient::setHeader(const std::string& key, const std::string& value) {
    // Update existing or add new
    for (auto& h : headers_) {
        if (h.first == key) {
            h.second = value;
            return;
        }
    }
    headers_.push_back({key, value});
}

HttpResponse HttpClient::get(const std::string& path) {
    return request("GET", path);
}

HttpResponse HttpClient::post(const std::string& path, const std::string& jsonBody) {
    return request("POST", path, jsonBody);
}

HttpResponse HttpClient::put(const std::string& path, const std::string& jsonBody) {
    return request("PUT", path, jsonBody);
}

HttpResponse HttpClient::del(const std::string& path) {
    return request("DELETE", path);
}

HttpResponse HttpClient::request(const std::string& method, const std::string& path, const std::string& body) {
    HttpResponse response;

    if (!hConnect_) {
        response.error = "WinHTTP connection not initialized";
        return response;
    }

    std::wstring wPath = toWide(path);
    std::wstring wMethod = toWide(method);

    DWORD flags = useHttps_ ? INTERNET_FLAG_SECURE : 0;
    HINTERNET hRequest = HttpOpenRequestW(
        hConnect_, wMethod.c_str(), wPath.c_str(),
        NULL, NULL, NULL, flags, 0);

    if (!hRequest) {
        response.error = "Failed to open request, error: " +
                         std::to_string(GetLastError());
        return response;
    }

    // Disable certificate validation for development/testing
    if (useHttps_) {
        DWORD dwFlags = SECURITY_FLAG_IGNORE_UNKNOWN_CA |
                SECURITY_FLAG_IGNORE_WRONG_USAGE |
                        SECURITY_FLAG_IGNORE_CERT_CN_INVALID |
                        SECURITY_FLAG_IGNORE_CERT_DATE_INVALID;
        InternetSetOptionW(hRequest, INTERNET_OPTION_SECURITY_FLAGS,
                   &dwFlags, sizeof(dwFlags));
    }

    // Set headers
    for (const auto& h : headers_) {
        std::wstring header = toWide(h.first + ": " + h.second);
        HttpAddRequestHeadersW(hRequest, header.c_str(), (DWORD)-1L,
                               HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE);
    }

    // Send request
    BOOL bResult;
    if (!body.empty()) {
        bResult = HttpSendRequestW(hRequest, NULL, 0,
                                   (LPVOID)body.c_str(), (DWORD)body.size());
    } else {
        bResult = HttpSendRequestW(hRequest, NULL, 0, NULL, 0);
    }

    if (!bResult) {
        DWORD err = GetLastError();
        response.error = "HttpSendRequest failed, error: " + std::to_string(err);
        InternetCloseHandle(hRequest);
        return response;
    }

    if (!bResult) {
        response.error = "HttpSendRequest failed, error: " +
                         std::to_string(GetLastError());
        InternetCloseHandle(hRequest);
        return response;
    }

    // Get status code
    DWORD statusCode = 0;
    DWORD dwSize = sizeof(statusCode);
    HttpQueryInfoW(hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER,
                   &statusCode, &dwSize, NULL);
    response.statusCode = static_cast<int>(statusCode);

    // Read response body
    std::string responseBody;
    DWORD bytesAvailable = 0;
    do {
        bytesAvailable = 0;
        if (!InternetQueryDataAvailable(hRequest, &bytesAvailable, 0, 0)) {
            response.error = "InternetQueryDataAvailable failed, error: " +
                             std::to_string(GetLastError());
            InternetCloseHandle(hRequest);
            return response;
        }
        if (bytesAvailable > 0) {
            std::vector<char> buffer(bytesAvailable + 1, 0);
            DWORD bytesRead = 0;
            if (!InternetReadFile(hRequest, buffer.data(), bytesAvailable, &bytesRead)) {
                response.error = "InternetReadFile failed, error: " +
                                 std::to_string(GetLastError());
                InternetCloseHandle(hRequest);
                return response;
            }
            responseBody.append(buffer.data(), bytesRead);
        }
    } while (bytesAvailable > 0);

    response.body = responseBody;
    response.success = (statusCode >= 200 && statusCode < 300);

    InternetCloseHandle(hRequest);
    return response;
}
