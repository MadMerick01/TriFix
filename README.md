# TriFix 0.04

TriFix is a C++20 Win32 and Direct3D 11 application scaffold. Version 0.04 adds tested, rendering-independent ray-to-centre-monitor geometry and physical-to-pixel conversion while retaining the Version 0.02 calibration grid unchanged.

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

## Version 0.04 scope

The geometry layer can intersect a ray with a plane, reject parallel and rearward intersections, transform a world point into centre-origin monitor-local physical coordinates, validate it against the bezel-excluding display rectangle, and map it to pixel-edge coordinates. The combined operation deliberately uses only `MonitorLayout::centre`; left/right runtime support is deferred.

No geometry calculation is connected to the renderer in this milestone. Version 0.04 does not perform perspective correction, image warping, projection changes, or calibration-grid replacement. The next milestone will integrate the geometry with a rendering/capture path and can then extend the calculation across the side monitors.
