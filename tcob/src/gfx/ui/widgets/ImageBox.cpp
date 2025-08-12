// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/ImageBox.hpp"

#include "tcob/core/Common.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/Transform.hpp"
#include "tcob/gfx/animation/Animation.hpp"
#include "tcob/gfx/ui/Form.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"
#include "tcob/gfx/ui/component/Icon.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui {

void image_box::style::Transition(style& target, style const& from, style const& to, f64 step)
{
    widget_style::Transition(target, from, to, step);

    target.DragAlpha = static_cast<f32>(from.DragAlpha + ((to.DragAlpha - from.DragAlpha) * step));
}

image_box::image_box(init const& wi)
    : widget {wi}
{
    Image.Changed.connect([this](auto const&) { queue_redraw(); });

    Fit.Changed.connect([this](auto const&) { queue_redraw(); });
    Fit(fit_mode::Contain);

    Class("image_box");

    _animationTween.Changed.connect([this](auto const& val) { Image.mutate([&val](icon& icon) { icon.TextureRegion = val; }); });
}

void image_box::start_animation(gfx::frame_animation const& ani, playback_mode mode)
{
    _animationTween.start(ani, mode);
}

void image_box::stop_animation()
{
    _animationTween.stop();
}

void image_box::on_draw(widget_painter& painter)
{
    rect_f const rect {draw_background(_style, painter)};

    scissor_guard const guard {painter, this};

    // image
    if (!Image->Texture) { return; }

    auto const& tex {Image->Texture};
    auto const  texSize {size_f {tex->info().Size}};

    rect_f targetRect {};

    switch (Fit) {
    case fit_mode::None:
        targetRect = {rect.Position, texSize};
        break;
    case fit_mode::Contain: {
        targetRect = {rect.Position, rect.Size.as_fitted(texSize)};
    } break;
    case fit_mode::Fill:
        targetRect = rect;
        break;
    case fit_mode::FitWidth: {
        f32 const factor {texSize.Width / texSize.Height};
        targetRect = {rect.Position, {rect.width(), rect.width() * factor}};
    } break;
    case fit_mode::FitHeight: {
        f32 const factor {texSize.Height / texSize.Width};
        targetRect = {rect.Position, {rect.height() * factor, rect.height()}};
    } break;
    }

    switch (_style.Alignment.Horizontal) {
    case gfx::horizontal_alignment::Left:     break;
    case gfx::horizontal_alignment::Right:    targetRect.Position.X += rect.width() - targetRect.width(); break;
    case gfx::horizontal_alignment::Centered: targetRect.Position.X += (rect.width() - targetRect.width()) / 2; break;
    }
    switch (_style.Alignment.Vertical) {
    case gfx::vertical_alignment::Top:    break;
    case gfx::vertical_alignment::Bottom: targetRect.Position.Y += rect.height() - targetRect.height(); break;
    case gfx::vertical_alignment::Middle: targetRect.Position.Y += (rect.height() - targetRect.height()) / 2; break;
    }

    auto& canvas {painter.canvas()};
    canvas.set_fill_style(colors::White);
    canvas.draw_image(tex.ptr(), Image->TextureRegion, targetRect);
    _imageRectCache = targetRect;

    if (_isDragging) {
        auto const    dragOffset {drag_offset()};
        point_f const globalOffset {global_position() - Bounds->Position - form().Bounds->Position};
        painter.add_overlay([this, alpha = _style.DragAlpha, globalOffset, dragOffset](widget_painter& painter) {
            auto& canvas {painter.canvas()};

            gfx::transform xform;
            xform.translate(globalOffset);
            painter.begin(Alpha, xform);

            canvas.set_global_alpha(alpha);
            canvas.draw_image(Image->Texture.ptr(), Image->TextureRegion,
                              {dragOffset, _imageRectCache.Size});
        });
    }
}

void image_box::on_update(milliseconds deltaTime)
{
    _animationTween.update(deltaTime);
}

void image_box::on_mouse_drag(input::mouse::motion_event const& ev)
{
    if (Draggable) {
        _isDragging = true;
        queue_redraw();
        ev.Handled = true;
    }
}

void image_box::on_mouse_button_up(input::mouse::button_event const& ev)
{
    if (_isDragging) {
        Dropped({.Sender = this, .Target = form().find_widget_at(ev.Position).get(), .Position = ev.Position});
        queue_redraw();
        ev.Handled = true;
    }

    _dragOffset = point_f::Zero;
    _isDragging = false;
}

void image_box::on_mouse_button_down(input::mouse::button_event const& ev)
{
    _isDragging = false;

    if (Draggable && ev.Button == controls().PrimaryMouseButton) {
        _dragOffset = global_to_parent(*this, ev.Position) - drag_origin();
        _isDragging = true;
        ev.Handled  = true;
    }
}

auto image_box::attributes() const -> widget_attributes
{
    auto retValue {widget::attributes()};

    retValue["fit"] = *Fit;

    return retValue;
}

auto image_box::drag_origin() const -> point_f
{
    return _imageRectCache.top_left();
}

auto image_box::drag_offset() const -> point_f
{
    return global_to_parent(*this, locate_service<input::system>().mouse()->get_position()) - _dragOffset;
}

} // namespace ui
