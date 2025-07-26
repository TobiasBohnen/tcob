// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/Popup.hpp"

#include "tcob/core/easing/Tween.hpp"
#include "tcob/gfx/ui/Layout.hpp"
#include "tcob/gfx/ui/widgets/Panel.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui {

popup::popup(init const& wi)
    : panel {wi}
{
    Class("popup");
}

void popup::on_update(milliseconds deltaTime)
{
    if (_fadeInTween) {
        get_layout()->apply(content_bounds().Size);
        _fadeInTween->update(deltaTime);
    }
    panel::on_update(deltaTime);
}

void popup::on_popup(widget* top)
{
    _fadeInTween = make_unique_tween<linear_tween<f32>>(FadeIn, 0.0f, 1.0f);
    _fadeInTween->Value.Changed.connect([this](auto val) { Alpha = val; });
    _fadeInTween->start();

    Popup({.Sender = this, .Widget = top});
}

} // namespace ui
