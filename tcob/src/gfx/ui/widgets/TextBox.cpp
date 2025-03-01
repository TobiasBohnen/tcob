// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <algorithm>

#include "tcob/gfx/ui/widgets/TextBox.hpp"

#include "tcob/core/StringUtils.hpp"
#include "tcob/gfx/ui/Form.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"

namespace tcob::gfx::ui {

void text_box::style::Transition(style& target, style const& left, style const& right, f64 step)
{
    widget_style::Transition(target, left, right, step);

    element::text::Transition(target.Text, left.Text, right.Text, step);
    element::caret::Transition(target.Caret, left.Caret, right.Caret, step);
}

text_box::text_box(init const& wi)
    : widget {wi}
{
    Text.Changed.connect([this](auto const& val) {
        _textLength = utf8::length(val);
        _textDirty  = true;
        _caretPos   = std::min(_caretPos, _textLength);
        force_redraw(this->name() + ": Text changed");
    });

    MaxLength(std::numeric_limits<usize>::max());

    Class("text_box");
}

void text_box::on_paint(widget_painter& painter)
{
    update_style(_style);

    rect_f rect {Bounds()};

    // background
    painter.draw_background_and_border(_style, rect, false);

    scissor_guard const guard {painter, this};

    // text
    if (!Text->empty() && _style.Text.Font) {
        if (_textDirty) {
            _formatResult = painter.format_text(_style.Text, rect, Text());
            _textDirty    = false;
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
    if (_caretTween) {
        _caretTween->pause();
        _caretVisible = true;
    }

    auto const& controls {parent_form()->Controls};
    if (ev.KeyCode == controls->NavLeftKey) {
        if (_caretPos > 0) {
            --_caretPos;
            force_redraw(this->name() + ": Caret moved");
        }
    } else if (ev.KeyCode == controls->NavRightKey) {
        if (_caretPos < _textLength) {
            ++_caretPos;
            force_redraw(this->name() + ": Caret moved");
        }
    } else if (ev.KeyCode == controls->ForwardDeleteKey) {
        if (_textLength > 0 && _caretPos < _textLength) {
            Text = utf8::remove(Text(), _caretPos);
        }
    } else if (ev.KeyCode == controls->BackwardDeleteKey) {
        if (_textLength > 0 && _caretPos > 0) {
            --_caretPos;
            Text = utf8::remove(Text(), _caretPos);
        }
    } else if (ev.KeyCode == controls->SubmitKey) {
        Submit({this, Text()});
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

void text_box::on_focus_gained()
{
    _caretPos = _textLength;

    _caretTween = make_unique_tween<square_wave_tween<bool>>(_style.Caret.BlinkRate, 1.0f, 0.0f);
    _caretTween->Value.Changed.connect([this](auto val) {
        _caretVisible = val;
        force_redraw(this->name() + ": Caret blink");
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
    widget_attributes retValue {{"text", Text()}};
    auto const        base {widget::attributes()};
    retValue.insert(base.begin(), base.end());
    return retValue;
}

void text_box::insert_text(utf8_string const& newText)
{
    text_event ev {this, newText};
    BeforeTextInserted(ev);
    usize const newTextLength {utf8::length(ev.Text)};
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

} // namespace ui
