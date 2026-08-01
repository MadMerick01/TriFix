#include "TriFix/Geometry/MonitorGeometry.h"

#include <cmath>
#include <cstdlib>
#include <iostream>

using namespace TriFix::Geometry;

namespace
{
    constexpr float Epsilon = 1.0e-2F;

    [[nodiscard]] bool Near(const float actual, const float expected) noexcept
    {
        return std::abs(actual - expected) < Epsilon;
    }

    void Require(const bool condition, const char* const description)
    {
        if (!condition)
        {
            std::cerr << "FAILED: " << description << '\n';
            std::exit(EXIT_FAILURE);
        }
    }

    [[nodiscard]] Ray RayFromEyeTo(const MonitorLayout& layout, const Vector3 target)
    {
        const Vector3 eye = layout.camera.eyePosition;
        return {eye, {target.x - eye.x, target.y - eye.y, target.z - eye.z}};
    }

    [[nodiscard]] Vector3 PointOnMonitor(
        const Monitor& monitor, const float localX, const float localY = 0.0F)
    {
        constexpr float Pi = 3.14159265358979323846F;
        const float yaw = monitor.yawDegrees * Pi / 180.0F;
        return {monitor.position.x + localX * std::cos(yaw), monitor.position.y + localY,
                monitor.position.z - localX * std::sin(yaw)};
    }

    void RequireHit(const MonitorLayout& layout, const Monitor& monitor, const MonitorId id,
                    const float localX, const float expectedLocalX, const float desktopOffset,
                    const char* const description)
    {
        const auto hit = IntersectMonitorLayout(
            RayFromEyeTo(layout, PointOnMonitor(monitor, localX)), layout);
        Require(hit && hit->monitor == id && Near(hit->localPixels.x, expectedLocalX) &&
                    Near(hit->localPixels.y, static_cast<float>(monitor.resolutionHeight) * 0.5F) &&
                    Near(hit->desktopPixels.x, desktopOffset + expectedLocalX) &&
                    Near(hit->desktopPixels.y, hit->localPixels.y),
                description);
    }
}

int main()
{
    const MonitorLayout rig = CreateSymmetricalTripleMonitorLayout(
        0.620F, 0.349F, 2560U, 1440U, 0.006F, 50.0F, {0.0F, 0.0F, 0.520F},
        {0.0F, 0.0F, 0.0F});

    Require(Near(rig.centre.position.z, 0.520F) && Near(rig.left.yawDegrees, -50.0F) &&
                Near(rig.right.yawDegrees, 50.0F),
            "supplied rig pose");
    Require(Near(rig.left.position.x, -rig.right.position.x) &&
                Near(rig.left.position.z, rig.right.position.z),
            "layout pose symmetry");

    RequireHit(rig, rig.left, MonitorId::Left, 0.0F, 1280.0F, 0.0F, "left centre");
    RequireHit(rig, rig.centre, MonitorId::Centre, 0.0F, 1280.0F, 2560.0F,
               "centre centre");
    RequireHit(rig, rig.right, MonitorId::Right, 0.0F, 1280.0F, 5120.0F,
               "right centre");
    RequireHit(rig, rig.left, MonitorId::Left, 0.310F, 2560.0F, 0.0F,
               "left inner edge");
    RequireHit(rig, rig.left, MonitorId::Left, -0.310F, 0.0F, 0.0F, "left outer edge");
    RequireHit(rig, rig.right, MonitorId::Right, -0.310F, 0.0F, 5120.0F,
               "right inner edge");
    RequireHit(rig, rig.right, MonitorId::Right, 0.310F, 2560.0F, 5120.0F,
               "right outer edge");

    const auto leftQuarter = IntersectMonitorLayout(
        RayFromEyeTo(rig, PointOnMonitor(rig.left, -0.155F, 0.08725F)), rig);
    const auto rightQuarter = IntersectMonitorLayout(
        RayFromEyeTo(rig, PointOnMonitor(rig.right, 0.155F, 0.08725F)), rig);
    Require(leftQuarter && rightQuarter && Near(leftQuarter->localPixels.x, 640.0F) &&
                Near(rightQuarter->localPixels.x, 1920.0F) &&
                Near(leftQuarter->localPixels.y, rightQuarter->localPixels.y),
            "corresponding side points are symmetric");

    Require(!IntersectMonitorLayout({rig.camera.eyePosition, {0.0F, 1.0F, 0.0F}}, rig),
            "ray misses every monitor");

    // Overlapping closed rectangles deliberately exercise nearest-plane selection.
    MonitorLayout overlap{
        Monitor{2.0F, 2.0F, 100U, 100U, {0.0F, 0.0F, 1.0F}, 0.0F, 0.0F},
        Monitor{2.0F, 2.0F, 100U, 100U, {0.0F, 0.0F, 2.0F}, 0.0F, 0.0F},
        Monitor{}, Camera{{0.0F, 0.0F, 0.0F}}};
    const auto nearest = IntersectMonitorLayout({{}, {0.0F, 0.0F, 1.0F}}, overlap);
    Require(nearest && nearest->monitor == MonitorId::Left && Near(nearest->distance, 1.0F),
            "nearest valid intersection");

    const MonitorLayout other = CreateSymmetricalTripleMonitorLayout(
        0.700F, 0.400F, 1920U, 1080U, 0.010F, 35.0F, {0.25F, 0.10F, 0.800F},
        {0.25F, 0.10F, -0.150F});
    RequireHit(other, other.right, MonitorId::Right, 0.0F, 960.0F, 3840.0F,
               "non-rig dimensions, angle, translation, and eye distance");

    const auto centre = CentreMonitorPixelIntersection(
        RayFromEyeTo(rig, rig.centre.position), rig);
    Require(centre && Near(centre->x, 1280.0F) && Near(centre->y, 720.0F),
            "Version 0.04 centre API remains operational");
    Require(!Intersect({{}, {1.0F, 0.0F, 0.0F}}, DisplayPlane(rig.centre)), "parallel ray");
    Require(!Intersect({{0.0F, 0.0F, 1.0F}, {0.0F, 0.0F, 1.0F}},
                       DisplayPlane(rig.centre)),
            "behind-origin plane");

    std::cout << "All monitor geometry tests passed.\n";
}
