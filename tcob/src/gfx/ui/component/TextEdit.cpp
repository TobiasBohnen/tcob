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
#include "tcob/core/input/Input_Codes.hpp"
#include "tcob/core/tweening/Tween.hpp"
#include "tcob/gfx/TextFormatter.hpp"
#include "tcob/gfx/ui/Form.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"
#include "tcob/gfx/ui/component/StyleElements.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

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

void text_edit::draw(widget_painter& painter, rect_f const& rect, text_element const& text, caret_element const& caret)
{
    if (!text.Font) {
        _formatResult = {};
        return;
    }

    if (_needsFormat) {
        _formatResult = painter.format_text(text, rect.Size, _text);
        _needsFormat  = false;
    }

    if (_selectedText.first >= _formatResult.QuadCount || _selectedText.second >= _formatResult.QuadCount) {
        deselect_text();
    }

    if (_textLength > 0) {
        if (is_text_selected()) {
            auto& canvas {painter.canvas()};
            canvas.set_fill_style(text.SelectColor);
            canvas.begin_path();

            auto const first {_formatResult.get_quad(_selectedText.first).value_or(gfx::text_formatter::quad_definition {})};

            size_f size {};
            size.Width  = _formatResult.get_quad(_selectedText.second).value_or(gfx::text_formatter::quad_definition {}).Rect.right() - first.Rect.left();
            size.Height = rect.height() * 0.9f;
            point_f pos {};
            pos.X = first.Rect.left();
            pos.Y = (rect.height() - size.Height) / 2;

            canvas.rect({pos + rect.Position, size});
            canvas.fill();
        }

        painter.draw_text(text, rect, _formatResult);
    }

    if (_caretVisible) {
        f32 offset {0.0f};
        if (!_formatResult.Tokens.empty()) {
            isize const cp {_caretPos};
            offset = cp == 0 ? _formatResult.get_quad(cp).value_or(gfx::text_formatter::quad_definition {}).Rect.left()
                             : _formatResult.get_quad(cp - 1).value_or(gfx::text_formatter::quad_definition {}).Rect.right();
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
    mark_dirty();
    TextChanged();
}

void text_edit::delete_backward()
{
    if (_textLength > 0 && _caretPos > 0) {
        --_caretPos;
        _text = utf8::remove(_text, _caretPos);
        --_textLength;
        mark_dirty();
        TextChanged();
    }
}

void text_edit::delete_forward()
{
    if (_textLength > 0 && _caretPos < _textLength) {
        _text = utf8::remove(_text, _caretPos);
        --_textLength;
        mark_dirty();
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
    if (_text == t) { return; }

    _text       = t;
    _textLength = utf8::length(t);
    _caretPos   = std::min(_caretPos, _textLength);
    mark_dirty();
    TextChanged();
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

auto text_edit::selected_text() const -> utf8_string
{
    if (!is_text_selected()) { return ""; }
    return utf8::substr(_text, _selectedText.first, _selectedText.second - _selectedText.first + 1);
}

void text_edit::key_down(widget const& widget, input::key_mods keyMods, input::key_code keyCode, bool selectable)
{
    pause_blinking();

    auto const& controls {*widget.form().Controls};

    if (controls.NavLeftKeys.contains(keyCode)) {
        if (_caretPos > 0) {
            isize const refPos {_caretPos - 1};
            if (keyMods.is_down(controls.SelectMod) && selectable) {
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
        } else if (is_text_selected() && !keyMods.is_down(controls.SelectMod)) {
            deselect_text();
        }
    } else if (controls.NavRightKeys.contains(keyCode)) {
        if (_caretPos < _textLength) {
            isize const refPos {_caretPos};
            if (keyMods.is_down(controls.SelectMod) && selectable) {
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
        } else if (is_text_selected() && !keyMods.is_down(controls.SelectMod)) {
            deselect_text();
        }
    } else if (keyCode == controls.ForwardDeleteKey) {
        if (!remove_selected_text()) { delete_forward(); }
    } else if (keyCode == controls.BackwardDeleteKey) {
        if (!remove_selected_text()) { delete_backward(); }
    }
}

void text_edit::key_up()
{
    resume_blinking();
}

void text_edit::mouse_drag(point_f mp)
{
    auto const targetCaretPos {calc_caret_pos(mp)};
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

void text_edit::mouse_button_down(point_f mp)
{
    auto const targetCaretPos {calc_caret_pos(mp)};
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

auto text_edit::calc_caret_pos(point_f mp) const -> isize
{
    if (_formatResult.QuadCount == 0) { return 0; }

    // before first
    auto const& firstRect {_formatResult.get_quad(0)->Rect};
    if (mp.X <= firstRect.center().X) { return 0; }
    // after last
    auto const& lastRect {_formatResult.get_quad(_formatResult.QuadCount - 1)->Rect};
    if (mp.X >= lastRect.center().X) { return _textLength; }

    // center check
    for (isize i {0}; i < _formatResult.QuadCount; ++i) {
        auto const rect {_formatResult.get_quad(i)->Rect};
        if (mp.X < rect.center().X) { return i; }
    }
    return _textLength;
}

void text_edit::mark_dirty()
{
    Invalidated();
    _needsFormat = true;
}

}
