# Architecture

TriFix keeps platform lifetime, message handling, and graphics ownership separate:

- `Application` owns the top-level services and drives the main loop.
- `Window` owns Win32 class registration and the native window.
- `Renderer` owns the Direct3D 11 device, immediate context, swap chain, and render target through `ComPtr` RAII handles.

The renderer recreates its back-buffer view when the client area changes. Its calibration pass draws a full-screen vertex-buffer quad and generates a one-pixel grid in a dedicated pixel shader. The pass binds its own pipeline state and derives coverage from the current viewport, leaving future reprojection and presentation passes free to extend the renderer without moving Win32 responsibilities into it.
