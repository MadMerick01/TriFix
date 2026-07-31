# Architecture

TriFix keeps platform lifetime, message handling, and graphics ownership separate:

- `Application` owns the top-level services and drives the main loop.
- `Window` owns Win32 class registration and the native window.
- `Renderer` owns the Direct3D 11 device, immediate context, swap chain, and render target through `ComPtr` RAII handles.

The renderer recreates its back-buffer view when the client area changes. Its calibration pass draws a full-screen vertex-buffer quad and generates a one-pixel grid in a dedicated pixel shader. The pass binds its own pipeline state and derives coverage from the current viewport, leaving future reprojection and presentation passes free to extend the renderer without moving Win32 responsibilities into it.

## Geometry conventions (Version 0.04)

The reusable `TriFix::Geometry` layer has no Win32, Direct3D, DXGI, or DirectX dependency. Version 0.04 calculates ray intersections and pixel positions for `MonitorLayout::centre` only; it is intentionally not called by the existing renderer.

- TriFix uses a right-handed world: **+X is right, +Y is up, and +Z is forward/away from the viewer**.
- A zero-yaw monitor is centred at `Monitor::position`, lies in the XY plane, and has its display-facing normal along -Z. The position is the physical display area's centre and the monitor-local origin.
- Positive yaw rotates counter-clockwise about +Y when viewed from above (+Y looking toward the origin), following the right-hand rule. Consequently, positive yaw turns the monitor's local +X edge toward world -Z.
- Monitor-local +X runs to the display's right edge and local +Y runs to its top edge. Coordinates are metres relative to the display centre.
- `physicalWidthMetres` and `physicalHeightMetres` describe only the active physical display area. `bezelWidthMetres` is metadata and is **not** included in intersection bounds or pixel conversion.
- Pixel coordinates use the top-left display edge as `(0, 0)`, with +X right and +Y down. They are continuous pixel-edge coordinates: the right and bottom edges are `(resolutionWidth, resolutionHeight)`, while pixel indices remain one less than those dimensions.

Ray directions need not be normalised: the reported intersection distance is the ray parameter. A hit at parameter zero is valid; parallel rays and negative-parameter intersections are rejected. Display bounds include all four physical edges.

The Version 0.02 full-screen calibration grid and its projection-free rendering path remain visually and functionally unchanged. A later milestone will connect physical geometry to reprojection, introduce warping/projection changes, and add side-monitor runtime calculations.
