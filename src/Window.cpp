#include "TriFix/Window.h"

#include <stdexcept>
#include <string>

namespace {

constexpr wchar_t WindowClassName[] = L"TriFixWindowClass";

} // namespace

namespace trifix {

Window::Window(HINSTANCE instance, std::wstring_view title, int clientWidth, int clientHeight)
    : instance_(instance) {
    WNDCLASSEXW windowClass{};
    windowClass.cbSize = sizeof(windowClass);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WindowProcedure;
    windowClass.hInstance = instance_;
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.lpszClassName = WindowClassName;

    if (RegisterClassExW(&windowClass) == 0) {
        throw std::runtime_error("Failed to register the window class.");
    }

    RECT rectangle{0, 0, clientWidth, clientHeight};
    if (AdjustWindowRect(&rectangle, WS_OVERLAPPEDWINDOW, FALSE) == FALSE) {
        UnregisterClassW(WindowClassName, instance_);
        throw std::runtime_error("Failed to calculate the window size.");
    }

    const std::wstring windowTitle(title);
    handle_ = CreateWindowExW(
        0,
        WindowClassName,
        windowTitle.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        rectangle.right - rectangle.left,
        rectangle.bottom - rectangle.top,
        nullptr,
        nullptr,
        instance_,
        nullptr);

    if (handle_ == nullptr) {
        UnregisterClassW(WindowClassName, instance_);
        throw std::runtime_error("Failed to create the application window.");
    }

    ShowWindow(handle_, SW_SHOWDEFAULT);
}

Window::~Window() {
    if (handle_ != nullptr) {
        DestroyWindow(handle_);
    }
    if (instance_ != nullptr) {
        UnregisterClassW(WindowClassName, instance_);
    }
}

bool Window::ProcessMessages(int& exitCode) const {
    MSG message{};
    while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE) != FALSE) {
        if (message.message == WM_QUIT) {
            exitCode = static_cast<int>(message.wParam);
            return false;
        }

        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
    return true;
}

bool Window::IsMinimized() const noexcept {
    return IsIconic(handle_) != FALSE;
}

LRESULT CALLBACK Window::WindowProcedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    if (message == WM_CLOSE) {
        DestroyWindow(window);
        return 0;
    }
    if (message == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(window, message, wParam, lParam);
}

} // namespace trifix
