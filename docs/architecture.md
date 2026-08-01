# Architecture and geometry conventions (Version 0.05)

`Application`, `Window`, and `Renderer` retain platform lifetime, Win32 display handling,
and Direct3D ownership respectively. `TriFix::Geometry` contains no platform or graphics API
dependency. The renderer's diagnostic is a pixel-space full-screen shader, not a projection.

## Coordinates and monitor construction

Version 0.05 preserves Version 0.04 conventions:

- The right-handed world is +X right, +Y up, +Z forward/away from the viewer.
- A zero-yaw monitor lies in XY, faces -Z, and is positioned at its illuminated-area centre.
  Local +X is display-right; local +Y is display-up.
- Yaw is measured from the centre monitor plane. Positive yaw follows the right-hand rule
  about +Y (counter-clockwise viewed from above), turning local +X toward world -Z. Thus the
  left panel has positive inward yaw and the right panel equal negative yaw.
- Width/height describe illuminated area only. `bezelWidthMetres` retains its established
  meaning as one physical panel-edge width and never participates in display hit bounds.

`CreateSymmetricalTripleMonitorLayout` defines centre-panel hinge/join points at local
`x = +/- (visibleWidth/2 + bezelWidth)`. A side panel's adjoining active edge is placed one
more bezel width from the same hinge, along that side panel's yawed axis. Its centre follows
mathematically by another half active width. Consequently each pair of illuminated inner
edges is separated through the join by two 6 mm bezel widths (approximately 12 mm), without
hard-coded world positions. The eye position is stored in the resulting layout.

## Rays and pixels

All three infinite planes are tested. Parallel, negative-parameter, and outside-rectangle
intersections are discarded, and the valid hit with the smallest ray parameter is returned.
Directions need not be normalised and parameter zero remains valid. Bounds include the four
physical edges.

Monitor pixels have a top-left origin, +X right, and +Y down. They are continuous **edge
coordinates**: exact left/top edges are 0 and exact right/bottom edges are the resolution
dimensions (2560 and 1440), although the last addressable pixel indices are 2559 and 1439.
The combined desktop concatenates left at x `[0,2560]`, centre at `[2560,5120]`, and right at
`[5120,7680]`. Shared numeric endpoints describe the continuous region boundaries; monitor
identity disambiguates an exact physical edge hit.

The test rig is 620 x 349 mm visible, 2560 x 1440 per panel, 6 mm bezels, +/-50 degree side
yaw, centre plane z=520 mm with the eye at the origin. Tests also use different dimensions,
resolution, yaw, translation, and eye distance to prevent calibration-profile hard-coding.

## Rendering scope

F11 uses Win32 virtual-desktop coordinates outside `Geometry` to place a borderless 7680 x
1440 window. Tab retains the known Version 0.02 grid path. The Version 0.05 display is solely
diagnostic: it does not perform off-axis projection, bezel/perspective correction, external
image capture or warping, application modification, or head tracking. Those rendering and
configuration steps remain for Version 0.07.


## Full-frame reference mapping

`InverseMapToReference` is platform-independent and is mirrored by `FullFramePixel.hlsl`. It maps
output pixel -> monitor-local metres -> world point -> calibrated-eye ray -> planar reference UV.
Desktop offsets exist only in the renderer's panel selection. Geometry does not include Windows,
D3D, DXGI, HLSL, texture, or presentation types. The shader generates the synthetic source
analytically, which is equivalent to sampling an internal render target while keeping this proof
self-contained and deterministic. Invalid coordinates retain their classification and render
magenta; they are never clamped.
