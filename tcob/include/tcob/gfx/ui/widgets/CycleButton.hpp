// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::gfx::ui {
////////////////////////////////////////////////////////////

class TCOB_API cycle_button : public widget {
public:
    class TCOB_API style : public background_style {
    public:
        element::text Text;
    };

    explicit cycle_button(init const& wi);

    prop_val<i32> SelectedItemIndex;

    void add_item(utf8_string const& item);
    void clear_items();

    auto get_item_at(i32 index) const -> utf8_string const&;
    auto get_selected_item() const -> utf8_string const&;
    auto get_item_count() const -> isize;

protected:
    void on_paint(widget_painter& painter) override;

    void on_mouse_down(input::mouse::button_event& ev) override;

    void on_update(milliseconds deltaTime) override;

    auto get_properties() const -> widget_attributes override;

private:
    std::vector<utf8_string> _items;
};
}
