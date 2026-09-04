#pragma once

#include <string>
#include <vector>
#include <functional>

#ifdef _WIN32
#include <windows.h>
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")
#endif

/**
 * @brief Lightweight HTTPS client using Windows WinHTTP.
 * Supports GET/POST/PUT/DELETE with JSON body and custom headers.
 */
struct HttpResponse {
    int statusCode = 0;
    std::string body;
    bool success = false;
    std::string error;
};

class HttpClient {
public:
    HttpClient(const std::string& baseUrl);
    ~HttpClient();

    void setHeader(const std::string& key, const std::string& value);

    HttpResponse get(const std::string& path);
    HttpResponse post(const std::string& path, const std::string& jsonBody);
    HttpResponse put(const std::string& path, const std::string& jsonBody = "");
    HttpResponse del(const std::string& path);

private:
    HttpResponse request(const std::string& method, const std::string& path, const std::string& body = "");

    std::string baseUrl_;
    std::string host_;
    int port_ = 443;
    bool useHttps_ = true;

    std::vector<std::pair<std::string, std::string>> headers_;

#ifdef _WIN32
    HINTERNET hSession_ = NULL;
    HINTERNET hConnect_ = NULL;
#endif
};
