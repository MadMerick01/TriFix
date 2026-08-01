#include "TriFix/Geometry/MonitorGeometry.h"

#include <algorithm>
#include <cmath>
#include <iterator>

#include "TriFix/Geometry/MonitorLayout.h"

namespace TriFix::Geometry
{
    namespace
    {
        constexpr float Pi = 3.14159265358979323846F;

        [[nodiscard]] constexpr float Dot(const Vector3& left, const Vector3& right) noexcept
        {
            return left.x * right.x + left.y * right.y + left.z * right.z;
        }

        [[nodiscard]] Vector3 MonitorRight(const Monitor& monitor) noexcept
        {
            const float yaw = monitor.yawDegrees * Pi / 180.0F;
            return {std::cos(yaw), 0.0F, -std::sin(yaw)};
        }

        [[nodiscard]] Vector3 MonitorNormal(const Monitor& monitor) noexcept
        {
            const float yaw = monitor.yawDegrees * Pi / 180.0F;
            return {-std::sin(yaw), 0.0F, -std::cos(yaw)};
        }
    }

    std::optional<RayPlaneIntersection> Intersect(
        const Ray& ray, const Plane& plane, const float parallelTolerance) noexcept
    {
        const float denominator = Dot(plane.normal, ray.direction);
        if (std::abs(denominator) <= std::abs(parallelTolerance))
        {
            return std::nullopt;
        }

        const float distance = -(Dot(plane.normal, ray.origin) + plane.distance) / denominator;
        if (distance < 0.0F)
        {
            return std::nullopt;
        }

        return RayPlaneIntersection{
            {ray.origin.x + distance * ray.direction.x,
             ray.origin.y + distance * ray.direction.y,
             ray.origin.z + distance * ray.direction.z},
            distance};
    }

    Plane DisplayPlane(const Monitor& monitor) noexcept
    {
        const Vector3 normal = MonitorNormal(monitor);
        return {normal, -Dot(normal, monitor.position)};
    }

    Vector2 WorldToMonitorLocal(const Vector3& point, const Monitor& monitor) noexcept
    {
        const Vector3 offset{
            point.x - monitor.position.x,
            point.y - monitor.position.y,
            point.z - monitor.position.z};
        return {Dot(offset, MonitorRight(monitor)), offset.y};
    }

    bool IsWithinDisplay(const Vector2& localPoint, const Monitor& monitor) noexcept
    {
        const float halfWidth = monitor.physicalWidthMetres * 0.5F;
        const float halfHeight = monitor.physicalHeightMetres * 0.5F;
        constexpr float edgeTolerance = 1.0e-6F;
        return monitor.physicalWidthMetres > 0.0F && monitor.physicalHeightMetres > 0.0F &&
               std::abs(localPoint.x) <= halfWidth + edgeTolerance &&
               std::abs(localPoint.y) <= halfHeight + edgeTolerance;
    }

    std::optional<Vector2> MonitorLocalToPixels(
        const Vector2& localPoint, const Monitor& monitor) noexcept
    {
        if (!IsWithinDisplay(localPoint, monitor) || monitor.resolutionWidth == 0U ||
            monitor.resolutionHeight == 0U)
        {
            return std::nullopt;
        }

        const float clampedX = std::clamp(localPoint.x, -monitor.physicalWidthMetres * 0.5F,
                                          monitor.physicalWidthMetres * 0.5F);
        const float clampedY = std::clamp(localPoint.y, -monitor.physicalHeightMetres * 0.5F,
                                          monitor.physicalHeightMetres * 0.5F);
        return Vector2{
            (clampedX / monitor.physicalWidthMetres + 0.5F) *
                static_cast<float>(monitor.resolutionWidth),
            (0.5F - clampedY / monitor.physicalHeightMetres) *
                static_cast<float>(monitor.resolutionHeight)};
    }

    std::optional<Vector2> CentreMonitorPixelIntersection(
        const Ray& ray, const MonitorLayout& layout) noexcept
    {
        const Monitor& monitor = layout.centre;
        const auto intersection = Intersect(ray, DisplayPlane(monitor));
        if (!intersection)
        {
            return std::nullopt;
        }
        return MonitorLocalToPixels(WorldToMonitorLocal(intersection->point, monitor), monitor);
    }

