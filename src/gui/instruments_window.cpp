
#include <set>

#include "orderbook_cache.h"

#include "imgui.h"
#include "instruments_window.h"
#include "../net/okx_client.h"

InstrumentsWindow::InstrumentsWindow(const nlohmann::json& instruments_json_, const std::string& error_, int& selected_row_, std::string& selected_pair_, int& selected_pair_idx_, std::vector<std::string>& pairs_, std::vector<size_t>& filtered_indices_, float table_width_)
    : instruments_json(instruments_json_), error(error_), selected_row(selected_row_), selected_pair(selected_pair_), selected_pair_index(selected_pair_idx_), pairs(pairs_), filtered_indices(filtered_indices_), table_width(table_width_) {}

static OrderbookCache g_orderbook_cache;
static std::string last_selected_pair;
static bool first_filter_load = true;

void InstrumentsWindow::draw() {
    if (!error.empty()) {
        ImGui::TextColored(ImVec4(1,0,0,1), "Instrument fetch error: %s", error.c_str());
    }
    if (instruments_json.contains("data")) {
        std::vector<std::string> unique_inst_families;
        std::set<std::string> seen_inst_families;
        for (const auto& item : instruments_json["data"]) {
            if (item.contains("instFamily")) {
                std::string inst_family_name = item["instFamily"].get<std::string>();
                if (!inst_family_name.empty() && seen_inst_families.insert(inst_family_name).second) {
                    unique_inst_families.push_back(inst_family_name);
                }
            }
        }
        if (pairs != unique_inst_families) {
            pairs = unique_inst_families;
            if (!pairs.empty()) {
                selected_pair_index = 0;
                selected_pair = pairs[0];
            } else {
                selected_pair_index = -1;
                selected_pair.clear();
            }
        }
    }
    if (!pairs.empty()) {
        std::vector<const char*> pair_cstrs;
        for (const auto& p : pairs) pair_cstrs.push_back(p.c_str());
        if (selected_pair_index < 0 || selected_pair_index >= (int)pairs.size()) {
            selected_pair_index = 0;
            selected_pair = pairs[0];
            first_filter_load = true;
        }
        if (ImGui::Combo("Pair", &selected_pair_index, pair_cstrs.data(), (int)pairs.size())) {
            selected_pair = pairs[selected_pair_index];
            selected_row = -1;
            first_filter_load = true;
        }
    }
    ImVec2 avail = ImGui::GetContentRegionAvail();
    ImGui::BeginGroup();
    ImGui::BeginChild("InstrumentsChild", ImVec2(table_width, avail.y), false, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysVerticalScrollbar);
    filtered_indices.clear();
    if (instruments_json.contains("data")) {
        bool need_prefetch = false;
        if (first_filter_load || last_selected_pair != selected_pair) {
            need_prefetch = true;
            last_selected_pair = selected_pair;
            first_filter_load = false;
        }
        for (size_t i = 0; i < instruments_json["data"].size(); ++i) {
            const auto& item = instruments_json["data"][i];
            std::string inst_family_name = item.contains("instFamily") ? item["instFamily"].get<std::string>() : "";
            if (selected_pair.empty() || inst_family_name == selected_pair) {
                filtered_indices.push_back(i);
                std::string instId = item.contains("instId") ? item["instId"].get<std::string>() : "";
                if (!instId.empty() && need_prefetch) {
                    g_orderbook_cache.prefetch(instId);
                }
            }
        }
    }
    if (ImGui::BeginTable("Instruments", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY)) {
        ImGui::TableSetupColumn("Instrument");
        ImGui::TableSetupColumn("Best BID Qty");
        ImGui::TableSetupColumn("Best BID");
        ImGui::TableSetupColumn("Market price");
        ImGui::TableSetupColumn("Best ASK");
        ImGui::TableSetupColumn("Best ASK Qty");
        ImGui::TableHeadersRow();
        int row_count = (int)filtered_indices.size();
        ImGuiListClipper clipper;
        clipper.Begin(row_count);
        while (clipper.Step()) {
            for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row) {
                const auto& item = instruments_json["data"][filtered_indices[row]];
                std::string instId = item.contains("instId") ? item["instId"].get<std::string>() : "";
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::PushID(row);
                bool is_selected = (selected_row == row);
                if (ImGui::Selectable(instId.c_str(), is_selected, ImGuiSelectableFlags_SpanAllColumns)) {
                    selected_row = row;
                }
                ImGui::PopID();

                std::string best_bid_qty = "", best_bid = "", best_ask = "", best_ask_qty = "";
                std::string market_price = "";
                bool loading = false;
                bool has_error = false;
                std::string error_msg;
                OrderbookView orderbook_view = g_orderbook_cache.get(instId);
                if (!orderbook_view.error.empty()) {
                    has_error = true;
                    error_msg = orderbook_view.error;
                }
                if (orderbook_view.data.is_null() && orderbook_view.loading) {
                    loading = true;
                }
                if (!orderbook_view.data.is_null()) {
                    const auto& orderbookData = orderbook_view.data;
                    if (orderbookData.contains("data") && orderbookData["data"].is_array() && !orderbookData["data"].empty()) {
                        const auto& object_data = orderbookData["data"][0];
                        if (object_data.contains("bids") && object_data["bids"].is_array() && !object_data["bids"].empty()) {
                            best_bid = object_data["bids"][0][0].get<std::string>();
                            best_bid_qty = object_data["bids"][0][1].get<std::string>();
                        }
                        if (object_data.contains("asks") && object_data["asks"].is_array() && !object_data["asks"].empty()) {
                            best_ask = object_data["asks"][0][0].get<std::string>();
                            best_ask_qty = object_data["asks"][0][1].get<std::string>();
                        }
                        // Market Price = (Best BID + Best ASK) / 2
                        if (!best_bid.empty() && !best_ask.empty()) {
                            try {
                                double bid = std::stod(best_bid);
                                double ask = std::stod(best_ask);
                                double mp = (bid + ask) / 2.0;
                                char buf[32];
                                snprintf(buf, sizeof(buf), "%.2f", mp);
                                market_price = buf;
                            } catch (...) {
                                market_price = "-";
                            }
                        } else {
                            market_price = "-";
                        }
                    }
                }
                if (has_error) {
                    ImGui::TableSetColumnIndex(1); ImGui::Text("ERR: %s", error_msg.c_str());
                    ImGui::TableSetColumnIndex(2); ImGui::Text("--");
                    ImGui::TableSetColumnIndex(3); ImGui::Text("--");
                    ImGui::TableSetColumnIndex(4); ImGui::Text("--");
                    ImGui::TableSetColumnIndex(5); ImGui::Text("--");
                    continue;
                }
                if (loading) {
                    best_bid = best_ask = market_price = "...";
                }
                ImGui::TableSetColumnIndex(1); ImGui::Text("%s", best_bid_qty.c_str());
                ImGui::TableSetColumnIndex(2); ImGui::Text("%s", best_bid.c_str());
                ImGui::TableSetColumnIndex(3); ImGui::Text("%s", market_price.c_str());
                ImGui::TableSetColumnIndex(4); ImGui::Text("%s", best_ask.c_str());
                ImGui::TableSetColumnIndex(5); ImGui::Text("%s", best_ask_qty.c_str());
            }
        }
        clipper.End();
        ImGui::EndTable();
    }
    ImGui::EndChild();
    ImGui::EndGroup();
}
