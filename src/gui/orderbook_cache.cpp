#include "orderbook_cache.h"
#include "../net/okx_client.h"
#include <cstdio>

extern OKXClient* g_okx_client;

std::future<nlohmann::json> OrderbookCache::fetch_orderbook_async(const std::string& instId, std::string& error) {
    return std::async(std::launch::async, [instId, &error]() {
        std::string local_error;
        if (!g_okx_client) {
            return nlohmann::json();
        }
        nlohmann::json ob = g_okx_client->fetch_orderbook(instId, local_error);
        error = local_error;
        return ob;
    });
}

void OrderbookCache::prefetch(const std::string& instId) {
    std::lock_guard<std::mutex> lock(mtx);
    auto it = cache.find(instId);
    if (it == cache.end() || (it->second.data.is_null() && !it->second.loading)) {
        cache[instId].loading = true;
        cache[instId].future = fetch_orderbook_async(instId, cache[instId].error);
    }
}

OrderbookView OrderbookCache::get(const std::string& instId) {
    std::lock_guard<std::mutex> lock(mtx);
    auto it = cache.find(instId);
    if (it == cache.end()) return {};

    if (it->second.data.is_null() && it->second.loading) {
        auto status = it->second.future.wait_for(std::chrono::milliseconds(0));
        if (status == std::future_status::ready) {
            it->second.data = it->second.future.get();
            it->second.loading = false;
        }
    }
    OrderbookView view;
    view.data = it->second.data;
    view.loading = it->second.loading;
    view.error = it->second.error;
    return view;
}