    MonitorLayout CreateSymmetricalTripleMonitorLayout(
        const float visibleWidthMetres,
        const float visibleHeightMetres,
        const std::uint32_t resolutionWidth,
        const std::uint32_t resolutionHeight,
        const float bezelWidthMetres,
        const float sideInwardAngleDegrees,
        const Vector3 centrePosition,
        const Vector3 eyePosition) noexcept
    {
        const Monitor centre{visibleWidthMetres, visibleHeightMetres, resolutionWidth,
                             resolutionHeight, centrePosition, 0.0F, bezelWidthMetres};
        const float hingeOffset = visibleWidthMetres * 0.5F + bezelWidthMetres;
        const Vector3 leftHinge{centrePosition.x - hingeOffset, centrePosition.y, centrePosition.z};
        const Vector3 rightHinge{centrePosition.x + hingeOffset, centrePosition.y, centrePosition.z};

        // +yaw turns a panel's right edge toward -Z.  Consequently the left panel must use
        // -yaw (and the right +yaw) so both outer edges come toward the eye.  The previous
        // signs put their outer edges away from the eye and made each display almost edge-on.
        Monitor left{visibleWidthMetres, visibleHeightMetres, resolutionWidth, resolutionHeight,
                     {}, -sideInwardAngleDegrees, bezelWidthMetres};
        Monitor right{visibleWidthMetres, visibleHeightMetres, resolutionWidth, resolutionHeight,
                      {}, sideInwardAngleDegrees, bezelWidthMetres};
        const Vector3 leftRight = MonitorRight(left);
        const Vector3 rightRight = MonitorRight(right);
        left.position = {leftHinge.x - hingeOffset * leftRight.x, leftHinge.y,
                         leftHinge.z - hingeOffset * leftRight.z};
        right.position = {rightHinge.x + hingeOffset * rightRight.x, rightHinge.y,
                          rightHinge.z + hingeOffset * rightRight.z};
        return {left, centre, right, Camera{eyePosition}};
    }

    std::optional<MonitorHit> IntersectMonitorLayout(
        const Ray& ray, const MonitorLayout& layout) noexcept
    {
        const Monitor* const monitors[]{&layout.left, &layout.centre, &layout.right};
        constexpr MonitorId ids[]{MonitorId::Left, MonitorId::Centre, MonitorId::Right};
        std::optional<MonitorHit> nearest;
        float desktopOffset = 0.0F;
        for (std::size_t index = 0; index < std::size(monitors); ++index)
        {
            const Monitor& monitor = *monitors[index];
            const auto planeHit = Intersect(ray, DisplayPlane(monitor));
            if (planeHit)
            {
                const auto pixels = MonitorLocalToPixels(
                    WorldToMonitorLocal(planeHit->point, monitor), monitor);
                if (pixels && (!nearest || planeHit->distance < nearest->distance))
                {
                    nearest = MonitorHit{ids[index], planeHit->point, planeHit->distance, *pixels,
                                         {desktopOffset + pixels->x, pixels->y}};
                }
            }
            desktopOffset += static_cast<float>(monitor.resolutionWidth);
        }
        return nearest;
    }

    ReferenceSample InverseMapToReference(
        const Vector2& monitorLocalPixels, const Monitor& monitor,
        const MonitorLayout& layout, const std::uint32_t sourceWidth,
        const std::uint32_t sourceHeight, const float referenceWidthMetres,
        const float referenceHeightMetres) noexcept
    {
        ReferenceSample result{};
        if (monitor.resolutionWidth == 0U || monitor.resolutionHeight == 0U ||
            sourceWidth == 0U || sourceHeight == 0U || referenceWidthMetres <= 0.0F ||
            referenceHeightMetres <= 0.0F)
            return result;

        const Vector2 metres{
            (monitorLocalPixels.x / static_cast<float>(monitor.resolutionWidth) - 0.5F) *
                monitor.physicalWidthMetres,
            (0.5F - monitorLocalPixels.y / static_cast<float>(monitor.resolutionHeight)) *
                monitor.physicalHeightMetres};
        const Vector3 right = MonitorRight(monitor);
        result.monitorPoint = {monitor.position.x + metres.x * right.x,
                               monitor.position.y + metres.y,
                               monitor.position.z + metres.x * right.z};
        const Vector3 eye = layout.camera.eyePosition;
        result.eyeRay = {result.monitorPoint.x - eye.x, result.monitorPoint.y - eye.y,
                         result.monitorPoint.z - eye.z};
        const float referenceDepth = layout.centre.position.z - eye.z;
        if (std::abs(result.eyeRay.z) <= 1.0e-8F)
            return result;
        // This is the sole perspective division in the inverse map.
        const float scale = referenceDepth / result.eyeRay.z;
        const float x = eye.x + result.eyeRay.x * scale - layout.centre.position.x;
        const float y = eye.y + result.eyeRay.y * scale - layout.centre.position.y;
        result.uv = {x / referenceWidthMetres + 0.5F, 0.5F - y / referenceHeightMetres};
        result.sourcePixels = {result.uv.x * static_cast<float>(sourceWidth),
                               result.uv.y * static_cast<float>(sourceHeight)};
        result.valid = std::isfinite(result.uv.x) && std::isfinite(result.uv.y) &&
                       result.uv.x >= 0.0F && result.uv.x <= 1.0F &&
                       result.uv.y >= 0.0F && result.uv.y <= 1.0F;
        return result;
    }
}
