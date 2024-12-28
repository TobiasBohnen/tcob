// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::gfx::ui {
////////////////////////////////////////////////////////////

class TCOB_API button : public widget {
public:
    enum class display_mode : u8 {
        TextAndIcon,
        OnlyIcon,
        OnlyText
    };

    class TCOB_API style : public background_style {
    public:
        display_mode  Display {display_mode::TextAndIcon};
        element::text Text;
    };

    explicit button(init const& wi);

    prop<utf8_string> Label;
    prop<image_def>   Icon;

protected:
    void on_paint(widget_painter& painter) override;

    void on_update(milliseconds deltaTime) override;

    auto attributes() const -> widget_attributes override;
};
}
