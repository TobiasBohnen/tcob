// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/Tooltip.hpp"

#include "tcob/core/Point.hpp"
#include "tcob/core/tweening/Tween.hpp"
#include "tcob/gfx/ui/Layout.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/widgets/Panel.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui {

tooltip::tooltip(init const& wi)
    : panel {wi}
{
    Class("tooltip");
}

void tooltip::on_update(milliseconds deltaTime)
{
    if (_fadeInTween) {
        get_layout()->apply(content_bounds().Size);
        _fadeInTween->update(deltaTime);
    }
    panel::on_update(deltaTime);
}

void tooltip::on_tooltip(widget* top)
{
    _fadeInTween = make_unique_tween<linear_tween<f32>>(FadeIn, 0.0f, 1.0f);
    _fadeInTween->Value.Changed.connect([this](auto val) { Alpha = val; });
    _fadeInTween->start();

    Popup({.Sender = this, .Widget = top});
}

////////////////////////////////////////////////////////////

toast::toast(init const& wi)
    : panel {wi}
{
    Class("toast");
}

auto toast::done() const -> bool { return _done; }

void toast::on_update(milliseconds deltaTime)
{
    if (done()) { return; }
    if (!_fadeTween) { on_toast(); }

    _fadeTween->update(deltaTime);

    panel::on_update(deltaTime);
}

void toast::on_toast()
{
    auto const total {FadeIn + Duration + FadeOut};
    auto const restPos {Bounds->Position};
    auto const size {Bounds->Size};

    point_f offset {};
    if (_corner == corner::BottomLeft || _corner == corner::BottomRight) {
        offset.Y = size.Height;
    } else {
        offset.Y = -size.Height;
    }
    if (_corner == corner::TopRight || _corner == corner::BottomRight) {
        offset.X = size.Width;
    } else {
        offset.X = -size.Width;
    }

    _fadeTween = make_unique_tween<linear_tween<f32>>(total, 0.0f, 1.0f);
    _fadeTween->Value.Changed.connect([total, restPos, offset, size, this](auto val) {
        auto const elapsed {val * total};
        if (elapsed < FadeIn) {
            Alpha  = val;
            Bounds = {restPos + offset * (1.0f - val * total / FadeIn), size};
        } else if (elapsed < FadeIn + Duration) {
            Alpha  = 1.0f;
            Bounds = {restPos, size};
        } else {
            Alpha  = 1.0f - static_cast<f32>((elapsed - FadeIn - Duration) / FadeOut);
            Bounds = {restPos, size};
        }
    });
    _fadeTween->Finished.connect([this] {
        _done = true;
    });
    _fadeTween->start();
}

}
