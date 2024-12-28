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

class TCOB_API progress_bar : public widget {
public:
    class TCOB_API style : public background_style {
    public:
        element::bar Bar;
    };

    explicit progress_bar(init const& wi);

    prop_val<i32> Min;
    prop_val<i32> Max;
    prop_val<i32> Value;

protected:
    void on_paint(widget_painter& painter) override;

    void on_update(milliseconds deltaTime) override;

    void virtual on_value_changed(i32 newVal);

    auto attributes() const -> widget_attributes override;

private:
    widget_tweener _tween;
};
}
