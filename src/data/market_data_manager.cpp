#include "market_data_manager.h"

MarketDataManager::MarketDataManager(const std::string& ssl_cert_path)
    : okx(ssl_cert_path) {}

void MarketDataManager::fetchInstruments() {
    instruments_json = okx.fetch_tickers(instruments_error);
}

void MarketDataManager::fetchOrderBook(const std::string& instId) {
    orderbook_json = okx.fetch_orderbook(instId, orderbook_error);
}

const nlohmann::json& MarketDataManager::getInstruments() const { return instruments_json; }
const nlohmann::json& MarketDataManager::getOrderBook() const { return orderbook_json; }
const std::string& MarketDataManager::getInstrumentsError() const { return instruments_error; }
const std::string& MarketDataManager::getOrderBookError() const { return orderbook_error; }
double MarketDataManager::getLastInstrumentsFetch() const { return last_instruments_fetch; }
double MarketDataManager::getLastOrderBookFetch() const { return last_orderbook_fetch; }
void MarketDataManager::updateLastInstrumentsFetch(double t) { last_instruments_fetch = t; }
void MarketDataManager::updateLastOrderBookFetch(double t) { last_orderbook_fetch = t; }
