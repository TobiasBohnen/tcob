// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/ImageBox.hpp"

#include "tcob/core/Rect.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui {

image_box::image_box(init const& wi)
    : widget {wi}
{
    Image.Changed.connect([this](auto const&) { request_redraw(this->name() + ": Image changed"); });

    Fit.Changed.connect([this](auto const&) { request_redraw(this->name() + ": Fit changed"); });
    Fit(fit_mode::Contain);

    Class("image_box");
}

void image_box::on_draw(widget_painter& painter)
{
    rect_f rect {draw_background(_style, painter)};

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
    canvas.draw_image(tex.ptr(), Image->Region, targetRect);
}

void image_box::on_update(milliseconds /* deltaTime */)
{
}

void image_box::on_animation_step(string const& val)
{
    Image.mut_ref().Region = val;
    request_redraw(this->name() + ": Animation Frame changed ");
}

auto image_box::attributes() const -> widget_attributes
{
    auto retValue {widget::attributes()};

    retValue["fit"] = *Fit;

    return retValue;
}

} // namespace ui
