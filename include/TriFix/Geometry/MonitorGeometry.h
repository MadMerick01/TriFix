#pragma once

#include <optional>

#include "TriFix/Geometry/Monitor.h"
#include "TriFix/Geometry/MonitorLayout.h"
#include "TriFix/Geometry/Plane.h"
#include "TriFix/Geometry/Ray.h"
#include "TriFix/Geometry/Vector2.h"

namespace TriFix::Geometry
{
    enum class MonitorId
    {
        Left,
        Centre,
        Right
    };

    struct MonitorHit final
    {
        MonitorId monitor{};
        Vector3 worldPoint{};
        float distance{};
        Vector2 localPixels{};
        Vector2 desktopPixels{};
    };

    /// A forward ray/plane intersection. Distance is the ray parameter t.
    struct RayPlaneIntersection final
    {
        Vector3 point{};
        float distance{};
    };

    /// Result of inverse-mapping one physical output pixel to an eye-facing planar canvas.
    struct ReferenceSample final
    {
        Vector3 monitorPoint{};
        Vector3 eyeRay{}; // deliberately unnormalised: its direction is what is projected
        Vector2 uv{};
        Vector2 sourcePixels{};
        bool valid{};
    };

    /// Reconstructs a pixel centre on monitor, casts its calibrated-eye ray, and intersects
    /// the plane parallel to the centre display. The canvas is centred on the centre ray.
    [[nodiscard]] ReferenceSample InverseMapToReference(
        const Vector2& monitorLocalPixels, const Monitor& monitor,
        const MonitorLayout& layout, std::uint32_t sourceWidth,
        std::uint32_t sourceHeight, float referenceWidthMetres,
        float referenceHeightMetres) noexcept;

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

    /// Creates three identical visible rectangles about the centre monitor. The two physical
    /// hinge points sit one bezel width beyond the centre display edges; each side display is
    /// placed a further bezel width beyond that same hinge along its own plane.
    [[nodiscard]] MonitorLayout CreateSymmetricalTripleMonitorLayout(
        float visibleWidthMetres,
        float visibleHeightMetres,
        std::uint32_t resolutionWidth,
        std::uint32_t resolutionHeight,
        float bezelWidthMetres,
        float sideInwardAngleDegrees,
        Vector3 centrePosition,
        Vector3 eyePosition) noexcept;

    /// Intersects every closed display rectangle and returns the smallest valid ray parameter.
    /// Desktop coordinates concatenate left, centre, then right at their full pixel widths.
    [[nodiscard]] std::optional<MonitorHit> IntersectMonitorLayout(
        const Ray& ray, const MonitorLayout& layout) noexcept;
}
