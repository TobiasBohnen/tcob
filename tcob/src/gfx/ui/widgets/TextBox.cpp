// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/TextBox.hpp"

#include <algorithm>
#include <cctype>
#include <limits>

#include "tcob/core/Point.hpp"
#include "tcob/core/Property.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/StringUtils.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/ui/Form.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/UIEvents.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"
#include "tcob/gfx/ui/component/StyleElements.hpp"
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

    MaxLength.Changed.connect([this](auto const& val) {
        if (_edit.text_length() > val) {
            _edit.set_text(utf8::substr(_edit.get_text(), 0, val));
            queue_redraw();
        }
    });
    MaxLength(std::numeric_limits<isize>::max());

    Selectable.Changed.connect([this](auto const& val) {
        if (!val) { _edit.deselect_text(); }
        queue_redraw();
    });
    Selectable(false);

    NumericOnly(false);

    Class("text_box");
}

auto text_box::selected_text() const -> utf8_string
{
    return _edit.selected_text();
}

void text_box::on_draw(widget_painter& painter)
{
    rect_f const         rect {draw_base(_style, painter)};
    scoped_scissor const guard {painter, this};

    _edit.draw(painter, rect, _style.Text, _style.Caret);
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

    auto const& controls {form().Controls};
    if (controls->SubmitKeys.contains(ev.KeyCode)) {
        if (NumericOnly) {
            auto const& t {_edit.get_text()};
            if (!helper::to_number<f64>(t).has_value()) {
                _edit.set_text(_lastValidText);
                return;
            }
        }
        Submit({.Sender = this, .Text = *Text});
    } else if (ev.KeyMods.is_down(controls->CutCopyPasteMod)) {
        if (ev.KeyCode == controls->CopyKey) {
            locate_service<input::system>().clipboard().set_text(selected_text());
        } else if (ev.KeyCode == controls->CutKey) {
            locate_service<input::system>().clipboard().set_text(selected_text());
            _edit.remove_selected_text();
        }
    } else {
        _edit.key_down(*this, ev.KeyMods, ev.KeyCode, Selectable);
    }

    ev.Handled = true;
}

void text_box::on_key_up(input::keyboard::event const& ev)
{
    _edit.key_up();
    ev.Handled = true;
}

void text_box::on_text_input(input::keyboard::text_input_event const& ev)
{
    insert_text(ev.Text);
    ev.Handled = true;
}

void text_box::on_mouse_drag(input::mouse::motion_event const& ev)
{
    if (Selectable) {
        _edit.mouse_drag(screen_to_content(*this, ev.Position));
        queue_redraw();
        ev.Handled = true;
    }
}

void text_box::on_mouse_button_down(input::mouse::button_event const& ev)
{
    if (ev.Button != controls().PrimaryMouseButton) { return; }

    _edit.mouse_button_down(screen_to_content(*this, ev.Position));
    ev.Handled = true;
}

void text_box::on_mouse_button_up(input::mouse::button_event const& ev)
{
    if (ev.Button != controls().PrimaryMouseButton) { return; }

    _edit.mouse_button_up();
    ev.Handled = true;
}

void text_box::on_focus_gained()
{
    _lastValidText = *Text;
    _edit.start_blinking(_style.Caret.BlinkRate);
}

void text_box::on_focus_lost()
{
    if (NumericOnly) {
        auto const& t {_edit.get_text()};
        if (!helper::to_number<f64>(t).has_value()) { _edit.set_text(_lastValidText); }
    }
    _edit.stop_blinking();
    FocusLost({.Sender = this});
}

auto text_box::attributes() const -> widget_attributes
{
    auto retValue {widget::attributes()};

    retValue["text"]          = *Text;
    retValue["selected_text"] = selected_text();
    retValue["max_length"]    = *MaxLength;
    retValue["selectable"]    = *Selectable;
    retValue["numeric_only"]  = *NumericOnly;

    return retValue;
}

void text_box::insert_text(utf8_string const& newText)
{
    text_event ev {.Sender = this, .Text = newText};
    BeforeTextInserted(ev);

    if (NumericOnly) {
        bool const valid {std::ranges::all_of(ev.Text, [](char c) {
            return std::isdigit(c) || c == '-' || c == '.';
        })};
        if (!valid) { return; }
    }

    isize const newTextLength {utf8::length(ev.Text)};
    if (newTextLength > 0 && _edit.text_length() + newTextLength <= MaxLength) {
        _edit.insert_text(ev.Text);
    }
}

}
