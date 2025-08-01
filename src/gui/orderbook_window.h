#pragma once

#include <string>

#include <nlohmann/json.hpp>

#include "base_window.h"

class OrderBookWindow : public BaseWindow {
public:
    OrderBookWindow(const nlohmann::json& orderbook_json, const std::string& error, float orderbook_width);
    void draw() override;
private:
    const nlohmann::json& orderbook_json;
    const std::string& error;
    float orderbook_width;
};
