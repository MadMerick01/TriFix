"""Structural regression checks for the CPU/HLSL inverse-map contract."""
from pathlib import Path

root = Path(__file__).parents[1]
shader = (root / "shaders/FullFramePixel.hlsl").read_text()
geometry = (root / "src/MonitorGeometry.cpp").read_text()

assert "panel == 0u ? -radians(50.0f)" in shader, "left yaw must remain negative"
assert "Visible.y" in shader and "resolutionHeight" in geometry, "Y must use physical height"
assert shader.count("Depth / point.z") == 1, "perspective division must occur exactly once"
assert "position.x - panel * Resolution.x" in shader, "desktop X must become panel-local first"
assert "side * radians(50.0f)" in shader, "sides must share one signed formula"
assert "clamp(" not in shader, "invalid samples must not smear source edges"
assert "PerspectiveReferenceY" not in shader, "mapping must not depend on yellow-line math"
for forbidden in ("distortion", "shear", "fitted", "leftOffset", "rightOffset"):
    assert forbidden not in shader, f"rig-specific tuning token present: {forbidden}"
assert "#include <Windows" not in geometry and "d3d" not in geometry.lower()
print("Full-frame inverse-mapping structural checks passed.")
