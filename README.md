# TriFix 0.07

TriFix is a C++20 Win32/Direct3D 11 prototype. Version 0.07 proves full-frame inverse reprojection with an internally generated, coherent
eye-facing test canvas. The accepted Version 0.06 projected-shape calibration remains byte-for-byte
in `CalibrationPixel.hlsl` as a comparison and fallback. This release does **not** capture or hook
an external application.

## Build and controls

Open `TriFix.sln` in Visual Studio 2022, select `Debug | x64` or `Release | x64`, then
build and run. The Windows 10/11 SDK and **Desktop development with C++** workload are
required.

- **F11** toggles a borderless 7680 x 1440 calibration window beginning at the virtual
  desktop's detected leftmost/topmost origin. Arrange the three target displays left-to-right
  as one 7680 x 1440 row first. If other displays extend above or farther left, automatic
  selection remains intentionally conservative: use windowed mode and Windows Display
  Settings rather than relying on F11.
- **Tab** switches between the new Version 0.07 full-frame test (startup default) and the
  unchanged Version 0.06 projected-shape calibration.
- **D** toggles the 0.07 diagnostic view: red invalid samples, UV encoded as red/green, white
  physical panel borders, green optical panel centres, and an amber 620-pixel rig-value bar.
  The title supplies the calibrated 620 x 349 mm, 50 degree yaw, 520 mm eye and 6 mm bezel values.
- **G** toggles the retained original one-pixel grid fallback independently.
- **Escape** always exits, including from borderless mode.

## Measured calibration profile

The supplied rig has three identical 620 x 349 mm illuminated areas at 2560 x 1440,
6 mm of bezel on every panel edge, a centre display directly ahead, side displays angled
inward 50 degrees, and an eye centred 520 mm in front of the centre plane. The 6 mm value
does not enlarge the active display: two adjacent bezel edges put approximately 12 mm of
physical material between the illuminated areas.

The pattern divides the back buffer into exact LEFT `[0,2560]`, CENTRE `[2560,5120]`, and
RIGHT `[5120,7680]` pixel-edge regions. It marks both joins, every display centre and corner,
horizontal/vertical references, a centre-reference 400-pixel circle and square, a 400-pixel ruler, and a line
continuous across all three regions. The small lower-left key shows the active resolution and
rig measurements; the full values also remain in the window title.

The centre circle and square remain unchanged. Each side retains its lower circle and square and
adds an upper counterpart above the amber line. The upper centre is the exact vertical reflection
of the corresponding lower centre in apparent eye-image-plane coordinates; its radius is shared
with the lower shape. Side shapes are defined on a unit-depth,
eye-centred image plane. For every boundary sample, a ray from the calibrated eye through that
reference point is intersected with the physical side-monitor plane. The resulting monitor-local
metres are converted independently: 620 mm maps to 2560 horizontal pixels and 349 mm maps to
1440 vertical pixels. The inverse calculation in the pixel shader evaluates the same mapping
once per physical display pixel. The outlines therefore distort moderately in local pixels but
round-trip to circles, equal-sided squares, straight edges, and right-angle corners at the eye.
The two sides are exact mirrors, and the amber reference-line construction remains independent.

The correction fixes two related coordinate-space errors. First, the side-panel yaw signs were
reversed: their outer edges extended away from the eye, leaving the modeled displays almost
edge-on rather than angled inward. Second, the previous shader normalized image-plane Y with a
span obtained solely by projecting the panel's horizontal edges. That horizontal image-plane
span is not a vertical metres-per-pixel scale and produced the photographed 495 x 47 and 268 x 37
flattening. Version 0.06 now performs one perspective division into eye-image coordinates and
uses the physical width and height only after ray/monitor-plane intersection.

For the supplied rig, the numerical round trip reports mirrored local-pixel bounds of
`[470.862,830.205]--[920.993,1250.005]` (left circle) and
`[1639.007,830.205]--[2089.138,1250.005]` (right circle), both 450.132 x 419.800 pixels.
The square bounds are `[1671.945,847.691]--[2041.983,1238.725]` on the left and
`[518.017,847.691]--[888.055,1238.725]` on the right, both 370.038 x 391.034 pixels.
All four shapes reconstruct to an apparent 0.164636545 x 0.164636545 unit boundary (aspect
1.000000000). Pixel-space width and height are deliberately not forced equal; the round-trip
eye-plane result, rather than a hand-tuned local-pixel aspect ratio, is the correctness test.

