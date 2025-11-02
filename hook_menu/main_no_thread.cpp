#include <windows.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <fstream>
#include <mutex>
#include <sstream>

constexpr int IDM_BASE = 1000;
constexpr int NUM_ITEMS = 7;
const char* SAVE_FILENAME = "menu_usage.txt";
const int CLICK_THRESHOLD = 10;  // rebuild and save after this many clicks

HWND g_hWnd = nullptr;
int g_clicksSinceSave = 0;

struct MenuDef { UINT id; std::string label; };
std::vector<MenuDef> g_menuDefs = {
    {IDM_BASE + 0, "Open"},
    {IDM_BASE + 1, "Copy"},
    {IDM_BASE + 2, "Paste"},
    {IDM_BASE + 3, "Delete"},
    {IDM_BASE + 4, "Rename"},
    {IDM_BASE + 5, "Properties"},
    {IDM_BASE + 6, "Custom..."}
};

std::unordered_map<UINT, int> g_usageCounts;
std::mutex g_usageMutex;

// ===== File I/O =====

void saveUsageToFile() {
    std::lock_guard<std::mutex> lock(g_usageMutex);
    std::ofstream out(SAVE_FILENAME, std::ios::trunc);
    for (auto& kv : g_usageCounts)
        out << kv.first << ":" << kv.second << "\n";
}

void loadUsageFromFile() {
    std::ifstream in(SAVE_FILENAME);
    if (!in.is_open()) return;
    std::string line;
    while (std::getline(in, line)) {
        size_t pos = line.find(':');
        if (pos == std::string::npos) continue;
        UINT id = static_cast<UINT>(std::stoul(line.substr(0, pos)));
        int count = std::stoi(line.substr(pos + 1));
        g_usageCounts[id] = count;
    }
}

// ===== Menu Building =====

HMENU buildAdaptiveMenu() {
    std::vector<std::pair<UINT, int>> items;
    {
        std::lock_guard<std::mutex> lock(g_usageMutex);
        for (auto& def : g_menuDefs)
            items.push_back({def.id, g_usageCounts[def.id]});
    }

    std::sort(items.begin(), items.end(), [](auto& a, auto& b) {
        if (a.second != b.second) return a.second > b.second;
        return a.first < b.first;
    });

    const size_t promoteCount = 4;
    HMENU mainPopup = CreatePopupMenu();
    for (size_t i = 0; i < items.size() && i < promoteCount; ++i) {
        UINT id = items[i].first;
        auto it = std::find_if(g_menuDefs.begin(), g_menuDefs.end(),
                               [&](auto& m) { return m.id == id; });
        AppendMenuA(mainPopup, MF_STRING, id, it->label.c_str());
    }

    if (items.size() > promoteCount) {
        HMENU more = CreatePopupMenu();
        for (size_t i = promoteCount; i < items.size(); ++i) {
            UINT id = items[i].first;
            auto it = std::find_if(g_menuDefs.begin(), g_menuDefs.end(),
                                   [&](auto& m) { return m.id == id; });
            AppendMenuA(more, MF_STRING, id, it->label.c_str());
        }
        AppendMenuA(mainPopup, MF_POPUP, (UINT_PTR)more, "More...");
    }

    HMENU root = CreateMenu();
    AppendMenuA(root, MF_POPUP, (UINT_PTR)mainPopup, "Actions");
    return root;
}

void rebuildMenu() {
    if (!g_hWnd) return;
    HMENU newMenu = buildAdaptiveMenu();
    SetMenu(g_hWnd, newMenu);
    DrawMenuBar(g_hWnd);
}

// ===== Window Procedure =====

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        {
            HMENU m = buildAdaptiveMenu();
            SetMenu(hwnd, m);
            DrawMenuBar(hwnd);
        }
        return 0;

    case WM_COMMAND:
        {
            UINT id = LOWORD(wParam);
            auto it = std::find_if(g_menuDefs.begin(), g_menuDefs.end(),
                                   [&](auto& m) { return m.id == id; });
            if (it == g_menuDefs.end()) break;

            {
                std::lock_guard<std::mutex> lock(g_usageMutex);
                g_usageCounts[id]++;
            }

            g_clicksSinceSave++;
            std::ostringstream oss;
            oss << "[LOG] Clicked: " << it->label << "\n";
            OutputDebugStringA(oss.str().c_str());

            // Show a quick message for demo
            MessageBoxA(hwnd, it->label.c_str(), "Clicked", MB_OK);

            if (g_clicksSinceSave >= CLICK_THRESHOLD) {
                saveUsageToFile();
                rebuildMenu();
                g_clicksSinceSave = 0;
            }
        }
        return 0;

    case WM_CLOSE:
        saveUsageToFile();
        DestroyWindow(hwnd);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// ===== Entry Point =====

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow) {
    loadUsageFromFile();
    for (auto& def : g_menuDefs)
        if (!g_usageCounts.count(def.id))
            g_usageCounts[def.id] = 0;

    const char* CLASS_NAME = "AdaptiveMenuSimpler";
    WNDCLASSA wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassA(&wc);

    g_hWnd = CreateWindowExA(
        0, CLASS_NAME, "Simpler Adaptive Menu Demo",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 300,
        nullptr, nullptr, hInst, nullptr
    );

    if (!g_hWnd) return 0;

    ShowWindow(g_hWnd, nCmdShow);
    UpdateWindow(g_hWnd);

    MSG msg;
    while (GetMessageA(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    return 0;
}
