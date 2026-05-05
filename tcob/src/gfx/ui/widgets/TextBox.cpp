// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/TextBox.hpp"

#include <algorithm>
#include <limits>

#include "tcob/core/Point.hpp"
#include "tcob/core/Property.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/core/StringUtils.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/ui/Form.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/StyleElements.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui {

void text_box::style::Transition(style& target, style const& from, style const& to, f64 step)
{
    widget_style::Transition(target, from, to, step);

    target.Text.lerp(from.Text, to.Text, step);
    target.Caret.lerp(from.Caret, to.Caret, step);
}

text_box::text_box(init const& wi)
    : widget {wi}
    , Text {make_prop_fn<string,
                         [](text_box const& w) { return w._edit.get_text(); },
                         [](text_box& w, string const& value) { w._edit.set_text(value); }>(this)}
{
    _edit.Invalidated.connect([this] { queue_redraw(); });
    _edit.TextChanged.connect([this] {
        _needsFormat = true;
        queue_redraw();
    });

    MaxLength.Changed.connect([this](auto const& val) {
        if (_edit.text_length() > val) {
            _edit.set_text(utf8::substr(_edit.get_text(), 0, val));
            queue_redraw();
        }
    });
    MaxLength(std::numeric_limits<isize>::max());

    Selectable.Changed.connect([this](auto const& val) {
        if (!val) { select_text(INVALID_INDEX, INVALID_INDEX); }
        queue_redraw();
    });
    Selectable(false);

    Class("text_box");
}

auto text_box::selected_text() const -> utf8_string
{
    if (!is_text_selected()) { return ""; }
    return utf8::substr(*Text, _selectedText.first, _selectedText.second - _selectedText.first + 1);
}

void text_box::select_text(isize first, isize last)
{
    _selectedText.first  = std::min(first, last);
    _selectedText.second = std::max(first, last);
    if (Selectable) { queue_redraw(); }
}

void text_box::deselect_text()
{
    select_text(INVALID_INDEX, INVALID_INDEX);
}

auto text_box::is_text_selected() const -> bool
{
    return Selectable && _selectedText.first != INVALID_INDEX && _selectedText.second != INVALID_INDEX;
}

