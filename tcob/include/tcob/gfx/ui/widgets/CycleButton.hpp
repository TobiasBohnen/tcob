// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <vector>

#include "tcob/core/Property.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::gfx::ui {
////////////////////////////////////////////////////////////

class TCOB_API cycle_button : public widget {
public:
    class TCOB_API style : public widget_style {
    public:
        element::text Text;

        void static Transition(style& target, style const& left, style const& right, f64 step);
    };

    explicit cycle_button(init const& wi);

    prop_val<isize> SelectedItemIndex;

    void add_item(utf8_string const& item);
    void clear_items();

    auto select_item(utf8_string const& item) -> bool;

    auto get_item_at(isize index) const -> utf8_string const&;
    auto selected_item() const -> utf8_string const&;
    auto item_count() const -> isize;

protected:
    void on_paint(widget_painter& painter) override;

    void on_update(milliseconds deltaTime) override;

    void on_click() override;

    auto attributes() const -> widget_attributes override;

private:
    std::vector<utf8_string> _items;

    cycle_button::style _style;
};
}
