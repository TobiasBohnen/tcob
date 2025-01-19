// Copyright (c) 2024 Tobias Bohnen
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

        auto operator==(style const& other) const -> bool = default;
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

    void on_mouse_down(input::mouse::button_event const& ev) override;

    void on_update(milliseconds deltaTime) override;

    auto attributes() const -> widget_attributes override;

private:
    std::vector<utf8_string> _items;
};
}
