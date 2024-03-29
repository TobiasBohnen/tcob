// Copyright (c) 2023 Tobias Bohnen
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
    };

    explicit toggle(init const& wi);

    prop<bool> Enabled;

protected:
    void on_paint(widget_painter& painter) override;

    void on_update(milliseconds deltaTime) override;

    void virtual on_enabled_changed();

    void on_click() override;

    auto get_properties() const -> widget_attributes override;
    auto get_flags() -> flags override;

private:
    widget_tweener _tween;
};
}
