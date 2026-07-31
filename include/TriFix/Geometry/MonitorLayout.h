#pragma once

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

        constexpr MonitorLayout() noexcept = default;
        constexpr MonitorLayout(Monitor leftValue, Monitor centreValue, Monitor rightValue) noexcept
            : left(leftValue), centre(centreValue), right(rightValue)
        {
        }
    };
}
