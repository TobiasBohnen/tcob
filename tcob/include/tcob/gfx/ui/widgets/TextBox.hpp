// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <memory>
#include <utility>

#include "tcob/core/Point.hpp"
#include "tcob/core/Property.hpp"
#include "tcob/core/Signal.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/TextFormatter.hpp"
#include "tcob/gfx/animation/Tween.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui {
////////////////////////////////////////////////////////////

class TCOB_API text_box : public widget {
public:
    class TCOB_API style : public widget_style {
    public:
        text_element  Text;
        caret_element Caret;

        void static Transition(style& target, style const& left, style const& right, f64 step);
    };

    explicit text_box(init const& wi);

    signal<text_event>       BeforeTextInserted;
    signal<text_event const> Submit;

    prop<utf8_string> Text;
    prop<isize>       MaxLength;
    prop<bool>        Selectable;

    auto selected_text() const -> utf8_string;

protected:
    void on_paint(widget_painter& painter) override;

    void on_update(milliseconds deltaTime) override;

    void on_key_down(input::keyboard::event const& ev) override;
    void on_key_up(input::keyboard::event const& ev) override;

    void on_text_input(input::keyboard::text_input_event const& ev) override;
    void on_text_editing(input::keyboard::text_editing_event const& ev) override;

    void on_mouse_drag(input::mouse::motion_event const& ev) override;
    void on_mouse_down(input::mouse::button_event const& ev) override;
    void on_mouse_up(input::mouse::button_event const& ev) override;

    void on_focus_gained() override;
    void on_focus_lost() override;

    auto attributes() const -> widget_attributes override;

    void on_styles_changed() override;

private:
    auto remove_selected_text() -> bool;
    void insert_text(utf8_string const& newText);

    void select_text(isize first, isize last);
    auto is_text_selected() const -> bool;
    auto calc_caret_pos(point_f mp) const -> isize;

    std::unique_ptr<gfx::square_wave_tween<bool>> _caretTween;
    isize                                         _caretPos {0};
    isize                                         _dragCaretPos {INVALID_INDEX};
    bool                                          _caretVisible {false};

    gfx::text_formatter::result _formatResult;
    isize                       _textLength {0};
    bool                        _textDirty {false};
    std::pair<isize, isize>     _selectedText {INVALID_INDEX, INVALID_INDEX};

    text_box::style _style;
};
}
