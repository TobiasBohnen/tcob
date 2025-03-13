// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/Tooltip.hpp"

#include "tcob/gfx/animation/Tween.hpp"
#include "tcob/gfx/ui/Layout.hpp"
#include "tcob/gfx/ui/widgets/Panel.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui {

tooltip::tooltip(init const& wi)
    : panel {wi}
{
    Class("tooltip");
}

void tooltip::force_redraw(string const& /* reason */)
{
}

void tooltip::on_update(milliseconds deltaTime)
{
    if (_fadeInTween) {
        current_layout()->apply();
        _fadeInTween->update(deltaTime);
    }
    panel::on_update(deltaTime);
}

void tooltip::on_popup(widget* top)
{
    _fadeInTween = gfx::make_unique_tween<gfx::linear_tween<f32>>(FadeIn, 0.0f, 1.0f);
    _fadeInTween->Value.Changed.connect([this](auto val) {
        Alpha = val;
        force_redraw(this->name() + ": Tooltip fade-in");
    });
    _fadeInTween->start();

    Popup({this, top});
}

} // namespace ui