The newly added upper shapes have monitor-local bounds
`[470.862,189.995]--[920.993,609.795]` (left circle) and
`[1639.007,189.995]--[2089.138,609.795]` (right circle), each 450.132 x 419.800 pixels.
The upper-square bounds are `[1671.945,201.275]--[2041.983,592.309]` on the left and
`[518.017,201.275]--[888.055,592.309]` on the right, each 370.038 x 391.034 pixels.
Every new boundary is finite, remains on its assigned panel and above the amber reference line,
and round-trips to 0.164636545 x 0.164636545 apparent units (aspect 1.000000000). Corresponding
upper and lower apparent dimensions are equal, and the two side results are exact mirrors.

## Seated visual check and report

From the normal driving eye position, verify and report:

- whether all three screens are filled at native resolution;
- whether each centre crosshair is physically centred;
- whether the red display boundaries align with the monitor joins;
- whether circles remain circular and squares remain square;
- whether the continuous amber reference line changes height at either join;
- whether Windows scaling or taskbars interfere;
- whether the left and right patterns appear symmetrical.

The retained Version 0.06 view proves that physical poses can be built from hinge geometry, rays
can select all three rectangles, and projected reference shapes round-trip correctly. Version 0.07
extends that proof to every output pixel of an internal canvas; application capture and display
configuration remain future work. Bezel correction, game capture/modification, and head tracking
are expressly not claimed here.


## Version 0.07 full-frame inverse reprojection

The procedural 7680 x 1440 reference canvas represents a 12.0 x 4.6 m eye-facing plane parallel
to the centre display at 520 mm. This deliberately wide field contains the highly oblique hinge
rays without clipping; source pixels need not represent square physical increments. It contains a regular square grid,
large circles and a square, horizontal/vertical references, diagonals, a horizon, three
colour-coded regions, and features that cross the nominal thirds. The centre physical panel maps
linearly about the source centre, so it is an undistorted (scaled) reference view.

For every output pixel the shader first selects its panel and removes the combined-desktop X
offset. It independently converts 2560 X pixels to 620 mm and 1440 Y pixels to 349 mm, constructs
the point from the panel centre and physical right/up basis, and casts the ray from the calibrated
eye. That ray is intersected with the reference plane using exactly one depth division. Only then
is the result converted to source UV/pixels. Invalid UVs are magenta rather than clamped, preventing
edge-smear bands. Both side panels use one formula with a sign parameter; there are no fitted
coefficients, manual mirror operations, shears, or per-side distortion constants. The Geometry
layer exposes the matching CPU operation and remains free of Windows and graphics APIs.

The numerical test uses a `0.01` tolerance (in the reported coordinate's units) and exercises each
panel's centre, four corners, upper/lower probes, inner/outer edges and joins; mirror classification;
finite/out-of-range behavior; ray-to-point reconstruction; centre identity; and a translated second
layout with 700 x 400 mm panels, 35 degree yaw, 950 mm eye-plane distance, and 1920 x 1080 pixels.
It deliberately derives the horizon nowhere: inverse-map correctness is independent of the legacy
yellow-line calculation.

## Rowan physical-rig checklist

Run at native 7680 x 1440 with Windows scaling controlled, press F11, and view from the calibrated
centred eye position 520 mm from the centre plane. Confirm that:

- the complete test image is visible across all three displays;
- circles appear circular and grid cells appear square;
- horizontal and vertical features appear perceptually straight;
- diagonals cross views without visible kinks;
- the horizon is level and continuous and features remain continuous at both joins;
- left and right views are symmetrical;
- **D** shows sensible UV gradients, borders, centres, and conspicuous red invalid areas (if any);
- moving away from the calibrated eye position reduces the apparent correctness;
- **Tab** restores the validated 0.06 pattern unchanged, **G** restores the grid, F11 still spans,
  and Escape exits.

Record a seated photograph and note any discontinuity by monitor and edge. Side output is expected
to look distorted in a flat Windows screenshot; perceived correctness from the calibrated eye is
the criterion. External-game capture, API hooks, injection, head tracking, upscaling, and
post-processing are explicitly outside Version 0.07.
