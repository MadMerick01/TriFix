#pragma once

#include <cstdint>

#include "TriFix/Geometry/Vector3.h"

namespace TriFix::Geometry
{
    /// Stores the physical pose, dimensions, resolution, and bezel of a monitor.
    class Monitor final
    {
    public:
        float physicalWidthMetres{};
        float physicalHeightMetres{};
        std::uint32_t resolutionWidth{};
        std::uint32_t resolutionHeight{};
        Vector3 position{};
        float yawDegrees{};
        float bezelWidthMetres{};

        constexpr Monitor() noexcept = default;
        constexpr Monitor(
            float physicalWidthMetresValue,
            float physicalHeightMetresValue,
            std::uint32_t resolutionWidthValue,
            std::uint32_t resolutionHeightValue,
            Vector3 positionValue,
            float yawDegreesValue,
            float bezelWidthMetresValue) noexcept
            : physicalWidthMetres(physicalWidthMetresValue),
              physicalHeightMetres(physicalHeightMetresValue),
              resolutionWidth(resolutionWidthValue),
              resolutionHeight(resolutionHeightValue),
              position(positionValue),
              yawDegrees(yawDegreesValue),
              bezelWidthMetres(bezelWidthMetresValue)
        {
        }
    };
}
