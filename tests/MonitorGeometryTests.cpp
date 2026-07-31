#include "TriFix/Geometry/MonitorGeometry.h"

#include <cmath>
#include <cstdlib>
#include <iostream>

using namespace TriFix::Geometry;

namespace
{
    constexpr float Epsilon = 1.0e-4F;

    [[nodiscard]] bool Near(float actual, float expected) noexcept
    {
        return std::abs(actual - expected) < Epsilon;
    }

    void Require(bool condition, const char* description)
    {
        if (!condition)
        {
            std::cerr << "FAILED: " << description << '\n';
            std::exit(EXIT_FAILURE);
        }
    }

    Ray RayTo(Vector3 target)
    {
        return {{0.0F, 0.0F, 0.0F}, target};
    }
}

int main()
{
    const Monitor centre{2.0F, 1.0F, 2000U, 1000U, {0.0F, 0.0F, 2.0F}, 0.0F, 0.02F};
    const MonitorLayout layout{{}, centre, {}};

    const auto middle = CentreMonitorPixelIntersection(RayTo({0.0F, 0.0F, 2.0F}), layout);
    Require(middle && Near(middle->x, 1000.0F) && Near(middle->y, 500.0F), "centre hit and pixels");

    struct Corner { Vector3 world; Vector2 pixels; };
    const Corner corners[]{
        {{-1.0F,  0.5F, 2.0F}, {0.0F, 0.0F}},
        {{ 1.0F,  0.5F, 2.0F}, {2000.0F, 0.0F}},
        {{-1.0F, -0.5F, 2.0F}, {0.0F, 1000.0F}},
        {{ 1.0F, -0.5F, 2.0F}, {2000.0F, 1000.0F}}};
    for (const Corner& corner : corners)
    {
        const auto pixel = CentreMonitorPixelIntersection(RayTo(corner.world), layout);
        Require(pixel && Near(pixel->x, corner.pixels.x) && Near(pixel->y, corner.pixels.y), "corner hit and edge pixels");
    }

    Require(!Intersect({{}, {1.0F, 0.0F, 0.0F}}, DisplayPlane(centre)), "parallel ray");
    Require(!Intersect({{0.0F, 0.0F, 3.0F}, {0.0F, 0.0F, 1.0F}}, DisplayPlane(centre)), "hit behind origin");
    Require(!CentreMonitorPixelIntersection(RayTo({1.1F, 0.0F, 2.0F}), layout), "outside display");

    const Monitor rotated{2.0F, 1.0F, 2000U, 1000U, {3.0F, 1.0F, 4.0F}, 30.0F, 0.0F};
    const MonitorLayout rotatedLayout{{}, rotated, {}};
    const auto rotatedMiddle = CentreMonitorPixelIntersection(RayTo(rotated.position), rotatedLayout);
    Require(rotatedMiddle && Near(rotatedMiddle->x, 1000.0F) && Near(rotatedMiddle->y, 500.0F), "translated and yawed centre");

    std::cout << "All monitor geometry tests passed.\n";
}
