#pragma once
#include <string>
#include <future>
#include <map>
#include <mutex>
#include <nlohmann/json.hpp>


struct OrderbookResult {
    nlohmann::json data;
    std::future<nlohmann::json> future;
    bool loading = false;
    std::string error;

    OrderbookResult() = default;
    OrderbookResult(const OrderbookResult&) = delete;
    OrderbookResult& operator=(const OrderbookResult&) = delete;
    OrderbookResult(OrderbookResult&&) = default;
    OrderbookResult& operator=(OrderbookResult&&) = default;
};

struct OrderbookView {
    nlohmann::json data;
    bool loading = false;
    std::string error;
};

class OrderbookCache {
public:
    void prefetch(const std::string& instId);
    OrderbookView get(const std::string& instId);

private:
    std::map<std::string, OrderbookResult> cache;
    std::mutex mtx;
    std::future<nlohmann::json> fetch_orderbook_async(const std::string& instId, std::string& error);
};
