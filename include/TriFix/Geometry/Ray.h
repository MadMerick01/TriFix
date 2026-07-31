#pragma once

#include "TriFix/Geometry/Vector3.h"

namespace TriFix::Geometry
{
    /// Describes a ray by its origin and direction in three-dimensional space.
    class Ray final
    {
    public:
        Vector3 origin{};
        Vector3 direction{};

        constexpr Ray() noexcept = default;
        constexpr Ray(Vector3 originValue, Vector3 directionValue) noexcept
            : origin(originValue), direction(directionValue)
        {
        }
    };
}
