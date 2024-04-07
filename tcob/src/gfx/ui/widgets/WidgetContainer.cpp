// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/WidgetContainer.hpp"

#include <ranges>

#include "tcob/gfx/ui/Form.hpp"

namespace tcob::gfx::ui {

widget_container::widget_container(init const& wi)
    : widget {wi}
{
}

void widget_container::update(milliseconds deltaTime)
{
    widget::update(deltaTime);

    for (auto const& w : get_widgets()) {
        w->update(deltaTime);
    }
}

auto widget_container::find_child_at(point_f pos) -> std::shared_ptr<widget>
{
    for (auto const& w : get_widgets_by_zorder() | std::views::reverse) {
        if (w->hit_test(pos)) {
            if (auto retValue {w->find_child_at(pos)}) {
                return retValue;
            }
            return w;
        }
    }
    return nullptr;
}

auto widget_container::find_child_by_name(string const& name) -> std::shared_ptr<widget>
{
    for (auto const& w : get_widgets()) {
        if (w->get_name() == name) {
            return w;
        }
        if (auto retValue {w->find_child_by_name(name)}) {
            return retValue;
        }
    }

    return nullptr;
}

auto widget_container::get_widgets_by_zorder() const -> std::vector<std::shared_ptr<widget>>
{
    auto retValue {get_widgets()};
    std::sort(retValue.begin(), retValue.end(), [](auto const& a, auto const& b) {
        return a->ZOrder() < b->ZOrder();
    });
    return retValue;
}

void widget_container::collect_widgets(std::vector<widget*>& vec)
{
    vec.push_back(this);
    for (auto const& w : get_widgets()) {
        w->collect_widgets(vec);
    }
}

void widget_container::update_style()
{
    widget::update_style();

    for (auto const& w : get_widgets()) {
        w->update_style();
    }
}

void widget_container::on_styles_changed()
{
    for (auto const& w : get_widgets()) {
        w->on_styles_changed();
    }
}

auto widget_container::get_paint_offset() const -> point_f
{
    point_f retValue {point_f::Zero};

    if (auto const* parent {get_parent()}) {
        retValue += parent->get_global_content_bounds().get_position();
        retValue -= parent->get_scroll_offset();
        if (auto const* form {get_form()}) {
            retValue -= form->Bounds->get_position();
        }
    }

    retValue -= get_scroll_offset();

    return retValue;
}

} // namespace ui
