#include <vector>
#include <algorithm>

#include "imgui.h"
#include "orderbook_window.h"

OrderBookWindow::OrderBookWindow(const nlohmann::json& orderbook_json_, const std::string& error_, float orderbook_width_)
    : orderbook_json(orderbook_json_), error(error_), orderbook_width(orderbook_width_) {}

void OrderBookWindow::draw() {
    if (!error.empty()) {
        ImGui::TextColored(ImVec4(1,0,0,1), "Order book fetch error: %s", error.c_str());
    }
    if (orderbook_json.contains("data") && !orderbook_json["data"].empty()) {
        const auto& ob = orderbook_json["data"][0];
        std::vector<std::vector<std::string>> bids, asks;
        if (ob.contains("bids")) {
            for (size_t i = 0; i < ob["bids"].size() && i < 50; ++i) {
                const auto& lvl = ob["bids"][i];
                std::vector<std::string> row;
                for (size_t k = 0; k < 2 && k < lvl.size(); ++k) row.push_back(lvl[k].get<std::string>());
                bids.push_back(row);
            }
        }
        if (ob.contains("asks")) {
            for (size_t i = 0; i < ob["asks"].size() && i < 50; ++i) {
                const auto& lvl = ob["asks"][i];
                std::vector<std::string> row;
                for (size_t k = 0; k < 2 && k < lvl.size(); ++k) row.push_back(lvl[k].get<std::string>());
                asks.push_back(row);
            }
        }
        std::reverse(bids.begin(), bids.end());
        if (ImGui::BeginTable("OrderBook", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY)) {
            ImGui::TableSetupColumn("BID Quantity");
            ImGui::TableSetupColumn("BID Price");
            ImGui::TableSetupColumn("ASK Price");
            ImGui::TableSetupColumn("ASK Quantity");
            ImGui::TableHeadersRow();
            size_t max_rows = std::max(bids.size(), asks.size());
            for (size_t i = 0; i < max_rows; ++i) {
                ImGui::TableNextRow();
                if (i < bids.size()) {
                    ImGui::TableSetColumnIndex(0); ImGui::Text("%s", bids[i].size() > 1 ? bids[i][1].c_str() : "");
                    ImGui::TableSetColumnIndex(1); ImGui::Text("%s", bids[i].size() > 0 ? bids[i][0].c_str() : "");
                } else {
                    ImGui::TableSetColumnIndex(0); ImGui::Text("");
                    ImGui::TableSetColumnIndex(1); ImGui::Text("");
                }
                if (i < asks.size()) {
                    ImGui::TableSetColumnIndex(2); ImGui::Text("%s", asks[i].size() > 0 ? asks[i][0].c_str() : "");
                    ImGui::TableSetColumnIndex(3); ImGui::Text("%s", asks[i].size() > 1 ? asks[i][1].c_str() : "");
                } else {
                    ImGui::TableSetColumnIndex(2); ImGui::Text("");
                    ImGui::TableSetColumnIndex(3); ImGui::Text("");
                }
            }
            ImGui::EndTable();
        }
    } else {
        ImGui::Text("No order book data.");
    }
}
