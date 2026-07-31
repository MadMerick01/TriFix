#pragma once

namespace TriFix::Geometry
{
    /// Represents a two-dimensional vector for geometry calculations.
    class Vector2 final
    {
    public:
        float x{};
        float y{};

        constexpr Vector2() noexcept = default;
        constexpr Vector2(float xValue, float yValue) noexcept
            : x(xValue), y(yValue)
        {
        }
    };
}
