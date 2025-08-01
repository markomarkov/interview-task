#include <iostream>
#include <set>
#include <string>

#include <nlohmann/json.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <openssl/ssl.h>

#include <GL/gl.h>
#include <windows.h>

#include "imgui.h"
#include "net/okx_client.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_opengl3.h"
#include "gui/base_window.h"
#include "gui/instruments_window.h"
#include "gui/orderbook_window.h"
#include "data/market_data_manager.h"
#include "core/app_state.h"
#include "core/app_window.h"
#include "config.h"

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
namespace beast = boost::beast;
namespace http = beast::http;
namespace asio = boost::asio;
using tcp = asio::ip::tcp;
using json = nlohmann::json;

OKXClient* g_okx_client = nullptr;

int main() {

    static OKXClient okx(CACERT_PATH);
    g_okx_client = &okx;

    MarketDataManager marketData(CACERT_PATH);
    AppState appState;

    marketData.fetchInstruments();

    AppWindow app(marketData, appState);
    app.run();
    return 0;
}