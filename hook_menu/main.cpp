// adaptive_menu_example.cpp
// Build with Visual Studio (x86/x64). Linker uses default user32.lib, kernel32.lib, etc.

#include <windows.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <fstream>
#include <thread>
#include <mutex>
#include <atomic>
#include <sstream>

constexpr int IDM_BASE = 1000; // base for command IDs
constexpr int NUM_ITEMS = 7;
const char* SAVE_FILENAME = "menu_usage.txt";
const UINT WM_REBUILD_MENU = WM_APP + 1;

HWND g_hWnd = nullptr;

// Menu item definitions: stable order/labels and IDs
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

// usage counts in memory
std::unordered_map<UINT, int> g_usageCounts;
std::mutex g_usageMutex;
std::atomic<int> g_totalClicks{0};
std::atomic<bool> g_running{true};

// forward
void rebuildMenuOnUIThread();
void saveUsageToFile();
void loadUsageFromFile();

// Create the initial menu (UI thread)
HMENU CreateAppMenu()
{
    HMENU hMenu = CreateMenu();
    HMENU fileMenu = CreatePopupMenu();

    // We'll put all items at top-level here initially; rebuildMenu will reorder later.
    for (const auto& md : g_menuDefs) {
        AppendMenuA(fileMenu, MF_STRING, md.id, md.label.c_str());
    }

    AppendMenuA(hMenu, MF_POPUP, (UINT_PTR)fileMenu, "Actions");
    return hMenu;
}

// Build a menu based on usage: promote top K, rest go into More...
void rebuildMenuLogic(HMENU hMenuTarget)
{
    // Copy usage counts into vector
    std::vector<std::pair<UINT,int>> vec;
    {
        std::lock_guard<std::mutex> lock(g_usageMutex);
        for (const auto& md : g_menuDefs) {
            int c = 0;
            auto it = g_usageCounts.find(md.id);
            if (it != g_usageCounts.end()) c = it->second;
            vec.push_back({md.id, c});
        }
    }

    // Sort descending by count (most used first)
    std::sort(vec.begin(), vec.end(), [](const auto& a, const auto& b){
        if (a.second != b.second) return a.second > b.second;
        return a.first < b.first; // tie-break by ID for stability
    });

    // Decide threshold: top N directly, others into More
    const size_t promoteCount = 4; // change as desired
    HMENU actions = CreatePopupMenu();

    // Add promoted items
    for (size_t i = 0; i < vec.size(); ++i) {
        UINT id = vec[i].first;
        const std::string* labelPtr = nullptr;
        for (const auto& md : g_menuDefs) if (md.id == id) { labelPtr = &md.label; break; }
        if (!labelPtr) continue;
        if (i < promoteCount) {
            AppendMenuA(actions, MF_STRING, id, labelPtr->c_str());
        }
    }

    // Build More submenu
    HMENU more = CreatePopupMenu();
    for (size_t i = promoteCount; i < vec.size(); ++i) {
        UINT id = vec[i].first;
        const std::string* labelPtr = nullptr;
        for (const auto& md : g_menuDefs) if (md.id == id) { labelPtr = &md.label; break; }
        if (!labelPtr) continue;
        AppendMenuA(more, MF_STRING, id, labelPtr->c_str());
    }

    AppendMenuA(actions, MF_POPUP, (UINT_PTR)more, "More...");

    // Replace the first menu item (Actions) in hMenuTarget with our new popup
    // For simplicity we assume the first item is Actions
    // Remove existing first item and insert new one
    // Find number of items
    int count = GetMenuItemCount(hMenuTarget);
    if (count >= 1) {
        RemoveMenu(hMenuTarget, 0, MF_BYPOSITION);
    }
    AppendMenuA(hMenuTarget, MF_POPUP, (UINT_PTR)actions, "Actions");
}

// Caller must run on UI thread
void rebuildMenuOnUIThread()
{
    if (!g_hWnd) return;
    HMENU hMenu = GetMenu(g_hWnd);
    if (!hMenu) return;

    // To avoid flicker: detach menu, rebuild, then set
    // Simpler: build a new menu and SetMenu
    HMENU newMenu = CreateMenu();
    rebuildMenuLogic(newMenu);
    SetMenu(g_hWnd, newMenu);
    DrawMenuBar(g_hWnd);
    // Destroy old menu - careful: GetMenu returns previous; DestroyMenu is appropriate if we own it.
    // Windows will destroy previous menu when the window is destroyed; avoid double-destroy issues.
}

