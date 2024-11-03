// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/Tooltip.hpp"

#include "tcob/gfx/ui/Layout.hpp"

namespace tcob::gfx::ui {

tooltip::tooltip(init const& wi)
    : panel {wi}
{
    Class("tooltip");
}

void tooltip::force_redraw(string const& /* reason */)
{
    get_layout()->mark_dirty();
}

void tooltip::on_update(milliseconds deltaTime)
{
    panel::on_update(deltaTime);
    if (_fadeInTween) {
        _fadeInTween->update(deltaTime);
    }
}

void tooltip::on_popup(widget* top)
{
    if (auto const* style {get_style<tooltip::style>()}) {
        _fadeInTween = make_unique_tween<linear_tween<f32>>(style->FadeIn, 0.0f, 1.0f);
        _fadeInTween->Value.Changed.connect([&](auto val) {
            Alpha = val;
            force_redraw(get_name() + ": Tooltip fade-in");
        });
        _fadeInTween->start();
    }

    Popup({this, top});
}

} // namespace ui
