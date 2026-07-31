#include "TriFix/Application.h"

#include <Windows.h>

#include <cstdlib>
#include <exception>

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int) {
    try {
        trifix::Application application(instance);
        return application.Run();
    } catch (const std::exception& exception) {
        MessageBoxA(nullptr, exception.what(), "TriFix startup error", MB_OK | MB_ICONERROR);
        return EXIT_FAILURE;
    }
}
