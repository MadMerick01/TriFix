#include "TriFix/Application.h"

#include <Windows.h>

namespace trifix {

Application::Application(HINSTANCE instance)
    : window_(instance, L"TriFix 0.05 - 620x349 mm, 2560x1440 x3, yaw 50 deg, eye 520 mm, bezel 6 mm", 1280, 720),
      renderer_(window_.Handle()) {}

int Application::Run() {
    int exitCode = 0;
    while (window_.ProcessMessages(exitCode)) {
        const bool f11Down = (GetAsyncKeyState(VK_F11) & 0x8000) != 0;
        if (f11Down && !f11WasDown_) {
            window_.ToggleSpanning();
        }
        f11WasDown_ = f11Down;
        const bool tabDown = (GetAsyncKeyState(VK_TAB) & 0x8000) != 0;
        if (tabDown && !tabWasDown_) {
            renderer_.ToggleCalibrationMode();
        }
        tabWasDown_ = tabDown;
        if (window_.IsMinimized()) {
            WaitMessage();
            continue;
        }

        renderer_.Render();
    }

    return exitCode;
}

} // namespace trifix
