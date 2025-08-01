#pragma once

#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "base_window.h"

class InstrumentsWindow : public BaseWindow {
public:
    InstrumentsWindow(const nlohmann::json& instruments_json, const std::string& error, int& selected_row, std::string& selected_pair, int& selected_pair_index, std::vector<std::string>& pairs, std::vector<size_t>& filtered_indices, float table_width);
    void draw() override;
private:
    const nlohmann::json& instruments_json;
    const std::string& error;
    int& selected_row;
    std::string& selected_pair;
    int& selected_pair_index;
    std::vector<std::string>& pairs;
    std::vector<size_t>& filtered_indices;
    float table_width;
};
