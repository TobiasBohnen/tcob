// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <memory>
#include <utility>

#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Signal.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/core/input/Input_Codes.hpp"
#include "tcob/core/tweening/Tween.hpp"
#include "tcob/gfx/TextFormatter.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/component/StyleElements.hpp"

namespace tcob::ui {
////////////////////////////////////////////////////////////

class TCOB_API text_edit {
public:
    signal<> Invalidated;

    void update(milliseconds deltaTime);
    void draw(widget_painter& painter, rect_f const& rect, text_element const& text, caret_element const& caret);

    void start_blinking(milliseconds blinkRate);
    void stop_blinking();

    void insert_text(utf8_string const& ch);
    void delete_backward();
    void delete_forward();

    auto get_text() const -> utf8_string const&;
    void set_text(utf8_string const& t);
    auto text_length() const -> isize;

    auto selected_text() const -> utf8_string;
    void select_text(isize first, isize last);
    auto remove_selected_text() -> bool;

    void key_down(widget const& widget, input::key_mods keyMods, input::key_code keyCode, bool selectable);
    void key_up();
    void mouse_drag(point_f mp);
    void mouse_button_down(point_f mp);
    void mouse_button_up();

private:
    void deselect_text();
    auto is_text_selected() const -> bool;

    void pause_blinking();
    void resume_blinking();

    void move_caret_left();
    void move_caret_right();
    void move_caret_home();
    void move_caret_end();

    auto calc_caret_pos(point_f mp) const -> isize;
    void mark_dirty();

    utf8_string             _text;
    isize                   _textLength {0};
    std::pair<isize, isize> _selectedText {INVALID_INDEX, INVALID_INDEX};

    isize _caretPos {0};
    isize _dragCaretPos {INVALID_INDEX};

    gfx::text_formatter::result _formatResult;
    bool                        _needsFormat {false};

    bool                                     _caretVisible {false};
    std::unique_ptr<square_wave_tween<bool>> _caretTween;
};

}
