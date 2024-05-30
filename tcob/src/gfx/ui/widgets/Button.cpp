// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/Button.hpp"

#include "tcob/gfx/ui/WidgetPainter.hpp"

namespace tcob::gfx::ui {

button::button(init const& wi)
    : widget {wi}
{
    Label.Changed.connect([&](auto const&) {
        // TODO: translation hook
        force_redraw(get_name() + ": Label changed");
    });
    Icon.Changed.connect([&](auto const&) { force_redraw(get_name() + ": Icon changed"); });

    Class("button");
}

void button::on_paint(widget_painter& painter)
{
    if (auto const* style {get_style<button::style>()}) {
        rect_f rect {Bounds()};

        // background
        painter.draw_background_and_border(*style, rect, false);

        scissor_guard const guard {painter, this};

        bool const drawText {!Label->empty() && (style->Display == display_mode::OnlyText || style->Display == display_mode::TextAndIcon)};
        bool const drawIcon {Icon->Texture && (style->Display == display_mode::OnlyIcon || style->Display == display_mode::TextAndIcon)};

        if (drawText) { // text
            rect_f textRect {rect};
            if (drawIcon) {
                textRect = {rect.get_center().X, rect.Y, rect.Width / 2, rect.Height};
                rect.Width /= 2;
            }

            if (style->Text.Font) {
                painter.draw_text(style->Text, textRect, Label());
            }
        }
        if (drawIcon) { // icon
            auto const iconSize {Icon->Texture->get_size()};
            f32 const  factor {iconSize.Height / static_cast<f32>(iconSize.Width)};
            f32 const  width {rect.Height * factor};
            rect = {{rect.get_center().X - width / 2, rect.Y}, {width, rect.Height}};
            auto& canvas {painter.get_canvas()};
            canvas.set_fill_style(style->Text.Color);
            canvas.draw_image(Icon->Texture.get_obj(), Icon->Region, rect);
        }
    }
}

void button::on_update(milliseconds /*deltaTime*/)
{
}

auto button::get_attributes() const -> widget_attributes
{
    widget_attributes retValue {{"label", Label()}};
    auto const        base {widget::get_attributes()};
    retValue.insert(base.begin(), base.end());
    return retValue;
}

} // namespace ui
