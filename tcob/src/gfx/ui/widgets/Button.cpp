// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/Button.hpp"

#include "tcob/gfx/ui/WidgetPainter.hpp"

namespace tcob::gfx::ui {

button::button(init const& wi)
    : widget {wi}
{
    Label.Changed.connect([this](auto const&) {
        // TODO: translation hook
        force_redraw(this->name() + ": Label changed");
    });
    Icon.Changed.connect([this](auto const&) { force_redraw(this->name() + ": Icon changed"); });

    Class("button");
}

void button::on_paint(widget_painter& painter)
{
    if (auto const* style {current_style<button::style>()}) {
        rect_f rect {Bounds()};

        // background
        painter.draw_background_and_border(*style, rect, false);

        scissor_guard const guard {painter, this};

        bool const drawText {!Label->empty() && (style->Display == display_mode::OnlyText || style->Display == display_mode::TextAndIcon)};
        bool const drawIcon {Icon->Texture && (style->Display == display_mode::OnlyIcon || style->Display == display_mode::TextAndIcon)};

        if (drawText) { // text
            rect_f textRect {rect};
            if (drawIcon) {
                textRect = {rect.center().X, rect.top(), rect.width() / 2, rect.height()};
                rect.Size.Width /= 2;
            }

            if (style->Text.Font) {
                painter.draw_text(style->Text, textRect, Label());
            }
        }
        if (drawIcon) { // icon
            auto const iconSize {Icon->Texture->info().Size};
            f32 const  factor {iconSize.Height / static_cast<f32>(iconSize.Width)};
            f32 const  width {rect.height() * factor};
            rect = {{rect.center().X - (width / 2), rect.top()}, {width, rect.height()}};
            auto& canvas {painter.canvas()};
            canvas.set_fill_style(style->Text.Color);
            canvas.draw_image(Icon->Texture.ptr(), Icon->Region, rect);
        }
    }
}

void button::on_update(milliseconds /*deltaTime*/)
{
}

auto button::attributes() const -> widget_attributes
{
    widget_attributes retValue {{"label", Label()}};
    auto const        base {widget::attributes()};
    retValue.insert(base.begin(), base.end());
    return retValue;
}

} // namespace ui
