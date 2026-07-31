#pragma once

#include <optional>

#include "TriFix/Geometry/Monitor.h"
#include "TriFix/Geometry/MonitorLayout.h"
#include "TriFix/Geometry/Plane.h"
#include "TriFix/Geometry/Ray.h"
#include "TriFix/Geometry/Vector2.h"

namespace TriFix::Geometry
{
    /// A forward ray/plane intersection. Distance is the ray parameter t.
    struct RayPlaneIntersection final
    {
        Vector3 point{};
        float distance{};
    };

    /// Returns no value when the ray is parallel to the plane or the hit is behind its origin.
    [[nodiscard]] std::optional<RayPlaneIntersection> Intersect(
        const Ray& ray, const Plane& plane, float parallelTolerance = 1.0e-6F) noexcept;

    /// Builds the physical display plane for a monitor (bezel is deliberately excluded).
    [[nodiscard]] Plane DisplayPlane(const Monitor& monitor) noexcept;

    /// Converts a world point to centre-origin monitor coordinates: +X right and +Y up.
    [[nodiscard]] Vector2 WorldToMonitorLocal(const Vector3& point, const Monitor& monitor) noexcept;

    /// Tests the closed physical display rectangle. Bezel width does not contribute to its size.
    [[nodiscard]] bool IsWithinDisplay(const Vector2& localPoint, const Monitor& monitor) noexcept;

    /// Converts a valid local point to pixel-edge coordinates with a top-left origin and +Y down.
    /// The right and bottom physical edges map to resolutionWidth and resolutionHeight.
    [[nodiscard]] std::optional<Vector2> MonitorLocalToPixels(
        const Vector2& localPoint, const Monitor& monitor) noexcept;

    /// Version 0.04's complete calculation, intentionally operating on only the centre monitor.
    [[nodiscard]] std::optional<Vector2> CentreMonitorPixelIntersection(
        const Ray& ray, const MonitorLayout& layout) noexcept;
}
