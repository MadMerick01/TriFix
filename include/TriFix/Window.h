#pragma once

#include <Windows.h>

#include <string_view>

namespace trifix {

class Window final {
public:
    Window(HINSTANCE instance, std::wstring_view title, int clientWidth, int clientHeight);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    [[nodiscard]] HWND Handle() const noexcept { return handle_; }
    [[nodiscard]] bool ProcessMessages(int& exitCode) const;
    [[nodiscard]] bool IsMinimized() const noexcept;

private:
    static LRESULT CALLBACK WindowProcedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam);

    HINSTANCE instance_ = nullptr;
    HWND handle_ = nullptr;
};

} // namespace trifix
