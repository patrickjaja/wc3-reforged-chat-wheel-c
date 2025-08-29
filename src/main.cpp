// src/main.cpp
#define UNICODE
#define _UNICODE
#define WIN32_LEAN_AND_MEAN

#include <windows.h>

// Fix for MinGW GDI+ headers
#ifndef PROPID
typedef ULONG PROPID;
#endif
#include <gdiplus.h>
#include <vector>
#include <string>
#include <cmath>
#include <memory>

// MinGW benötigt manchmal diese Defines
#ifndef WS_EX_LAYERED
#define WS_EX_LAYERED 0x00080000
#endif

#ifndef WS_EX_TRANSPARENT
#define WS_EX_TRANSPARENT 0x00000020
#endif

// Link Libraries (MinGW style)
#ifdef __MINGW32__
#pragma comment(lib, "gdiplus")
#pragma comment(lib, "gdi32")
#pragma comment(lib, "user32")
#endif

using namespace Gdiplus;

class ChatWheel {
private:
    HWND hwnd;
    HWND wc3Window;
    std::vector<std::wstring> messages;
    int selectedSegment = -1;
    bool visible = false;
    POINT wheelCenter;
    int wheelRadius = 150;
    ULONG_PTR gdiplusToken;
    static ChatWheel* instance;
    
    // GDI+ Objekte
    std::unique_ptr<Bitmap> backBuffer;
    std::unique_ptr<Graphics> backGraphics;
    
public:
    ChatWheel() {
        instance = this;
        
        messages = {
            L"Well played!",
            L"Get back!",
            L"Push now!",
            L"Need help!",
            L"On my way",
            L"Enemy missing!",
            L"Good game",
            L"Thanks!"
        };
        
        InitGDIPlus();
        CreateOverlayWindow();
    }
    
    ~ChatWheel() {
        if (gdiplusToken) {
            GdiplusShutdown(gdiplusToken);
        }
    }
    
    void InitGDIPlus() {
        GdiplusStartupInput gdiplusStartupInput;
        GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    }
    
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        ChatWheel* wheel = nullptr;
        
