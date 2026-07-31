#pragma once

namespace TriFix::Geometry
{
    /// Represents a three-dimensional vector for positions and directions.
    class Vector3 final
    {
    public:
        float x{};
        float y{};
        float z{};

        constexpr Vector3() noexcept = default;
        constexpr Vector3(float xValue, float yValue, float zValue) noexcept
            : x(xValue), y(yValue), z(zValue)
        {
        }
    };
}
