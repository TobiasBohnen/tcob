// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/core/Property.hpp"
#include "tcob/core/Signal.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/UIEvents.hpp"
#include "tcob/gfx/ui/component/StyleElements.hpp"
#include "tcob/gfx/ui/component/TextEdit.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui {
////////////////////////////////////////////////////////////

class TCOB_API text_box : public widget {
public:
    class TCOB_API style : public widget_style {
    public:
        text_element  Text;
        caret_element Caret;

        static void Transition(style& target, style const& from, style const& to, f64 step);
    };

    explicit text_box(init const& wi);

    signal<text_event>       BeforeTextInserted;
    signal<text_event const> Submit;

    prop_fn<utf8_string> Text;
    prop<isize>          MaxLength;
    prop<bool>           Selectable;
    prop<bool>           NumericOnly;

    auto selected_text() const -> utf8_string;

protected:
    void on_draw(widget_painter& painter) override;

    void on_update(milliseconds deltaTime) override;

    void on_key_down(input::keyboard::event const& ev) override;
    void on_key_up(input::keyboard::event const& ev) override;

    void on_text_input(input::keyboard::text_input_event const& ev) override;

    void on_mouse_drag(input::mouse::motion_event const& ev) override;
    void on_mouse_button_down(input::mouse::button_event const& ev) override;
    void on_mouse_button_up(input::mouse::button_event const& ev) override;

    void on_focus_gained() override;
    void on_focus_lost() override;

    auto attributes() const -> widget_attributes override;

private:
    void insert_text(utf8_string const& newText);

    text_edit       _edit;
    utf8_string     _lastValidText;
    text_box::style _style;
};
}
