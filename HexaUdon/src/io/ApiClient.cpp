#include "io/ApiClient.hpp"

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <utility>

#include <nlohmann/json.hpp>

#include "io/JsonReader.hpp"

using json = nlohmann::json;

namespace {
std::unordered_map<std::string, std::string> readEnvFile(const std::string& path) {
    std::ifstream file(path);
    if (!file) throw std::runtime_error("Cannot open env file: " + path);

    std::unordered_map<std::string, std::string> values;
    std::string line;
    while (std::getline(file, line)) {
        const size_t separator = line.find('=');
        if (separator == std::string::npos || line.empty() || line[0] == '#') continue;
        values[line.substr(0, separator)] = line.substr(separator + 1);
    }
    return values;
}

std::string required(const std::unordered_map<std::string, std::string>& values, const std::string& key) {
    const auto found = values.find(key);
    if (found == values.end() || found->second.empty()) {
        throw std::runtime_error("Missing required .env value: " + key);
    }
    return found->second;
}

std::string shellQuote(const std::string& value) {
#ifdef _WIN32
    std::string quoted = "\"";
    for (char character : value) {
        quoted += character == '"' ? "\\\"" : std::string(1, character);
    }
    return quoted + "\"";
#else
    std::string quoted = "'";
    for (char character : value) quoted += character == '\'' ? "'\\''" : std::string(1, character);
    return quoted + "'";
#endif
}

std::string runCurl(const std::string& command) {
    const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
    const std::filesystem::path responsePath = std::filesystem::temp_directory_path() /
        ("hexaudon-response-" + std::to_string(stamp) + ".json");
    const int exitCode = std::system((command + " > " + shellQuote(responsePath.string()) + " 2>&1").c_str());
    std::ifstream responseFile(responsePath);
    std::stringstream response;
    response << responseFile.rdbuf();
    std::error_code removeError;
    std::filesystem::remove(responsePath, removeError);
    if (exitCode != 0) throw std::runtime_error("API request failed: " + response.str());
    return response.str();
}
}

ApiClient::ApiClient(std::string baseUrlValue, std::string tokenValue, std::string matchIdValue, bool traceValue)
    : baseUrl(std::move(baseUrlValue)), token(std::move(tokenValue)), matchId(std::move(matchIdValue)), trace(traceValue) {
    while (!baseUrl.empty() && baseUrl.back() == '/') baseUrl.pop_back();
}

ApiClient ApiClient::fromEnv(const std::string& envPath) {
    const auto values = readEnvFile(envPath);
    const bool trace = values.count("API_TRACE") && values.at("API_TRACE") == "1";
    ApiClient client(required(values, "API_BASE_URL"), required(values, "API_TOKEN"), values.count("MATCH_ID") ? values.at("MATCH_ID") : "", trace);
    if (client.matchId.empty()) {
        const json response = json::parse(client.request("GET", client.baseUrl + "/api/game/matches"));
        if (!response.contains("matches") || !response["matches"].is_array() || response["matches"].empty()) {
            throw std::runtime_error("The API returned no assigned matches");
        }
        if (!response["matches"][0].contains("id") || !response["matches"][0]["id"].is_string()) {
            throw std::runtime_error("The matches response has no string id");
        }
        client.matchId = response["matches"][0]["id"].get<std::string>();
    }
    return client;
}

std::string ApiClient::endpoint(const std::string& suffix) const {
    return baseUrl + "/api/game/matches/" + matchId + suffix;
}

std::string ApiClient::request(const std::string& method, const std::string& path, const std::string& body) const {
    if (trace) std::cerr << "[API] " << method << " " << path << " request_bytes=" << body.size() << std::endl;
    std::string command = "curl.exe -fsS -X " + method + " -H " + shellQuote("x-api-token: " + token) +
        " -H " + shellQuote("Content-Type: application/json");
    std::filesystem::path bodyPath;
    if (!body.empty()) {
        const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
        bodyPath = std::filesystem::temp_directory_path() / ("hexaudon-request-" + std::to_string(stamp) + ".json");
        std::ofstream bodyFile(bodyPath);
        if (!bodyFile) throw std::runtime_error("Cannot create temporary request body");
        bodyFile << body;
        command += " --data-binary " + shellQuote("@" + bodyPath.string());
    }
    command += " " + shellQuote(path) + " 2>&1";
    std::error_code removeError;
    try {
        const std::string response = runCurl(command);
        if (trace) std::cerr << "[API] response_bytes=" << response.size() << std::endl;
        if (!bodyPath.empty()) std::filesystem::remove(bodyPath, removeError);
        return response;
    } catch (...) {
        if (!bodyPath.empty()) std::filesystem::remove(bodyPath, removeError);
        throw;
    }
}

GameConfig ApiClient::getConfig() const {
    const json response = json::parse(request("GET", endpoint("/config")));
    std::stringstream input(response.dump());
    std::streambuf* original = std::cin.rdbuf(input.rdbuf());
    try {
        GameConfig config = JsonReader::readGameConfig();
        std::cin.rdbuf(original);
        return config;
    } catch (...) {
        std::cin.rdbuf(original);
        throw;
    }
}

GameState ApiClient::getStatus() const {
    const json response = json::parse(request("GET", endpoint("/status")));
    std::stringstream input(response.dump());
    std::streambuf* original = std::cin.rdbuf(input.rdbuf());
    try {
        GameState state = JsonReader::readGameState();
        std::cin.rdbuf(original);
        return state;
    } catch (...) {
        std::cin.rdbuf(original);
        throw;
    }
}

void ApiClient::submitAgentTypes(const std::string& jsonBody) const { request("POST", endpoint("/agents"), jsonBody); }
void ApiClient::submitActions(const std::string& jsonBody) const { request("POST", endpoint("/answer"), jsonBody); }