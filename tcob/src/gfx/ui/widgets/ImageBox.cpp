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
    Image.Changed.connect([&](auto const&) { force_redraw(get_name() + ": Image changed"); });

    Class("image_box");
}

void image_box::on_paint(widget_painter& painter)
{
    if (auto const* style {get_style<image_box::style>()}) {
        rect_f rect {Bounds()};

        // background
        painter.draw_background_and_border(*style, rect, false);

        scissor_guard const guard {painter, this};

        // image
        if (Image->Texture) {
            auto const& tex {Image->Texture};
            rect_f      targetRect {};

            switch (style->Fit) {
            case fit_mode::None:
                targetRect = {rect.get_position(), static_cast<size_f>(tex->get_size())};
                break;
            case fit_mode::Contain: {
                size_f const imgSize {static_cast<size_f>(tex->get_size())};
                f32 const    aspectRatioToShrink {imgSize.Width / imgSize.Height};
                f32 const    aspectRatioTarget {rect.Width / rect.Height};
                f32 const    factor {(aspectRatioToShrink > aspectRatioTarget) ? (rect.Width / imgSize.Width) : (rect.Height / imgSize.Height)};
                targetRect = {rect.get_position(), {imgSize.Width * factor, imgSize.Height * factor}};
            } break;
            case fit_mode::Fill:
                targetRect = rect;
                break;
            case fit_mode::FitWidth: {
                f32 const factor {tex->get_size().Width / static_cast<f32>(tex->get_size().Height)};
                targetRect = {rect.get_position(), {rect.Width, rect.Width * factor}};
            } break;
            case fit_mode::FitHeight: {
                f32 const factor {tex->get_size().Height / static_cast<f32>(tex->get_size().Width)};
                targetRect = {rect.get_position(), {rect.Height * factor, rect.Height}};
            } break;
            }

            switch (style->Alignment.Horizontal) {
            case horizontal_alignment::Left:
                break;
            case horizontal_alignment::Right:
                targetRect.X += rect.Width - targetRect.Width;
                break;
            case horizontal_alignment::Centered:
                targetRect.X += (rect.Width - targetRect.Width) / 2;
                break;
            }
            switch (style->Alignment.Vertical) {
            case vertical_alignment::Top:
                break;
            case vertical_alignment::Bottom:
                targetRect.Y += rect.Height - targetRect.Height;
                break;
            case vertical_alignment::Middle:
                targetRect.Y += (rect.Height - targetRect.Height) / 2;
                break;
            }

            auto& canvas {painter.get_canvas()};
            canvas.set_fill_style(colors::White);
            canvas.draw_image(tex.get_obj(), Image->Region, targetRect);
        }
    }
}

void image_box::on_update(milliseconds /*deltaTime*/)
{
}

} // namespace ui