void text_box::on_draw(widget_painter& painter)
{
    rect_f const         rect {draw_base(_style, painter)};
    scoped_scissor const guard {painter, this};

    if (!(*Text).empty() && _style.Text.Font) {
        if (_needsFormat) {
            _formatResult = painter.format_text(_style.Text, rect.Size, *Text);
            _needsFormat  = false;
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

    if (_edit.caret_visible()) {
        f32 offset {0.0f};
        if (!_formatResult.Tokens.empty()) {
            isize const cp {_edit.caret_pos()};
            offset = cp == 0 ? _formatResult.get_quad(cp).Rect.left()
                             : _formatResult.get_quad(cp - 1).Rect.right();
        }
        painter.draw_caret(_style.Caret, rect, {offset, 0});
    }
}

void text_box::on_update(milliseconds deltaTime)
{
    _edit.update(deltaTime);
    if (form().top_widget() == this) {
        form().change_cursor_mode(cursor_mode::Text);
    }
}

void text_box::on_key_down(input::keyboard::event const& ev)
{
    using namespace tcob::enum_ops;
    _edit.pause_blinking();

    auto const& controls {form().Controls};

    if (ev.KeyCode == controls->NavLeftKey) {
        if (_edit.caret_pos() > 0) {
            isize const refPos {_edit.caret_pos() - 1};
            if (ev.KeyMods.is_down(controls->SelectMod)) {
                if (!is_text_selected()) {
                    select_text(refPos, refPos);
                } else if (_selectedText.first == refPos) {
                    deselect_text();
                } else {
                    bool const b {_selectedText.first < refPos};
                    select_text(b ? _selectedText.first : refPos,
                                b ? refPos - 1 : _selectedText.second);
                }
                _edit.move_caret_left();
            } else {
                if (is_text_selected()) {
                    while (_edit.caret_pos() > _selectedText.first) { _edit.move_caret_left(); }
                    deselect_text();
                } else {
                    _edit.move_caret_left();
                }
            }
        } else if (is_text_selected() && !ev.KeyMods.is_down(controls->SelectMod)) {
            deselect_text();
        }
    } else if (ev.KeyCode == controls->NavRightKey) {
        if (_edit.caret_pos() < _edit.text_length()) {
            isize const refPos {_edit.caret_pos()};
            if (ev.KeyMods.is_down(controls->SelectMod)) {
                if (!is_text_selected()) {
                    select_text(refPos, refPos);
                } else if (_selectedText.second == refPos) {
                    deselect_text();
                } else {
                    bool const b {_selectedText.second > refPos};
                    select_text(b ? refPos + 1 : _selectedText.first,
                                b ? _selectedText.second : refPos);
                }
                _edit.move_caret_right();
            } else {
                if (is_text_selected()) {
                    while (_edit.caret_pos() <= _selectedText.second) { _edit.move_caret_right(); }
                    deselect_text();
                } else {
                    _edit.move_caret_right();
                }
            }
        } else if (is_text_selected() && !ev.KeyMods.is_down(controls->SelectMod)) {
            deselect_text();
        }
    } else if (ev.KeyCode == controls->ForwardDeleteKey) {
        if (!remove_selected_text()) { _edit.delete_forward(); }
    } else if (ev.KeyCode == controls->BackwardDeleteKey) {
        if (!remove_selected_text()) { _edit.delete_backward(); }
    } else if (ev.KeyCode == controls->SubmitKey) {
        Submit({.Sender = this, .Text = *Text});
    } else if (ev.KeyMods.is_down(controls->CutCopyPasteMod)) {
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
    _edit.resume_blinking();
    ev.Handled = true;
}

void text_box::on_text_input(input::keyboard::text_input_event const& ev)
{
    // TODO: active text input on focus
    insert_text(ev.Text);
    ev.Handled = true;
}

void text_box::on_mouse_drag(input::mouse::motion_event const& ev)
{
    isize const target {calc_caret_pos(screen_to_content(*this, ev.Position))};
    if (_edit.caret_pos() != target) {
        if (target < _dragCaretPos) {
            select_text(_dragCaretPos - 1, target);
        } else if (target > _dragCaretPos) {
            select_text(_dragCaretPos, target - 1);
        } else {
            deselect_text();
        }

        while (_edit.caret_pos() < target) { _edit.move_caret_right(); }
        while (_edit.caret_pos() > target) { _edit.move_caret_left(); }
        ev.Handled = true;
    }
}

void text_box::on_mouse_button_down(input::mouse::button_event const& ev)
{
    if (ev.Button != controls().PrimaryMouseButton) { return; }
    isize const target {calc_caret_pos(screen_to_content(*this, ev.Position))};
    if (_edit.caret_pos() != target) {
        deselect_text();
        while (_edit.caret_pos() < target) { _edit.move_caret_right(); }
        while (_edit.caret_pos() > target) { _edit.move_caret_left(); }
    }
    _dragCaretPos = target;
    ev.Handled    = true;
}

void text_box::on_mouse_button_up(input::mouse::button_event const& ev)
{
    if (ev.Button != controls().PrimaryMouseButton) { return; }

    _dragCaretPos = -1;
    ev.Handled    = true;
}

void text_box::on_focus_gained()
{
    _edit.start_blinking(_style.Caret.BlinkRate);
}

void text_box::on_focus_lost()
{
    _edit.stop_blinking();
    FocusLost({.Sender = this});
}

auto text_box::attributes() const -> widget_attributes
{
    auto retValue {widget::attributes()};

    retValue["text"]          = *Text;
    retValue["selected_text"] = selected_text();
    retValue["max_length"]    = *MaxLength;
    retValue["selectedable"]  = *Selectable;

    return retValue;
}

void text_box::insert_text(utf8_string const& newText)
{
    remove_selected_text();

    text_event ev {.Sender = this, .Text = newText};
    BeforeTextInserted(ev);
    isize const newTextLength {utf8::length(ev.Text)};
    if (newTextLength > 0 && _edit.text_length() + newTextLength <= MaxLength) {
        _edit.insert_char(ev.Text);
    }
}

void text_box::on_styles_changed()
{
    widget::on_styles_changed();
    _needsFormat = true;
}

auto text_box::remove_selected_text() -> bool
{
    if (is_text_selected()) {
        isize const count {_selectedText.second - _selectedText.first + 1};
        while (_edit.caret_pos() > _selectedText.first) { _edit.move_caret_left(); }
        for (isize i {0}; i < count; ++i) { _edit.delete_forward(); }
        deselect_text();
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
    if (mp.X >= lastRect.center().X) { return _edit.text_length(); }

    // center check
    for (isize i {0}; i < _formatResult.QuadCount; ++i) {
        auto const rect {_formatResult.get_quad(i).Rect};
        if (mp.X < rect.center().X) { return i; }
    }
    return _edit.text_length();
}

}
