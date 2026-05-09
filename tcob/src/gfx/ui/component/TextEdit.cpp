// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/component/TextEdit.hpp"

#include <algorithm>
#include <utility>

#include "tcob/core/Common.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/core/StringUtils.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/core/tweening/Tween.hpp"
#include "tcob/gfx/TextFormatter.hpp"
#include "tcob/gfx/ui/StyleElements.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"

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

void text_edit::draw(widget_painter& painter, rect_f const& rect, gfx::text_formatter::result const& formatResult, text_element const& text, caret_element const& caret)
{
    auto const selText {selected_text_indices()};
    if (selText.first >= formatResult.QuadCount || selText.second >= formatResult.QuadCount) {
        deselect_text();
    }

    if (_textLength > 0) {
        if (is_text_selected()) {
            auto& canvas {painter.canvas()};
            canvas.set_fill_style(text.SelectColor);
            canvas.begin_path();

            auto const first {formatResult.get_quad(selText.first).value_or(gfx::text_formatter::quad_definition {})};

            size_f size {};
            size.Width  = formatResult.get_quad(selText.second).value_or(gfx::text_formatter::quad_definition {}).Rect.right() - first.Rect.left();
            size.Height = rect.height() * 0.9f;
            point_f pos {};
            pos.X = first.Rect.left();
            pos.Y = (rect.height() - size.Height) / 2;

            canvas.rect({pos + rect.Position, size});
            canvas.fill();
        }

        painter.draw_text(text, rect, formatResult);
    }

    if (_caretVisible) {
        f32 offset {0.0f};
        if (!formatResult.Tokens.empty()) {
            isize const cp {_caretPos};
            offset = cp == 0 ? formatResult.get_quad(cp).value_or(gfx::text_formatter::quad_definition {}).Rect.left()
                             : formatResult.get_quad(cp - 1).value_or(gfx::text_formatter::quad_definition {}).Rect.right();
        }
        painter.draw_caret(caret, rect, {offset, 0});
    }
}

void text_edit::insert_text(utf8_string const& ch)
{
    remove_selected_text();
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

auto text_edit::selected_text_indices() const -> std::pair<isize, isize>
{
    return _selectedText;
}

void text_edit::select_text(isize first, isize last)
{
    _selectedText.first  = std::min(first, last);
    _selectedText.second = std::max(first, last);
}

void text_edit::deselect_text()
{
    select_text(INVALID_INDEX, INVALID_INDEX);
}

auto text_edit::is_text_selected() const -> bool
{
    return _selectedText.first != INVALID_INDEX && _selectedText.second != INVALID_INDEX;
}

void text_edit::key_down(input::keyboard::event const& ev, control_map const& controls, bool selectable)
{
    pause_blinking();

    if (controls.NavLeftKeys.contains(ev.KeyCode)) {
        if (_caretPos > 0) {
            isize const refPos {_caretPos - 1};
            if (ev.KeyMods.is_down(controls.SelectMod) && selectable) {
                if (!is_text_selected()) {
                    select_text(refPos, refPos);
                } else if (_selectedText.first == refPos) {
                    deselect_text();
                } else {
                    bool const b {_selectedText.first < refPos};
                    select_text(b ? _selectedText.first : refPos,
                                b ? refPos - 1 : _selectedText.second);
                }
                move_caret_left();
            } else {
                if (is_text_selected()) {
                    while (_caretPos > _selectedText.first) { move_caret_left(); }
                    deselect_text();
                } else {
                    move_caret_left();
                }
            }
        } else if (is_text_selected() && !ev.KeyMods.is_down(controls.SelectMod)) {
            deselect_text();
        }
    } else if (controls.NavRightKeys.contains(ev.KeyCode)) {
        if (_caretPos < text_length()) {
            isize const refPos {_caretPos};
            if (ev.KeyMods.is_down(controls.SelectMod) && selectable) {
                if (!is_text_selected()) {
                    select_text(refPos, refPos);
                } else if (_selectedText.second == refPos) {
                    deselect_text();
                } else {
                    bool const b {_selectedText.second > refPos};
                    select_text(b ? refPos + 1 : _selectedText.first,
                                b ? _selectedText.second : refPos);
                }
                move_caret_right();
            } else {
                if (is_text_selected()) {
                    while (_caretPos <= _selectedText.second) { move_caret_right(); }
                    deselect_text();
                } else {
                    move_caret_right();
                }
            }
        } else if (is_text_selected() && !ev.KeyMods.is_down(controls.SelectMod)) {
            deselect_text();
        }
    } else if (ev.KeyCode == controls.ForwardDeleteKey) {
        if (!remove_selected_text()) { delete_forward(); }
    } else if (ev.KeyCode == controls.BackwardDeleteKey) {
        if (!remove_selected_text()) { delete_backward(); }
    }
}

void text_edit::key_up(input::keyboard::event const& /* ev */, control_map const& /* controls */)
{
    resume_blinking();
}

void text_edit::drag_select(isize targetCaretPos)
{
    if (_caretPos != targetCaretPos) {
        if (targetCaretPos < _dragCaretPos) {
            select_text(_dragCaretPos - 1, targetCaretPos);
        } else if (targetCaretPos > _dragCaretPos) {
            select_text(_dragCaretPos, targetCaretPos - 1);
        } else {
            deselect_text();
        }

        while (_caretPos < targetCaretPos) { move_caret_right(); }
        while (_caretPos > targetCaretPos) { move_caret_left(); }
    }
}

void text_edit::mouse_button_down(isize targetCaretPos)
{
    if (_caretPos != targetCaretPos) {
        deselect_text();
        while (_caretPos < targetCaretPos) { move_caret_right(); }
        while (_caretPos > targetCaretPos) { move_caret_left(); }
    }
    _dragCaretPos = targetCaretPos;
}

void text_edit::mouse_button_up()
{
    _dragCaretPos = -1;
}

auto text_edit::remove_selected_text() -> bool
{
    if (is_text_selected()) {
        isize const count {_selectedText.second - _selectedText.first + 1};
        while (_caretPos > _selectedText.first) { move_caret_left(); }
        for (isize i {0}; i < count; ++i) { delete_forward(); }
        deselect_text();
        return true;
    }

    return false;
}

}