// Save periodically or at shutdown
void autosaveThreadFunc()
{
    while (g_running.load()) {
        // Sleep and then save
        std::this_thread::sleep_for(std::chrono::seconds(30));
        if (!g_running.load()) break;
        saveUsageToFile();
        // Also request a rebuild on UI thread every interval (optional)
        if (g_hWnd) PostMessageA(g_hWnd, WM_REBUILD_MENU, 0, 0);
    }
}

// Save usage to file
void saveUsageToFile()
{
    std::unordered_map<UINT,int> copy;
    {
        std::lock_guard<std::mutex> lock(g_usageMutex);
        copy = g_usageCounts;
    }
    std::ofstream out(SAVE_FILENAME, std::ios::trunc);
    if (!out.is_open()) return;
    for (const auto& kv : copy) {
        out << kv.first << ":" << kv.second << "\n";
    }
}

// Load usage from file
void loadUsageFromFile()
{
    std::ifstream in(SAVE_FILENAME);
    if (!in.is_open()) return;
    std::string line;
    std::lock_guard<std::mutex> lock(g_usageMutex);
    while (std::getline(in, line)) {
        size_t pos = line.find(':');
        if (pos == std::string::npos) continue;
        UINT id = static_cast<UINT>(std::stoul(line.substr(0, pos)));
        int count = std::stoi(line.substr(pos+1));
        g_usageCounts[id] = count;
    }
}

// Window procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_CREATE:
        {
            HMENU menu = CreateAppMenu();
            SetMenu(hwnd, menu);
            DrawMenuBar(hwnd);
        }
        return 0;

    case WM_COMMAND:
        {
            UINT id = LOWORD(wParam);
            // Handle menu commands
            bool handled = false;
            for (const auto& md : g_menuDefs) {
                if (md.id == id) {
                    // record usage
                    {
                        std::lock_guard<std::mutex> lock(g_usageMutex);
                        g_usageCounts[id]++;
                    }
                    g_totalClicks++;
                    std::ostringstream oss;
                    oss << "Command: " << md.label << " (id=" << id << ") clicked. Count=" << g_usageCounts[id] << "\n";
                    OutputDebugStringA(oss.str().c_str());
                    handled = true;

                    // For demonstration show a message box for some commands
                    if (id == IDM_BASE + 0) { MessageBoxA(hwnd, "Open selected", "Action", MB_OK); }
                    else if (id == IDM_BASE + 6) { MessageBoxA(hwnd, "Custom selected", "Action", MB_OK); }

                    // Rebuild layout in UI thread after some threshold
                    if ((g_totalClicks.load() % 10) == 0) {
                        PostMessageA(hwnd, WM_REBUILD_MENU, 0, 0);
                    }
                    break;
                }
            }
            if (!handled) {
                return DefWindowProc(hwnd, uMsg, wParam, lParam);
            }
        }
        return 0;

    case WM_REBUILD_MENU:
        rebuildMenuOnUIThread();
        return 0;

    case WM_CLOSE:
        // Save now
        g_running = false;
        saveUsageToFile();
        DestroyWindow(hwnd);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow)
{
    // Init load data
    for (const auto& md : g_menuDefs) g_usageCounts[md.id] = 0;
    loadUsageFromFile();

    // Register class
    const char* CLASS_NAME = "AdaptiveMenuDemoClass";
    WNDCLASSA wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

    RegisterClassA(&wc);

    g_hWnd = CreateWindowExA(
        0,
        CLASS_NAME,
        "Adaptive Menu Demo",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 600, 400,
        nullptr,
        nullptr,
        hInst,
        nullptr
    );

    if (!g_hWnd) return 0;

    ShowWindow(g_hWnd, nCmdShow);

    // Start autosave thread
    std::thread autosaveThread(autosaveThreadFunc);
    autosaveThread.detach();

    // Message loop
    MSG msg;
    while (GetMessageA(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    // Final save on exit (just in case)
    saveUsageToFile();
    g_running = false;
    return 0;
}
