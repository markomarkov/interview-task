#pragma once

#include <string>
#include <vector>
#include <windows.h>

#include <nlohmann/json.hpp>

#include "../data/market_data_manager.h"
#include "app_state.h"

class AppWindow {
public:
    AppWindow(MarketDataManager& marketData, AppState& appState);
    ~AppWindow();
    void run();
private:
    void init_window();
    void init_imgui();
    void cleanup();
    void main_loop();
    void render();

    HWND hwnd = nullptr;
    HDC hdc = nullptr;
    HGLRC hrc = nullptr;
    bool done = false;

    MarketDataManager& marketData;
    AppState& appState;
};
