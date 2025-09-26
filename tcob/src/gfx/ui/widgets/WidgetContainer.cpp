// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/WidgetContainer.hpp"

#include <memory>

#include "tcob/core/Point.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui {

widget_container::widget_container(init const& wi)
    : widget {wi}
{
}

void widget_container::update(milliseconds deltaTime)
{
    widget::update(deltaTime);

    for (auto const& w : widgets()) {
        w->update(deltaTime);
    }
}

void widget_container::draw(widget_painter& painter)
{
    if (!is_visible() || Bounds->width() <= 0 || Bounds->height() <= 0) { return; }

    if (needs_redraw()) {
        painter.begin(Alpha);
        on_draw(painter);
        on_draw_children(painter);
        painter.end();
    }

    set_redraw(false);
}

auto widget_container::find_child_at(point_i pos) -> widget*
{
    for (auto const& widget : widgets()) { // ZORDER
        if (!widget->hit_test(pos)) { continue; }
        if (auto container {std::dynamic_pointer_cast<widget_container>(widget)}) {
            if (auto* retValue {container->find_child_at(pos)}) {
                return retValue;
            }
        }
        return widget.get();
    }
    return nullptr;
}

auto widget_container::find_child_by_name(string const& name) -> widget*
{
    for (auto const& widget : widgets()) {
        if (widget->name() == name) { return widget.get(); }
        if (auto container {std::dynamic_pointer_cast<widget_container>(widget)}) {
            if (auto* retValue {container->find_child_by_name(name)}) {
                return retValue;
            }
        }
    }

    return nullptr;
}

void widget_container::on_prepare_redraw()
{
    widget::on_prepare_redraw();

    for (auto const& w : widgets()) {
        w->prepare_redraw();
    }
}

void widget_container::on_styles_changed()
{
    widget::on_styles_changed();
    for (auto const& w : widgets()) {
        w->on_styles_changed();
    }
}

void widget_container::set_redraw(bool val)
{
    widget::set_redraw(val);
    for (auto const& w : widgets()) {
        w->set_redraw(val);
    }
}

}