        if (uMsg == WM_CREATE) {
            CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
            wheel = reinterpret_cast<ChatWheel*>(pCreate->lpCreateParams);
            SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(wheel));
        } else {
            wheel = reinterpret_cast<ChatWheel*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        }
        
        switch (uMsg) {
            case WM_PAINT: {
                if (wheel && wheel->visible) {
                    PAINTSTRUCT ps;
                    HDC hdc = BeginPaint(hwnd, &ps);
                    wheel->Draw(hdc);
                    EndPaint(hwnd, &ps);
                }
                return 0;
            }
            
            case WM_TIMER: {
                if (wheel && wParam == 1) {
                    wheel->UpdateSelection();
                }
                return 0;
            }
            
            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;
                
            default:
                return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
    }
    
    void CreateOverlayWindow() {
        HINSTANCE hInstance = GetModuleHandle(NULL);
        
        // Window Class
        WNDCLASSEXW wc = {};
        wc.cbSize = sizeof(WNDCLASSEXW);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = hInstance;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
        wc.lpszClassName = L"ChatWheelOverlay";
        
        if (!RegisterClassExW(&wc)) {
            MessageBoxW(NULL, L"Failed to register window class", L"Error", MB_OK);
            return;
        }
        
        // Create Window
        hwnd = CreateWindowExW(
            WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
            L"ChatWheelOverlay",
            L"Chat Wheel",
            WS_POPUP,
            CW_USEDEFAULT, CW_USEDEFAULT, 400, 400,
            NULL, NULL, hInstance, this
        );
        
        if (!hwnd) {
            MessageBoxW(NULL, L"Failed to create window", L"Error", MB_OK);
            return;
        }
        
        // Set transparency - make black color fully transparent
        SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);
    }
    
    void Show() {
        // Find WC3
        wc3Window = FindWindowW(NULL, L"Warcraft III");
        if (!wc3Window) {
            wc3Window = FindWindowW(NULL, L"Warcraft III: Reforged");
        }
        
        if (!wc3Window) {
            MessageBoxW(NULL, L"Warcraft III not found!", L"Error", MB_OK);
            return;
        }
        
        // Get cursor position
        GetCursorPos(&wheelCenter);
        
        // Position window
        SetWindowPos(hwnd, HWND_TOPMOST,
                    wheelCenter.x - 200, wheelCenter.y - 200,
                    400, 400,
                    SWP_SHOWWINDOW | SWP_NOACTIVATE);
        
        visible = true;
        InvalidateRect(hwnd, NULL, TRUE);
        
        // Start timer for mouse tracking
        SetTimer(hwnd, 1, 10, NULL);
    }
    
    void Hide() {
        visible = false;
        KillTimer(hwnd, 1);
        ShowWindow(hwnd, SW_HIDE);
        
        if (selectedSegment >= 0 && selectedSegment < static_cast<int>(messages.size())) {
            SendMessageToWC3(messages[selectedSegment]);
            selectedSegment = -1;
        }
    }
    
    void UpdateSelection() {
        if (!visible) return;
        
        POINT cursor;
        GetCursorPos(&cursor);
        
        int dx = cursor.x - wheelCenter.x;
        int dy = cursor.y - wheelCenter.y;
        float distance = sqrtf(static_cast<float>(dx * dx + dy * dy));
        
        if (distance > 30 && distance < wheelRadius) {
            float angle = atan2f(static_cast<float>(dy), static_cast<float>(dx)) * 180.0f / 3.14159f + 90.0f;
            if (angle < 0) angle += 360.0f;
            
            int newSegment = static_cast<int>(angle / (360.0f / messages.size()));
            if (newSegment >= static_cast<int>(messages.size())) {
                newSegment = messages.size() - 1;
            }
            
            if (newSegment != selectedSegment) {
                selectedSegment = newSegment;
                InvalidateRect(hwnd, NULL, FALSE);
            }
        } else if (selectedSegment != -1) {
            selectedSegment = -1;
            InvalidateRect(hwnd, NULL, FALSE);
        }
    }
    
    void Draw(HDC hdc) {
        if (!visible) return;
        
        // Create back buffer
        HDC memDC = CreateCompatibleDC(hdc);
        HBITMAP memBitmap = CreateCompatibleBitmap(hdc, 400, 400);
        HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);
        
        // Clear background with transparent color (black will be made transparent by LWA_COLORKEY)
        RECT rect = {0, 0, 400, 400};
        FillRect(memDC, &rect, (HBRUSH)GetStockObject(BLACK_BRUSH));
        
        // GDI+ drawing
        {
            Graphics graphics(memDC);
            graphics.SetSmoothingMode(SmoothingModeAntiAlias);
            
            int numSegments = static_cast<int>(messages.size());
            float anglePerSegment = 360.0f / numSegments;
            
            for (int i = 0; i < numSegments; i++) {
                float startAngle = i * anglePerSegment - 90;
                
                // Colors
                Color fillColor = (i == selectedSegment) ?
                    Color(200, 74, 158, 255) : Color(180, 42, 42, 42);
                Color borderColor(255, 100, 100, 100);
                
                // Draw segment
                SolidBrush fillBrush(fillColor);
                Pen borderPen(borderColor, 2.0f);
                
                graphics.FillPie(&fillBrush, 50, 50, 300, 300, startAngle, anglePerSegment);
                graphics.DrawPie(&borderPen, 50, 50, 300, 300, startAngle, anglePerSegment);
                
                // Draw text
                float midAngle = (startAngle + anglePerSegment / 2) * 3.14159f / 180.0f;
                int textX = 200 + static_cast<int>(wheelRadius * 0.6f * cosf(midAngle));
                int textY = 200 + static_cast<int>(wheelRadius * 0.6f * sinf(midAngle));
                
                FontFamily fontFamily(L"Arial");
                Font font(&fontFamily, 10, FontStyleBold, UnitPixel);
                StringFormat format;
                format.SetAlignment(StringAlignmentCenter);
                format.SetLineAlignment(StringAlignmentCenter);
                
                SolidBrush textBrush(Color(255, 255, 255, 255));
                RectF textRect(static_cast<float>(textX - 50), 
                             static_cast<float>(textY - 15), 100.0f, 30.0f);
                
                graphics.DrawString(messages[i].c_str(), -1, &font,
                                  textRect, &format, &textBrush);
            }
            
            // Center dead zone
            SolidBrush centerBrush(Color(200, 20, 20, 20));
            graphics.FillEllipse(&centerBrush, 170, 170, 60, 60);
        }
        
        // Blit to screen
        BitBlt(hdc, 0, 0, 400, 400, memDC, 0, 0, SRCCOPY);
        
        // Cleanup
        SelectObject(memDC, oldBitmap);
        DeleteObject(memBitmap);
        DeleteDC(memDC);
    }
    
    void SendMessageToWC3(const std::wstring& message) {
        if (!wc3Window) return;
        
        SetForegroundWindow(wc3Window);
        Sleep(50);
        
        // Send Shift+Enter to open "chat to all" textfield
        INPUT inputs[4] = {};
        
        // Press Shift
        inputs[0].type = INPUT_KEYBOARD;
        inputs[0].ki.wVk = VK_SHIFT;
        
        // Press Enter
        inputs[1].type = INPUT_KEYBOARD;
        inputs[1].ki.wVk = VK_RETURN;
        
        // Release Enter
        inputs[2].type = INPUT_KEYBOARD;
        inputs[2].ki.wVk = VK_RETURN;
        inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;
        
        // Release Shift
        inputs[3].type = INPUT_KEYBOARD;
        inputs[3].ki.wVk = VK_SHIFT;
        inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;
        
        SendInput(4, inputs, sizeof(INPUT));
        Sleep(100);
        
        // Type message
        INPUT input = {};
        input.type = INPUT_KEYBOARD;
        
        for (wchar_t c : message) {
            input.ki.dwFlags = KEYEVENTF_UNICODE;
            input.ki.wVk = 0;
            input.ki.wScan = c;
            SendInput(1, &input, sizeof(INPUT));
            
            input.ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
            SendInput(1, &input, sizeof(INPUT));
        }
        
        Sleep(50);
        
        // Send Enter to submit message
        input.ki.dwFlags = 0;
        input.ki.wVk = VK_RETURN;
        input.ki.wScan = 0;
        SendInput(1, &input, sizeof(INPUT));
        
        input.ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(1, &input, sizeof(INPUT));
    }
    
    static ChatWheel* GetInstance() { return instance; }
    HWND GetHwnd() const { return hwnd; }
    bool IsVisible() const { return visible; }
};

ChatWheel* ChatWheel::instance = nullptr;

// Entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow) {
    ChatWheel wheel;
    
    // Register hotkey (Ctrl+G)
    if (!RegisterHotKey(NULL, 1, MOD_CONTROL, 'G')) {
        MessageBoxW(NULL, L"Failed to register hotkey (Ctrl+G)", L"Error", MB_OK);
        return 1;
    }
    
    MSG msg;
    bool wheelActive = false;
    
    while (GetMessage(&msg, NULL, 0, 0)) {
        if (msg.message == WM_HOTKEY && msg.wParam == 1) {
            if (!wheelActive) {
                wheel.Show();
                wheelActive = true;
            }
        } else if (wheelActive && !(GetAsyncKeyState(VK_CONTROL) & 0x8000)) {
            wheel.Hide();
            wheelActive = false;
        }
        
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    UnregisterHotKey(NULL, 1);
    return 0;
}

// MinGW Entry Point Helper
#ifdef __MINGW32__
int main() {
    return WinMain(GetModuleHandle(NULL), NULL, GetCommandLineA(), SW_SHOW);
}
#endif
