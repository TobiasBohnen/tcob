// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/core/Signal.hpp"
#include "tcob/tcob_config.hpp"

#include "tcob/core/Property.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/StyleElements.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/charting/Charting.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui::charts {
////////////////////////////////////////////////////////////

class TCOB_API legend : public widget {
public:
    class TCOB_API style : public widget_style {
    public:
        text_element Text;

        static void Transition(style& target, style const& from, style const& to, f64 step);
    };

    explicit legend(init const& wi);

    prop<chart_base*> For;

protected:
    void on_draw(widget_painter& painter) override;

    void on_update(milliseconds deltaTime) override;

private:
    legend::style _style;

    scoped_connection _con;
};
}
