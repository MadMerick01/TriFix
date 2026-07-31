# TriFix 0.02

TriFix is a C++20 Win32 and Direct3D 11 application scaffold. Version 0.02 renders a GPU-generated, pixel-accurate calibration grid over a dark grey back buffer and adapts it to the resizable window.

## Build

1. Open `TriFix.sln` in Visual Studio 2022 Community.
2. Select either `Debug | x64` or `Release | x64`.
3. Build and run the `TriFix` project.

The Windows 10/11 SDK and the **Desktop development with C++** workload are required.

## Layout

- `src/` — implementation and Windows entry point
- `include/` — public application, window, and rendering interfaces
- `shaders/` — HLSL vertex and pixel shaders
- `assets/` — reserved for runtime assets
- `config/` — reserved for calibration and application configuration
- `docs/` — design and milestone documentation
