// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::gfx::ui {
////////////////////////////////////////////////////////////

class TCOB_API checkbox : public widget {
public:
    class TCOB_API style : public widget_style {
    public:
        element::tick Tick;

        void static Transition(style& target, style const& left, style const& right, f64 step);
    };

    explicit checkbox(init const& wi);

    prop<bool> Checked;

protected:
    void on_paint(widget_painter& painter) override;

    void on_update(milliseconds deltaTime) override;

    void virtual on_checked_changed();

    void on_click() override;

    auto attributes() const -> widget_attributes override;
    auto flags() -> widget_flags override;

private:
    checkbox::style _style;
};
}
