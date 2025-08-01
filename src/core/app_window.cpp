#include <GL/gl.h>
#include <windows.h>

#include "imgui.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_opengl3.h"

#include "app_window.h"
#include "../gui/instruments_window.h"
#include "../gui/orderbook_window.h"
#include "../config.h"

#ifdef _WIN32
LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

AppWindow::AppWindow(MarketDataManager& marketData_, AppState& appState_)
    : marketData(marketData_), appState(appState_) {

}
AppWindow::~AppWindow() { cleanup(); }

void AppWindow::run() {
    init_window();
    init_imgui();
    main_loop();
    cleanup();

}

void AppWindow::init_window() {
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, DefWindowProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, "ImGui Example", NULL };
    RegisterClassEx(&wc);
    hwnd = CreateWindow(wc.lpszClassName, "OKX Futures", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, NULL, NULL, wc.hInstance, NULL);
    hdc = GetDC(hwnd);
    PIXELFORMATDESCRIPTOR pfd = { sizeof(PIXELFORMATDESCRIPTOR), 1, PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, PFD_TYPE_RGBA, 32, 0,0,0,0,0,0,0,0,0,0,0,0,0,24,8,0,PFD_MAIN_PLANE,0,0,0,0 };
    int pf = ChoosePixelFormat(hdc, &pfd);
    SetPixelFormat(hdc, pf, &pfd);
    hrc = wglCreateContext(hdc);
    wglMakeCurrent(hdc, hrc);
    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);
}

void AppWindow::init_imgui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplOpenGL3_Init();
}

void AppWindow::main_loop() {
    
    done = false;
    int loopCount = 0;
    while (!done) {
        MSG msg;
        while (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
            if (ImGui_ImplWin32_WndProcHandler(msg.hwnd, msg.message, msg.wParam, msg.lParam))
                continue;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done) break;
        ++loopCount;

        double now = ImGui::GetTime();
        if (now - marketData.getLastInstrumentsFetch() > FETCH_INTERVAL) {
            marketData.fetchInstruments();
            marketData.updateLastInstrumentsFetch(now);
        }
        if (appState.selected_row >= 0 && appState.selected_row < (int)appState.filtered_indices.size()) {
            const auto& item = marketData.getInstruments()["data"][appState.filtered_indices[appState.selected_row]];
            std::string instId = item["instId"].get<std::string>();
            static std::string last_inst_id;
            static int last_selected_row = -1;
            static double last_orderbook_fetch = 0.0;
            if (last_inst_id != instId || last_selected_row != appState.selected_row || (now - last_orderbook_fetch > FETCH_INTERVAL)) {
                marketData.fetchOrderBook(instId);
                last_inst_id = instId;
                last_selected_row = appState.selected_row;
                last_orderbook_fetch = now;
                marketData.updateLastOrderBookFetch(now);
            }
        }
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        render();
        ImGui::Render();
        ImGuiIO& io = ImGui::GetIO();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SwapBuffers(hdc);
    }

}

void AppWindow::render() {
    ImGui::SetNextWindowSizeConstraints(ImVec2(900, 500), ImVec2(FLT_MAX, FLT_MAX));
    ImGui::SetNextWindowPos(ImVec2(40, 40), ImGuiCond_Once);
    ImGui::Begin("Futures Instruments");
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16, 16));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 8));
    ImVec2 avail = ImGui::GetContentRegionAvail();
    float table_width = avail.x * 0.52f;
    float orderbook_width = avail.x - table_width - 16.0f;
    ImGui::BeginChild("InstrumentsChild", ImVec2(table_width, avail.y), true);
    InstrumentsWindow instrumentsWin(
        marketData.getInstruments(),
        marketData.getInstrumentsError(),
        appState.selected_row,
        appState.selected_pair,
        appState.selected_pair_index,
        appState.pairs,
        appState.filtered_indices,
        table_width);
    instrumentsWin.draw();
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginGroup();
    if (appState.selected_row >= 0 && appState.selected_row < (int)appState.filtered_indices.size()) {
        const auto& item = marketData.getInstruments()["data"][appState.filtered_indices[appState.selected_row]];
        std::string instId = item["instId"].get<std::string>();
        if (last_inst_id != instId || marketData.getLastOrderBookFetch() != appState.selected_row || (ImGui::GetTime() - marketData.getLastOrderBookFetch() > FETCH_INTERVAL)) {
            // fetchOrderBook logic should be called from outside
        }
        ImGui::BeginChild("OrderBookChild", ImVec2(orderbook_width, avail.y), true);
        ImGui::Text("Order Book: %s", instId.c_str());
        OrderBookWindow orderbookWin(marketData.getOrderBook(), marketData.getOrderBookError(), orderbook_width);
        orderbookWin.draw();
        ImGui::EndChild();
    }
    ImGui::EndGroup();
    ImGui::PopStyleVar(2);
    ImGui::End();
}

void AppWindow::cleanup() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    wglMakeCurrent(NULL, NULL);
    if (hrc) wglDeleteContext(hrc);
    if (hdc && hwnd) ReleaseDC(hwnd, hdc);
    if (hwnd) DestroyWindow(hwnd);
    UnregisterClass("ImGui Example", GetModuleHandle(NULL));
}
