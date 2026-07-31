#pragma once

#include "TriFix/Geometry/Vector3.h"

namespace TriFix::Geometry
{
    /// Describes a plane using a normal and its signed distance from the origin.
    class Plane final
    {
    public:
        Vector3 normal{};
        float distance{};

        constexpr Plane() noexcept = default;
        constexpr Plane(Vector3 normalValue, float distanceValue) noexcept
            : normal(normalValue), distance(distanceValue)
        {
        }
    };
}
