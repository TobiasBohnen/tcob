// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/WidgetContainer.hpp"

#include "tcob/gfx/ui/Form.hpp"

namespace tcob::gfx::ui {

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

auto widget_container::find_child_at(point_f pos) -> std::shared_ptr<widget>
{
    for (auto const& widget : widgets_by_zorder(true)) {
        if (!widget->hit_test(pos)) { continue; }
        if (auto container {std::dynamic_pointer_cast<widget_container>(widget)}) {
            if (auto retValue {container->find_child_at(pos)}) {
                return retValue;
            }
        }
        return widget;
    }
    return nullptr;
}

auto widget_container::find_child_by_name(string const& name) -> std::shared_ptr<widget>
{
    for (auto const& widget : widgets()) {
        if (widget->name() == name) { return widget; }
        if (auto container {std::dynamic_pointer_cast<widget_container>(widget)}) {
            if (auto retValue {container->find_child_by_name(name)}) {
                return retValue;
            }
        }
    }

    return nullptr;
}

auto widget_container::widgets_by_zorder(bool reverse) const -> std::vector<std::shared_ptr<widget>>
{
    return detail::widgets_by_zorder(widgets(), reverse);
}

void widget_container::prepare_redraw()
{
    widget::prepare_redraw();

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

auto widget_container::paint_offset() const -> point_f
{
    point_f retValue {point_f::Zero};

    if (auto const* wparent {parent()}) {
        retValue += wparent->global_content_bounds().Position;
        retValue -= wparent->scroll_offset();
        if (auto const* form {parent_form()}) {
            retValue -= form->Bounds->Position;
        }
    }

    retValue -= scroll_offset();

    return retValue;
}

} // namespace ui
