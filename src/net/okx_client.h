#pragma once

#include <string>

#include <nlohmann/json.hpp>

class OKXClient {
public:
    OKXClient(const std::string& certificate_file_path);
    nlohmann::json fetch_tickers(std::string& error);
    nlohmann::json fetch_orderbook(const std::string& instId, std::string& error);
private:
    std::string cert_file_path;
    nlohmann::json fetch_json_https(const std::string& host, const std::string& port, const std::string& target, std::string& error_out);
};

extern OKXClient* g_okx_client;
