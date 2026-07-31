#include "TriFix/Application.h"

#include <Windows.h>

namespace trifix {

Application::Application(HINSTANCE instance)
    : window_(instance, L"TriFix 0.01", 1280, 720),
      renderer_(window_.Handle()) {}

int Application::Run() {
    int exitCode = 0;
    while (window_.ProcessMessages(exitCode)) {
        if (window_.IsMinimized()) {
            WaitMessage();
            continue;
        }

        renderer_.Render();
    }

    return exitCode;
}

} // namespace trifix
