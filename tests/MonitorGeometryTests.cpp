#include "TriFix/Geometry/MonitorGeometry.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>

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

    // Full-frame inverse mapping: centre view is an identity into the middle source third.
    // Wide plane contains even the very oblique rays adjacent to the 50-degree hinges.
    constexpr float canvasWidth = 12.0F;
    constexpr float canvasHeight = 4.6F;
    const auto centreSample = InverseMapToReference(
        {1280.0F, 720.0F}, rig.centre, rig, 7680U, 1440U, canvasWidth, canvasHeight);
    Require(centreSample.valid && Near(centreSample.sourcePixels.x, 3840.0F) &&
                Near(centreSample.sourcePixels.y, 720.0F),
            "centre inverse map is undistorted");

    const Monitor* monitors[]{&rig.left, &rig.centre, &rig.right};
    const Vector2 probes[]{{0.0F, 0.0F}, {2560.0F, 0.0F}, {0.0F, 1440.0F},
                           {2560.0F, 1440.0F}, {1280.0F, 720.0F}, {1.0F, 360.0F},
                           {2559.0F, 1080.0F}};
    for (const Monitor* monitor : monitors)
        for (const Vector2 probe : probes)
        {
            const auto sample = InverseMapToReference(
                probe, *monitor, rig, 7680U, 1440U, canvasWidth, canvasHeight);
            Require(std::isfinite(sample.monitorPoint.x) && std::isfinite(sample.eyeRay.z) &&
                        std::isfinite(sample.uv.x) && std::isfinite(sample.uv.y),
                    "representative inverse map is finite");
            const Vector3 reconstructed{
                rig.camera.eyePosition.x + sample.eyeRay.x,
                rig.camera.eyePosition.y + sample.eyeRay.y,
                rig.camera.eyePosition.z + sample.eyeRay.z};
            Require(Near(reconstructed.x, sample.monitorPoint.x) &&
                        Near(reconstructed.y, sample.monitorPoint.y) &&
                        Near(reconstructed.z, sample.monitorPoint.z),
                    "eye-ray round trip agrees with physical point");
        }

    const auto leftMirror = InverseMapToReference(
        {411.0F, 337.0F}, rig.left, rig, 7680U, 1440U, canvasWidth, canvasHeight);
    const auto rightMirror = InverseMapToReference(
        {2149.0F, 337.0F}, rig.right, rig, 7680U, 1440U, canvasWidth, canvasHeight);
    Require(Near(leftMirror.uv.x, 1.0F - rightMirror.uv.x) &&
                Near(leftMirror.uv.y, rightMirror.uv.y) &&
                leftMirror.valid == rightMirror.valid,
            "inverse mappings have exact left-right mirror correspondence");
    const auto invalid = InverseMapToReference(
        {1280.0F, -10000.0F}, rig.centre, rig, 7680U, 1440U, canvasWidth, canvasHeight);
    Require(!invalid.valid, "out-of-range source sample is classified without clamping");
    const auto otherSample = InverseMapToReference(
        {1919.0F, 1.0F}, other.right, other, 5760U, 1080U, 2.1F, 0.4F);
    Require(std::isfinite(otherSample.uv.x) && std::isfinite(otherSample.uv.y),
            "inverse mapping supports a second translated layout");

    for (std::size_t index = 0; index < 3; ++index)
    {
        const auto sample = InverseMapToReference(
            {1280.0F, 720.0F}, *monitors[index], rig, 7680U, 1440U,
            canvasWidth, canvasHeight);
        std::cout << "monitor " << index << ": local=(1280,720) physical=("
                  << sample.monitorPoint.x << ',' << sample.monitorPoint.y << ','
                  << sample.monitorPoint.z << ") ray=(" << sample.eyeRay.x << ','
                  << sample.eyeRay.y << ',' << sample.eyeRay.z << ") uv=("
                  << sample.uv.x << ',' << sample.uv.y << ") source=("
                  << sample.sourcePixels.x << ',' << sample.sourcePixels.y << ") valid="
                  << sample.valid << '\n';
    }

    std::cout << "All monitor geometry tests passed.\n";
}
