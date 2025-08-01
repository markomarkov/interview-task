#pragma once

#include <string>

#include <nlohmann/json.hpp>

#include "../net/okx_client.h"

class MarketDataManager {
public:
    MarketDataManager(const std::string& ssl_cert_path);
    void fetchInstruments();
    void fetchOrderBook(const std::string& instId);

    const nlohmann::json& getInstruments() const;
    const nlohmann::json& getOrderBook() const;
    const std::string& getInstrumentsError() const;
    const std::string& getOrderBookError() const;
    double getLastInstrumentsFetch() const;
    double getLastOrderBookFetch() const;
    void updateLastInstrumentsFetch(double t);
    void updateLastOrderBookFetch(double t);
private:
    OKXClient okx;
    nlohmann::json instruments_json;
    nlohmann::json orderbook_json;
    std::string instruments_error;
    std::string orderbook_error;
    double last_instruments_fetch = 0.0;
    double last_orderbook_fetch = 0.0;
};
