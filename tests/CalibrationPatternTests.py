#!/usr/bin/env python3
"""Focused source-level validation for the calibration reference line."""

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

    print("Calibration reference line is horizontal, continuous, and mirrored.")


if __name__ == "__main__":
    main()
