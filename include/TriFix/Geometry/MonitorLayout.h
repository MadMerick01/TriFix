#pragma once

#include "TriFix/Geometry/Camera.h"
#include "TriFix/Geometry/Monitor.h"

namespace TriFix::Geometry
{
    /// Groups the left, centre, and right monitors of a triple-monitor layout.
    class MonitorLayout final
    {
    public:
        Monitor left{};
        Monitor centre{};
        Monitor right{};
        Camera camera{};

        constexpr MonitorLayout() noexcept = default;
        constexpr MonitorLayout(Monitor leftValue, Monitor centreValue, Monitor rightValue) noexcept
            : left(leftValue), centre(centreValue), right(rightValue)
        {
        }

        constexpr MonitorLayout(
            Monitor leftValue, Monitor centreValue, Monitor rightValue, Camera cameraValue) noexcept
            : left(leftValue), centre(centreValue), right(rightValue), camera(cameraValue)
        {
        }
    };
}
