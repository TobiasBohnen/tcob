// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/TextBox.hpp"

#include <algorithm>
#include <limits>

#include "tcob/core/Common.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/core/StringUtils.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/animation/Tween.hpp"
#include "tcob/gfx/ui/Form.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui {

void text_box::style::Transition(style& target, style const& left, style const& right, f64 step)
{
    widget_style::Transition(target, left, right, step);

    text_element::Transition(target.Text, left.Text, right.Text, step);
    caret_element::Transition(target.Caret, left.Caret, right.Caret, step);
}

text_box::text_box(init const& wi)
    : widget {wi}
{
    Text.Changed.connect([this](auto const& val) {
        _textLength = utf8::length(val);
        _textDirty  = true;
        _caretPos   = std::min(_caretPos, _textLength);
        request_redraw(this->name() + ": Text changed");
    });

    MaxLength.Changed.connect([this](auto const& val) {
        if (_textLength > val) {
            Text = utf8::substr(Text(), 0, val);
            request_redraw(this->name() + ": MaxLength changed");
        }
    });
    MaxLength(std::numeric_limits<isize>::max());

    Selectable.Changed.connect([this](auto const& val) {
        if (!val) {
            select_text(INVALID_INDEX, INVALID_INDEX);
        }
    });
    Selectable(false);

    Class("text_box");
}

auto text_box::selected_text() const -> utf8_string
{
    if (!is_text_selected()) { return ""; }
    return utf8::substr(Text(), _selectedText.first, _selectedText.second - _selectedText.first + 1);
}

void text_box::select_text(isize first, isize last)
{
    _selectedText.first  = std::min(first, last);
    _selectedText.second = std::max(first, last);

    request_redraw(this->name() + ": SelectedText changed");
}

auto text_box::is_text_selected() const -> bool
{
    return _selectedText.first != INVALID_INDEX && _selectedText.second != INVALID_INDEX;
}

void text_box::set_caret_pos(isize pos)
{
    if (_caretPos == pos) { return; }
    _caretPos = pos;
    request_redraw(this->name() + ": Caret moved");
}

void text_box::on_draw(widget_painter& painter)
{
    rect_f rect {draw_background(_style, painter)};

    scissor_guard const guard {painter, this};

    // text
    if (!Text->empty() && _style.Text.Font) {
        if (_textDirty) {
            _formatResult = painter.format_text(_style.Text, rect, Text());
            _textDirty    = false;
        }

        if (_selectedText.first >= _formatResult.QuadCount || _selectedText.second >= _formatResult.QuadCount) {
            _selectedText = {INVALID_INDEX, INVALID_INDEX};
        }
        if (is_text_selected()) {
            auto& canvas {painter.canvas()};
            canvas.set_fill_style(_style.Text.SelectColor);
            canvas.begin_path();

            auto const& first {_formatResult.get_quad(_selectedText.first).Rect};

            size_f size {};
            size.Width  = _formatResult.get_quad(_selectedText.second).Rect.right() - first.left();
            size.Height = rect.height() * 0.9f;
            point_f pos {};
            pos.X = first.left();
            pos.Y = (rect.height() - size.Height) / 2;

            canvas.rect({pos + rect.Position, size});
            canvas.fill();
        }

        painter.draw_text(_style.Text, rect, _formatResult);
    } else {
        _formatResult = {};
    }

    if (_caretVisible) {
        f32 offset {0.0f};
        if (!_formatResult.Tokens.empty()) {
            if (_caretPos == 0) {
                offset = _formatResult.get_quad(_caretPos).Rect.left();
            } else {
                offset = _formatResult.get_quad(_caretPos - 1).Rect.right();
            }
        }
        painter.draw_caret(_style.Caret, rect, {offset, 0});
    }
}

void text_box::on_update(milliseconds deltaTime)
{
    if (_caretTween) {
        _caretTween->update(deltaTime);
    }
}

