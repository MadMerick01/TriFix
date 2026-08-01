# TriFix 0.06

TriFix is a C++20 Win32/Direct3D 11 prototype. Version 0.06 adds physically projected
side-monitor reference shapes to the separately tested triple-monitor geometry and diagnostic
spanning calibration pattern. It does **not**
yet capture, warp, or perspective-correct another application.

## Build and controls

Open `TriFix.sln` in Visual Studio 2022, select `Debug | x64` or `Release | x64`, then
build and run. The Windows 10/11 SDK and **Desktop development with C++** workload are
required.

- **F11** toggles a borderless 7680 x 1440 calibration window beginning at the virtual
  desktop's detected leftmost/topmost origin. Arrange the three target displays left-to-right
  as one 7680 x 1440 row first. If other displays extend above or farther left, automatic
  selection remains intentionally conservative: use windowed mode and Windows Display
  Settings rather than relying on F11.
- **Tab** switches between the Version 0.06 triple diagnostic and the retained Version
  0.02 one-pixel grid.
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

The centre circle and square remain unchanged. Side shapes are defined on a unit-depth,
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

## Seated visual check and report

From the normal driving eye position, verify and report:

- whether all three screens are filled at native resolution;
- whether each centre crosshair is physically centred;
- whether the red display boundaries align with the monitor joins;
- whether circles remain circular and squares remain square;
- whether the continuous amber reference line changes height at either join;
- whether Windows scaling or taskbars interfere;
- whether the left and right patterns appear symmetrical.

Version 0.06 proves that physical poses can be built from hinge geometry, rays can select and
address all three visible rectangles, and a native combined-desktop diagnostic can be
presented with perspective-correct reference shapes. A later version still needs application capture and full reprojection integration and
configuration/display selection. Bezel correction, game capture/modification, and head
tracking are expressly not claimed here.
