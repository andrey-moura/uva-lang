#include <windows.h>

#include <frame.hpp>

#include <uva-ui/app.hpp>

extern uva::lang::ui::app* uvaapp;

extern HINSTANCE hInstance;

struct window_data {
    HWND hwnd;
};

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            // All painting occurs here, between BeginPaint and EndPaint.

            FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW+1));

            EndPaint(hwnd, &ps);
        }
        return 0;

    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

uva::lang::ui::frame::frame(std::string_view __title)
{
    const TCHAR* class_name = TEXT("uva::lang::ui::frame");

    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = class_name;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, class_name, __title.data(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, hInstance, nullptr);

    if(hwnd == nullptr) {
        throw std::runtime_error("Failed to create window");
    }

    os_specific_data_as<window_data>()->hwnd = hwnd;
}

uva::lang::ui::frame::~frame()
{

}

void uva::lang::ui::frame::show(bool maximized)
{
    HWND hwnd = os_specific_data_as<window_data>()->hwnd;

    ShowWindow(hwnd, maximized ? SW_MAXIMIZE : SW_SHOW);
    UpdateWindow(hwnd);
}

void uva::lang::ui::frame::hide()
{

}