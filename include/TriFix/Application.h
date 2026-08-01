#pragma once

#include "TriFix/Renderer.h"
#include "TriFix/Window.h"

namespace trifix {

class Application final {
public:
    explicit Application(HINSTANCE instance);

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    int Run();

private:
    Window window_;
    Renderer renderer_;
    bool f11WasDown_ = false;
    bool tabWasDown_ = false;
};

} // namespace trifix
