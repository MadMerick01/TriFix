#pragma once

#include "TriFix/Geometry/Vector3.h"

namespace TriFix::Geometry
{
    /// Stores the viewer's eye position used by reprojection calculations.
    class Camera final
    {
    public:
        Vector3 eyePosition{};

        constexpr Camera() noexcept = default;
        constexpr explicit Camera(Vector3 eyePositionValue) noexcept
            : eyePosition(eyePositionValue)
        {
        }
    };
}
