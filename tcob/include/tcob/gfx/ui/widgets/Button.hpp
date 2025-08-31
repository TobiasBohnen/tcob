// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/core/Property.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/StyleElements.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/component/Icon.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui {
////////////////////////////////////////////////////////////

class TCOB_API button : public widget {
public:
    class TCOB_API style : public widget_style {
    public:
        text_element    Text;
        icon_text_order IconTextOrder {icon_text_order::IconBeforeText};

        static void Transition(style& target, style const& from, style const& to, f64 step);
    };

    explicit button(init const& wi);

    prop<utf8_string> Label;
    prop<icon>        Icon;

protected:
    void on_draw(widget_painter& painter) override;

    void on_update(milliseconds deltaTime) override;

    auto attributes() const -> widget_attributes override;

private:
    button::style _style;
};
}
