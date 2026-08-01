#!/usr/bin/env python3
"""Round-trip apparent-geometry validation for the Version 0.06 rig."""

import math
import re
from pathlib import Path

SHADER = Path(__file__).parents[1] / "shaders" / "CalibrationPixel.hlsl"
WIDTH, HEIGHT = 0.620, 0.349
PIXELS = (2560.0, 1440.0)
YAW = math.radians(50.0)
BEZEL = 0.006


def dot(a, b):
    return sum(x * y for x, y in zip(a, b))


class SideMonitor:
    """Physical panel plus its eye-centred unit-depth image plane."""

    def __init__(self, side: int):
        self.side = side
        self.yaw = side * YAW
        self.right = (math.cos(self.yaw), 0.0, -math.sin(self.yaw))
        hinge = WIDTH / 2.0 + BEZEL
        self.centre = (side * hinge + side * hinge * self.right[0], 0.0,
                       0.520 + side * hinge * self.right[2])
        length = math.sqrt(dot(self.centre, self.centre))
        self.forward = tuple(value / length for value in self.centre)
        self.image_right = (self.forward[2], 0.0, -self.forward[0])
        self.normal = (-math.sin(self.yaw), 0.0, -math.cos(self.yaw))

    def pixel_to_world(self, pixel):
        local_x = (pixel[0] / PIXELS[0] - 0.5) * WIDTH
        local_y = (0.5 - pixel[1] / PIXELS[1]) * HEIGHT
        return tuple(self.centre[i] + local_x * self.right[i] +
                     local_y * (1.0 if i == 1 else 0.0) for i in range(3))

    def apparent(self, world):
        """Direction from eye projected onto a unit-depth eye image plane."""
        depth = dot(world, self.forward)
        return (dot(world, self.image_right) / depth, world[1] / depth)

    def reference_to_pixel(self, reference):
        """Ray from eye through a reference-plane point, intersected with the panel."""
        direction = tuple(self.forward[i] + reference[0] * self.image_right[i] +
                          reference[1] * (1.0 if i == 1 else 0.0) for i in range(3))
        distance = dot(self.normal, self.centre) / dot(self.normal, direction)
        world = tuple(distance * value for value in direction)
        offset = tuple(world[i] - self.centre[i] for i in range(3))
        local_x, local_y = dot(offset, self.right), offset[1]
        # Independent physical-axis conversion is essential: 620 mm -> 2560 X pixels,
        # while 349 mm -> 1440 Y pixels.  No projected horizontal span enters Y.
        return ((local_x / WIDTH + 0.5) * PIXELS[0],
                (0.5 - local_y / HEIGHT) * PIXELS[1])

    def shape_centre(self, reference_x, vertical):
        local_x = reference_x if self.side < 0 else PIXELS[0] - reference_x
        lower = self.apparent(self.pixel_to_world((local_x, 1040.0)))
        return (lower[0], lower[1] if vertical == "lower" else -lower[1])

    def radius(self):
        top = self.apparent(self.pixel_to_world((1280.0, 520.0)))
        bottom = self.apparent(self.pixel_to_world((1280.0, 920.0)))
        return math.dist(top, bottom) / 2.0


def bounds(points):
    return (min(p[0] for p in points), min(p[1] for p in points),
            max(p[0] for p in points), max(p[1] for p in points))


