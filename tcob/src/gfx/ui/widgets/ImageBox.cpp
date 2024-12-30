// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/ImageBox.hpp"

#include "tcob/gfx/ui/WidgetPainter.hpp"

namespace tcob::gfx::ui {

image_box::image_box(init const& wi)
    : widget {wi}
{
    Image.Changed.connect([&](auto const&) { force_redraw(this->name() + ": Image changed"); });

    Class("image_box");
}

void image_box::on_paint(widget_painter& painter)
{
    if (auto const* style {current_style<image_box::style>()}) {
        rect_f rect {Bounds()};

        // background
        painter.draw_background_and_border(*style, rect, false);

        scissor_guard const guard {painter, this};

        // image
        if (Image->Texture) {
            auto const& tex {Image->Texture};
            auto const  texSize {size_f {tex->info().Size}};

            rect_f targetRect {};

            switch (style->Fit) {
            case fit_mode::None:
                targetRect = {rect.Position, texSize};
                break;
            case fit_mode::Contain: {
                size_f const imgSize {texSize};
                f32 const    aspectRatioToShrink {imgSize.Width / imgSize.Height};
                f32 const    aspectRatioTarget {rect.width() / rect.height()};
                f32 const    factor {(aspectRatioToShrink > aspectRatioTarget) ? (rect.width() / imgSize.Width) : (rect.height() / imgSize.Height)};
                targetRect = {rect.Position, {imgSize.Width * factor, imgSize.Height * factor}};
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

            switch (style->Alignment.Horizontal) {
            case horizontal_alignment::Left:
                break;
            case horizontal_alignment::Right:
                targetRect.Position.X += rect.width() - targetRect.width();
                break;
            case horizontal_alignment::Centered:
                targetRect.Position.X += (rect.width() - targetRect.width()) / 2;
                break;
            }
            switch (style->Alignment.Vertical) {
            case vertical_alignment::Top:
                break;
            case vertical_alignment::Bottom:
                targetRect.Position.Y += rect.height() - targetRect.height();
                break;
            case vertical_alignment::Middle:
                targetRect.Position.Y += (rect.height() - targetRect.height()) / 2;
                break;
            }

            auto& canvas {painter.canvas()};
            canvas.set_fill_style(colors::White);
            canvas.draw_image(tex.ptr(), Image->Region, targetRect);
        }
    }
}

void image_box::on_update(milliseconds /*deltaTime*/)
{
}

} // namespace ui
