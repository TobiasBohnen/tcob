// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/gfx/ui/WidgetTweener.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::gfx::ui {
////////////////////////////////////////////////////////////

class TCOB_API toggle : public widget {
public:
    class TCOB_API style : public background_style {
    public:
        milliseconds  Delay {0};
        element::tick Tick;

        auto operator==(style const& other) const -> bool = default;
    };

    explicit toggle(init const& wi);

    prop<bool> Checked;

protected:
    void on_paint(widget_painter& painter) override;

    void on_update(milliseconds deltaTime) override;

    void virtual on_checked_changed();

    void on_click() override;

    auto attributes() const -> widget_attributes override;
    auto flags() -> widget_flags override;

private:
    widget_tweener _tween;
};
}
