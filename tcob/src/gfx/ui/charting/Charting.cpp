// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/charting/Charting.hpp"

#include <vector>

#include "tcob/core/Color.hpp"
#include "tcob/core/Common.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/UI.hpp"

namespace tcob::ui::charts {

void chart_style::Transition(chart_style& target, chart_style const& from, chart_style const& to, f64 step)
{
    widget_style::Transition(target, from, to, step);

    target.Colors.clear();
    target.Colors.resize(from.Colors.size());
    for (usize i {0}; i < from.Colors.size(); ++i) {
        if (i >= to.Colors.size()) {
            target.Colors[i] = from.Colors[i];
        } else {
            target.Colors[i] = color::Lerp(from.Colors[i], to.Colors[i], step);
        }
    }
}

void grid_chart_style::Transition(grid_chart_style& target, grid_chart_style const& from, grid_chart_style const& to, f64 step)
{
    chart_style::Transition(target, from, to, step);

    target.GridLineWidth = helper::lerp(from.GridLineWidth, to.GridLineWidth, step);
    target.GridColor     = color::Lerp(from.GridColor, to.GridColor, step);
}

}
