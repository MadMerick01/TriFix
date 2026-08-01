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

The centre circle and square remain unchanged. On each side, every sampled pixel ray is
intersected with an eye-facing reference plane derived from the physical monitor pose. Their
outlines therefore look distorted in desktop pixel space but circular and square from the
calibrated eye point; the two sides are exact geometric mirrors. This diagnostic projection is
independent of the separately retained amber reference-line construction.

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
