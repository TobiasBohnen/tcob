// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <memory>

#include "tcob/core/Signal.hpp"
#include "tcob/core/tweening/Tween.hpp"

namespace tcob::ui {
////////////////////////////////////////////////////////////

class TCOB_API text_edit {
public:
    signal<> TextChanged;
    signal<> Submitted;
    signal<> Invalidated;

    void update(milliseconds deltaTime);

    void start_blinking(milliseconds blinkRate);
    void stop_blinking();
    void pause_blinking();
    void resume_blinking();

    void insert_char(utf8_string const& ch);
    void delete_backward();
    void delete_forward();

    void move_caret_left();
    void move_caret_right();
    void move_caret_home();
    void move_caret_end();
    auto caret_pos() const -> isize;
    auto caret_visible() const -> bool;

    auto get_text() const -> utf8_string const&;
    void set_text(utf8_string const& t);
    auto text_length() const -> isize;

private:
    utf8_string                              _text;
    isize                                    _caretPos {0};
    isize                                    _textLength {0};
    bool                                     _caretVisible {false};
    std::unique_ptr<square_wave_tween<bool>> _caretTween;
};

}