def validate_shape(monitor, name, vertical):
    centre = monitor.shape_centre(700.0 if name == "circle" else 1860.0, vertical)
    radius = monitor.radius()
    reference = []
    if name == "circle":
        reference = [(centre[0] + radius * math.cos(i * math.tau / 720),
                      centre[1] + radius * math.sin(i * math.tau / 720)) for i in range(720)]
    else:
        for edge in range(4):
            for i in range(181):
                value = -radius + 2.0 * radius * i / 180.0
                reference.append(((centre[0] + value, centre[1] - radius),
                                  (centre[0] + radius, centre[1] + value),
                                  (centre[0] - value, centre[1] + radius),
                                  (centre[0] - radius, centre[1] - value))[edge])

    pixels = [monitor.reference_to_pixel(point) for point in reference]
    assert all(math.isfinite(value) for point in pixels for value in point)
    assert all(0.0 <= x <= PIXELS[0] and 0.0 <= y <= PIXELS[1] for x, y in pixels)
    # Required reconstruction: local pixel -> physical panel -> eye direction -> image plane.
    reconstructed = [monitor.apparent(monitor.pixel_to_world(point)) for point in pixels]
    max_round_trip = max(math.dist(a, b) for a, b in zip(reference, reconstructed))
    assert max_round_trip < 1e-12
    apparent_bounds = bounds(reconstructed)
    apparent_width = apparent_bounds[2] - apparent_bounds[0]
    apparent_height = apparent_bounds[3] - apparent_bounds[1]
    assert math.isclose(apparent_width, 2.0 * radius, rel_tol=2e-5)
    assert math.isclose(apparent_height, 2.0 * radius, rel_tol=2e-5)

    if name == "circle":
        radial_error = max(abs(math.dist(point, centre) - radius) for point in reconstructed)
        assert radial_error < 1e-12
    else:
        corners = [(centre[0] - radius, centre[1] - radius),
                   (centre[0] + radius, centre[1] - radius),
                   (centre[0] + radius, centre[1] + radius),
                   (centre[0] - radius, centre[1] + radius)]
        for index, corner in enumerate(corners):
            previous = corners[index - 1]
            following = corners[(index + 1) % 4]
            a = (previous[0] - corner[0], previous[1] - corner[1])
            b = (following[0] - corner[0], following[1] - corner[1])
            assert abs(dot(a, b)) < 1e-12  # four right angles
        for edge in range(4):
            samples = reconstructed[edge * 181:(edge + 1) * 181]
            start, end = samples[0], samples[-1]
            assert max(abs((end[0] - start[0]) * (p[1] - start[1]) -
                           (end[1] - start[1]) * (p[0] - start[0])) for p in samples) < 1e-12

    pixel_bounds = bounds(pixels)
    assert pixel_bounds[3] - pixel_bounds[1] > 300.0
    assert (pixel_bounds[2] - pixel_bounds[0]) / (pixel_bounds[3] - pixel_bounds[1]) < 2.0
    print(f"{('left' if monitor.side < 0 else 'right')} {vertical} {name}: "
          f"local pixels [{pixel_bounds[0]:.3f}, {pixel_bounds[1]:.3f}]--"
          f"[{pixel_bounds[2]:.3f}, {pixel_bounds[3]:.3f}] "
          f"({pixel_bounds[2]-pixel_bounds[0]:.3f}x{pixel_bounds[3]-pixel_bounds[1]:.3f}); "
          f"apparent={apparent_width:.9f}x{apparent_height:.9f}, "
          f"aspect={apparent_width/apparent_height:.9f}")
    return pixel_bounds, apparent_bounds


def main():
    source = SHADER.read_text(encoding="utf-8")
    assert re.search(r"dot\(ray, imageRight\) / depth, ray\.y / depth", source)
    assert "referenceMetresPerPixel" not in source and "referenceSpan" not in source
    assert re.search(r"float yaw = region == 0u \? -sideYaw : sideYaw", source)
    assert re.search(r"return float2\(lowerCentre\.x, -lowerCentre\.y\)", source)
    assert "region != 1u" in source

    results = {}
    for side in (-1, 1):
        monitor = SideMonitor(side)
        for vertical in ("lower", "upper"):
            for name in ("circle", "square"):
                results[side, vertical, name] = validate_shape(monitor, name, vertical)

    for vertical in ("lower", "upper"):
        for name in ("circle", "square"):
            left, right = results[-1, vertical, name][0], results[1, vertical, name][0]
            assert math.isclose(left[0] + right[2], PIXELS[0], abs_tol=1e-8)
            assert math.isclose(left[2] + right[0], PIXELS[0], abs_tol=1e-8)
            assert math.isclose(left[1], right[1], abs_tol=1e-8)
            assert math.isclose(left[3], right[3], abs_tol=1e-8)
            left_monitor, right_monitor = SideMonitor(-1), SideMonitor(1)
            reference_x = 700.0 if name == "circle" else 1860.0
            left_centre = left_monitor.shape_centre(reference_x, vertical)
            right_centre = right_monitor.shape_centre(reference_x, vertical)
            assert math.isclose(left_centre[0], -right_centre[0], abs_tol=1e-15)
            assert math.isclose(left_centre[1], right_centre[1], abs_tol=1e-15)
            assert math.isclose(left_monitor.radius(), right_monitor.radius(), abs_tol=1e-15)

    for side in (-1, 1):
        for name in ("circle", "square"):
            lower = results[side, "lower", name][1]
            upper = results[side, "upper", name][1]
            assert math.isclose(lower[2] - lower[0], upper[2] - upper[0], abs_tol=1e-12)
            assert math.isclose(lower[3] - lower[1], upper[3] - upper[1], abs_tol=1e-12)
            assert math.isclose(lower[0], upper[0], abs_tol=1e-12)
            assert math.isclose(lower[2], upper[2], abs_tol=1e-12)
            assert math.isclose(lower[1], -upper[3], abs_tol=1e-12)
            assert math.isclose(lower[3], -upper[1], abs_tol=1e-12)

    # Every upper boundary is safely above even the lowest (inner-edge) yellow line.
    assert all(results[side, "upper", name][0][3] < 650.0
               for side in (-1, 1) for name in ("circle", "square"))
    print("Round-trip lower/upper apparent circle/square geometry passed validation.")


if __name__ == "__main__":
    main()
