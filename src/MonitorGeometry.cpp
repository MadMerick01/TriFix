#include "TriFix/Geometry/MonitorGeometry.h"

#include <cmath>

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
        return monitor.physicalWidthMetres > 0.0F && monitor.physicalHeightMetres > 0.0F &&
               std::abs(localPoint.x) <= halfWidth && std::abs(localPoint.y) <= halfHeight;
    }

    std::optional<Vector2> MonitorLocalToPixels(
        const Vector2& localPoint, const Monitor& monitor) noexcept
    {
        if (!IsWithinDisplay(localPoint, monitor) || monitor.resolutionWidth == 0U ||
            monitor.resolutionHeight == 0U)
        {
            return std::nullopt;
        }

        return Vector2{
            (localPoint.x / monitor.physicalWidthMetres + 0.5F) *
                static_cast<float>(monitor.resolutionWidth),
            (0.5F - localPoint.y / monitor.physicalHeightMetres) *
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
}
