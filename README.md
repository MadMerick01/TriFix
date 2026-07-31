# TriFix 0.03

TriFix is a C++20 Win32 and Direct3D 11 application scaffold. Version 0.03 adds rendering-independent geometry data structures for future monitor reprojection while retaining the Version 0.02 calibration grid.

## Build

1. Open `TriFix.sln` in Visual Studio 2022 Community.
2. Select either `Debug | x64` or `Release | x64`.
3. Build and run the `TriFix` project.

The Windows 10/11 SDK and the **Desktop development with C++** workload are required.

## Layout

- `src/` — implementation and Windows entry point
- `include/` — public application, window, and rendering interfaces
- `include/TriFix/Geometry/` — rendering-independent geometry data structures
- `shaders/` — HLSL vertex and pixel shaders
- `assets/` — reserved for runtime assets
- `config/` — reserved for calibration and application configuration
- `docs/` — design and milestone documentation
