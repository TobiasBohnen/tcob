// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/component/TextEdit.hpp"

#include <algorithm>

#include "tcob/core/Common.hpp"
#include "tcob/core/StringUtils.hpp"
#include "tcob/core/tweening/Tween.hpp"

namespace tcob::ui {

void text_edit::start_blinking(milliseconds blinkRate)
{
    _caretTween = make_unique_tween<square_wave_tween<bool>>(blinkRate, 1.0f, 0.0f);
    _caretTween->Value.Changed.connect([this](bool val) {
        _caretVisible = val;
        Invalidated();
    });
    _caretTween->start(playback_mode::Looped);
}

void text_edit::stop_blinking()
{
    _caretVisible = false;
    _caretTween   = nullptr;
    Invalidated();
}

void text_edit::pause_blinking()
{
    if (_caretTween) {
        _caretTween->pause();
        _caretVisible = true;
    }
}
void text_edit::resume_blinking()
{
    if (_caretTween) { _caretTween->resume(); }
}

void text_edit::update(milliseconds deltaTime)
{
    if (_caretTween) { _caretTween->update(deltaTime); }
}

void text_edit::insert_char(utf8_string const& ch)
{
    _text = utf8::insert(_text, ch, _caretPos);
    _textLength += utf8::length(ch);
    _caretPos += utf8::length(ch);
    TextChanged();
    Invalidated();
}

void text_edit::delete_backward()
{
    if (_textLength > 0 && _caretPos > 0) {
        --_caretPos;
        _text = utf8::remove(_text, _caretPos);
        --_textLength;
        TextChanged();
        Invalidated();
    }
}

void text_edit::delete_forward()
{
    if (_textLength > 0 && _caretPos < _textLength) {
        _text = utf8::remove(_text, _caretPos);
        --_textLength;
        TextChanged();
        Invalidated();
    }
}

void text_edit::move_caret_left()
{
    if (_caretPos > 0) {
        --_caretPos;
        Invalidated();
    }
}

void text_edit::move_caret_right()
{
    if (_caretPos < _textLength) {
        ++_caretPos;
        Invalidated();
    }
}

void text_edit::move_caret_home()
{
    _caretPos = 0;
    Invalidated();
}

void text_edit::move_caret_end()
{
    _caretPos = _textLength;
    Invalidated();
}

auto text_edit::caret_pos() const -> isize { return _caretPos; }
auto text_edit::caret_visible() const -> bool { return _caretVisible; }

auto text_edit::text_length() const -> isize { return _textLength; }

auto text_edit::get_text() const -> utf8_string const& { return _text; }

void text_edit::set_text(utf8_string const& t)
{
    _text       = t;
    _textLength = utf8::length(t);
    _caretPos   = std::min(_caretPos, _textLength);
    TextChanged();
    Invalidated();
}

}
