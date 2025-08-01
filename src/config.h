#pragma once

#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#define CACERT_PATH "C:/msys64/etc/certs/cacert.pem"

constexpr double FETCH_INTERVAL = 10.0; // seconds
constexpr size_t MAX_ORDERBOOK_ROWS = 50;

inline double last_instruments_fetch = 0.0;
inline double last_orderbook_fetch = 0.0;
inline std::string last_inst_id;
inline nlohmann::json orderbook;
inline int last_selected_row = -1;
inline std::vector<size_t> filtered_indices;
inline std::string instruments_error;
inline std::string orderbook_error;