void text_box::on_key_down(input::keyboard::event const& ev)
{
    using namespace tcob::enum_ops;

    if (_caretTween) {
        _caretTween->pause();
        _caretVisible = true;
    }

    auto const& controls {parent_form()->Controls};
    if (ev.KeyCode == controls->NavLeftKey) {
        if (_caretPos > 0) {
            set_caret_pos(_caretPos - 1);
        }
    } else if (ev.KeyCode == controls->NavRightKey) {
        if (_caretPos < _textLength) {
            set_caret_pos(_caretPos + 1);
        }
    } else if (ev.KeyCode == controls->ForwardDeleteKey) {
        if (!remove_selected_text()) {
            if (_textLength > 0 && _caretPos < _textLength) {
                Text = utf8::remove(Text(), _caretPos);
            }
        }
    } else if (ev.KeyCode == controls->BackwardDeleteKey) {
        if (!remove_selected_text()) {
            if (_textLength > 0 && _caretPos > 0) {
                --_caretPos;
                Text = utf8::remove(Text(), _caretPos);
            }
        }
    } else if (ev.KeyCode == controls->SubmitKey) {
        Submit({this, Text()});
    } else if ((ev.KeyMods & controls->CutCopyPasteMod) == controls->CutCopyPasteMod) {
        if (is_text_selected()) {
            if (ev.KeyCode == controls->CopyKey) {
                locate_service<input::system>().clipboard().set_text(selected_text());
            } else if (ev.KeyCode == controls->CutKey) {
                locate_service<input::system>().clipboard().set_text(selected_text());
                remove_selected_text();
            }
        }
    }

    ev.Handled = true;
}

void text_box::on_key_up(input::keyboard::event const& ev)
{
    if (_caretTween) {
        _caretTween->resume();
        ev.Handled = true;
    }
}

void text_box::on_text_input(input::keyboard::text_input_event const& ev)
{
    insert_text(ev.Text);
    ev.Handled = true;
}

void text_box::on_text_editing(input::keyboard::text_editing_event const& /* ev */)
{
}

void text_box::on_mouse_drag(input::mouse::motion_event const& ev)
{
    isize const target {calc_caret_pos(global_to_content(*this, ev.Position))};
    if (_caretPos != target) {
        if (target < _dragCaretPos) {
            select_text(_dragCaretPos - 1, target);
        } else if (target > _dragCaretPos) {
            select_text(_dragCaretPos, target - 1);
        } else {
            select_text(INVALID_INDEX, INVALID_INDEX);
        }

        set_caret_pos(target);
        ev.Handled = true;
    }
}

void text_box::on_mouse_down(input::mouse::button_event const& ev)
{
    isize const target {calc_caret_pos(global_to_content(*this, ev.Position))};
    if (_caretPos != target) {
        select_text(INVALID_INDEX, INVALID_INDEX);
        set_caret_pos(target);
    }
    _dragCaretPos = target;
    ev.Handled    = true;
}

void text_box::on_mouse_up(input::mouse::button_event const& ev)
{
    _dragCaretPos = -1;
    ev.Handled    = true;
}

void text_box::on_focus_gained()
{
    _caretTween = gfx::make_unique_tween<gfx::square_wave_tween<bool>>(_style.Caret.BlinkRate, 1.0f, 0.0f);
    _caretTween->Value.Changed.connect([this](auto val) {
        _caretVisible = val;
        request_redraw(this->name() + ": Caret blink");
    });
    _caretTween->start(playback_mode::Looped);
}

void text_box::on_focus_lost()
{
    _caretVisible = false;
    _caretTween   = nullptr;
}

auto text_box::attributes() const -> widget_attributes
{
    widget_attributes retValue {{"text", Text()}, {"selected_text", selected_text()}};
    auto const        base {widget::attributes()};
    retValue.insert(base.begin(), base.end());
    return retValue;
}

void text_box::insert_text(utf8_string const& newText)
{
    remove_selected_text();

    text_event ev {this, newText};
    BeforeTextInserted(ev);
    isize const newTextLength {utf8::length(ev.Text)};
    if (newTextLength > 0 && _textLength + newTextLength <= MaxLength) {
        Text = utf8::insert(Text(), ev.Text, _caretPos);
        _caretPos += newTextLength;
    }
}

void text_box::on_styles_changed()
{
    widget::on_styles_changed();
    _textDirty = true;
}

auto text_box::remove_selected_text() -> bool
{
    if (is_text_selected()) {
        Text      = utf8::remove(Text(), _selectedText.first, _selectedText.second - _selectedText.first + 1);
        _caretPos = _selectedText.first;
        select_text(INVALID_INDEX, INVALID_INDEX);
        return true;
    }

    return false;
}

auto text_box::calc_caret_pos(point_f mp) const -> isize
{
    if (_formatResult.QuadCount == 0) { return 0; }

    // before first
    auto const& firstRect {_formatResult.get_quad(0).Rect};
    if (mp.X <= firstRect.center().X) { return 0; }
    // after last
    auto const& lastRect {_formatResult.get_quad(_formatResult.QuadCount - 1).Rect};
    if (mp.X >= lastRect.center().X) { return _textLength; }

    // center check
    for (isize i {0}; i < _formatResult.QuadCount; ++i) {
        auto const rect {_formatResult.get_quad(i).Rect};
        f32 const  mid {rect.center().X};
        if (mp.X < mid) { return i; }
    }

    return _textLength;
}

} // namespace ui
