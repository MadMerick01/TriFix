# Architecture

TriFix keeps platform lifetime, message handling, and graphics ownership separate:

- `Application` owns the top-level services and drives the main loop.
- `Window` owns Win32 class registration and the native window.
- `Renderer` owns the Direct3D 11 device, immediate context, swap chain, and render target through `ComPtr` RAII handles.

The renderer recreates its back-buffer view when the client area changes. No graphics pipeline or shader is required for the initial clear-only frame. Future capture, calibration, reprojection, and presentation components can therefore be added without moving Win32 responsibilities into the renderer.
