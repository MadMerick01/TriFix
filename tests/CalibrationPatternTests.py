#!/usr/bin/env python3
"""Focused numerical and source validation for the calibration pattern."""

import math
import re
from pathlib import Path


SHADER = Path(__file__).parents[1] / "shaders" / "CalibrationPixel.hlsl"


def require(pattern: str, source: str) -> re.Match[str]:
    match = re.search(pattern, source, re.DOTALL)
    if match is None:
        raise AssertionError(f"Calibration shader does not match: {pattern}")
    return match


def main() -> None:
    source = SHADER.read_text(encoding="utf-8")
    function = require(
        r"float PerspectiveReferenceY\(float desktopX, uint region\)\s*\{(.*?)\n\}",
        source,
    ).group(1)
    reference_y = float(
        require(r"const float referenceY = ([0-9.]+)f;", function).group(1)
    )
    slope_width = float(
        require(
            r"return referenceY \+ abs\(desktopX - innerJoinX\) / ([0-9.]+)f;",
            function,
        ).group(1)
    )
    require(r"if \(region == 1u\)\s*return referenceY;", function)
    require(
        r"const float innerJoinX = region == 0u \? 2560\.0f : 5120\.0f;",
        function,
    )

    def line_y(desktop_x: float, region: int) -> float:
        if region == 1:
            return reference_y
        inner_join_x = 2560.0 if region == 0 else 5120.0
        return reference_y + abs(desktop_x - inner_join_x) / slope_width

    left_outer = line_y(0.0, 0)
    left_join = line_y(2560.0, 0)
    centre_left = line_y(2560.0, 1)
    centre_right = line_y(5120.0, 1)
    right_join = line_y(5120.0, 2)
    right_outer = line_y(7680.0, 2)

    assert centre_left == centre_right
    assert left_join == centre_left == centre_right == right_join
    assert left_outer == right_outer
    assert math.isclose(left_outer - left_join, right_outer - right_join)

    # Independently reproduce the shader's general ray/plane construction. A point on
    # either eye-facing reference plane is projected onto its physical monitor plane.
    width, height, bezel = 0.620, 0.349, 0.006
    resolution = (2560.0, 1440.0)
    angle = math.radians(50.0)
    eye = (0.0, 0.0, 0.0)

    def dot(a: tuple[float, ...], b: tuple[float, ...]) -> float:
        return sum(x * y for x, y in zip(a, b))

    def projected(reference_pixel: tuple[float, float], side: int) -> tuple[float, float]:
        yaw = -side * angle
        right = (math.cos(yaw), 0.0, -math.sin(yaw))
        hinge = width / 2.0 + bezel
        centre = (side * hinge + side * hinge * right[0], 0.0,
                  0.520 + side * hinge * right[2])
        normal_length = math.sqrt(dot(centre, centre))
        view_normal = tuple(value / normal_length for value in centre)
        view_right = (view_normal[2], 0.0, -view_normal[0])
        def reference_x(monitor_x: float) -> float:
            edge = tuple(centre[i] + monitor_x * right[i] for i in range(3))
            scale = dot(view_normal, centre) / dot(view_normal, edge)
            projected_edge = tuple(scale * value for value in edge)
            return dot(tuple(projected_edge[i] - centre[i] for i in range(3)), view_right)

        reference_left = reference_x(-width / 2.0)
        reference_right = reference_x(width / 2.0)
        reference_min = min(reference_left, reference_right)
        reference_span = abs(reference_right - reference_left)
        metres_per_pixel = reference_span / resolution[0]
        local = (reference_min + reference_pixel[0] * metres_per_pixel,
                 (resolution[1] * 0.5 - reference_pixel[1]) * metres_per_pixel)
        target = tuple(centre[i] + local[0] * view_right[i] +
                       local[1] * (1.0 if i == 1 else 0.0) for i in range(3))
        monitor_normal = (-math.sin(yaw), 0.0, -math.cos(yaw))
        ray_scale = dot(monitor_normal, centre) / dot(monitor_normal, target)
        hit = tuple(ray_scale * value for value in target)
        offset = tuple(hit[i] - centre[i] for i in range(3))
        monitor_local = (dot(offset, right), offset[1])
        return ((monitor_local[0] / width + 0.5) * resolution[0],
                (0.5 - monitor_local[1] / height) * resolution[1])

    square_corners = [(1660.0, 840.0), (2060.0, 840.0),
                      (2060.0, 1240.0), (1660.0, 1240.0)]
    left = [projected(point, -1) for point in square_corners]
    right = [projected((resolution[0] - point[0], point[1]), 1)
             for point in square_corners]
    for left_point, right_point in zip(left, right):
        assert math.isclose(left_point[0] + right_point[0], resolution[0], abs_tol=1e-9)
        assert math.isclose(left_point[1], right_point[1], abs_tol=1e-9)
    # Perspective makes the projected corners non-affine in pixel space.
    assert not math.isclose(left[1][1] - left[0][1], left[2][1] - left[3][1])

    def validate_bounds(name: str, points: list[tuple[float, float]],
                        desktop_offset: float) -> tuple[float, ...]:
        assert points and all(math.isfinite(value) for point in points for value in point)
        bounds = (min(point[0] for point in points), min(point[1] for point in points),
                  max(point[0] for point in points), max(point[1] for point in points))
        assert bounds[0] < resolution[0] and bounds[2] > 0.0
        assert bounds[1] < resolution[1] and bounds[3] > 0.0
        assert 10.0 < bounds[2] - bounds[0] <= resolution[0]
        assert 10.0 < bounds[3] - bounds[1] <= resolution[1]
        # Local side-monitor coordinates must not already contain a 2560/5120 desktop offset.
        assert all(-1.0 <= value <= resolution[0] + 1.0 for point in points for value in point[:1])
        combined = (bounds[0] + desktop_offset, bounds[1],
                    bounds[2] + desktop_offset, bounds[3])
        assert combined[0] < desktop_offset + resolution[0] and combined[2] > desktop_offset
        print(f"{name}: x=[{bounds[0]:.3f}, {bounds[2]:.3f}], "
              f"y=[{bounds[1]:.3f}, {bounds[3]:.3f}], "
              f"size={bounds[2]-bounds[0]:.3f}x{bounds[3]-bounds[1]:.3f} px; "
              f"desktop x=[{combined[0]:.3f}, {combined[2]:.3f}]")
        return bounds

    circle_points = [(700.0 + 200.0 * math.cos(index * math.tau / 720.0),
                      1040.0 + 200.0 * math.sin(index * math.tau / 720.0))
                     for index in range(720)]
    shapes = {"square": square_corners, "circle": circle_points}
    for shape_name, reference_points in shapes.items():
        left_points = [projected(point, -1) for point in reference_points]
        right_points = [projected((resolution[0] - point[0], point[1]), 1)
                        for point in reference_points]
        left_bounds = validate_bounds(f"left {shape_name}", left_points, 0.0)
        right_bounds = validate_bounds(f"right {shape_name}", right_points, 5120.0)
        for left_value, right_value in zip(left_bounds, right_bounds):
            assert math.isfinite(left_value) and math.isfinite(right_value)
        assert math.isclose(left_bounds[0] + right_bounds[2], resolution[0], abs_tol=1e-6)
        assert math.isclose(left_bounds[2] + right_bounds[0], resolution[0], abs_tol=1e-6)
        assert math.isclose(left_bounds[1], right_bounds[1], abs_tol=1e-6)
        assert math.isclose(left_bounds[3], right_bounds[3], abs_tol=1e-6)

    shape_function = require(
        r"float2 ApparentShapePixels\(float2 localPixels, uint region\)\s*\{(.*?)\n\}",
        source,
    ).group(1)
    require(r"dot\(viewNormal, monitorCentre - eye\) / dot\(viewNormal, ray\)", shape_function)
    require(r"const float2 visibleMetres", shape_function)
    require(r"float referenceSpan = abs\(rightReferenceX - leftReferenceX\)", shape_function)
    require(r"float referenceMetresPerPixel = referenceSpan / resolution\.x", shape_function)
    assert "referencePixels.x = resolution.x - referencePixels.x" not in shape_function
    assert "PerspectiveReferenceY" not in shape_function
    assert not re.search(r"shear|shapeSlope|fittedOffset", shape_function, re.IGNORECASE)

    print("Calibration line and geometric side-shape projection passed validation.")


if __name__ == "__main__":
    main()
