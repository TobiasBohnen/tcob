// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/Spinner.hpp"

#include <algorithm>
#include <cctype>
#include <format>
#include <string>

#include "tcob/core/Common.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/StringUtils.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/core/input/Input_Codes.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/ui/Form.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/StyleElements.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui {

void spinner::style::Transition(style& target, style const& from, style const& to, f64 step)
{
    widget_style::Transition(target, from, to, step);

    target.Text.lerp(from.Text, to.Text, step);
    target.Caret.lerp(from.Caret, to.Caret, step);
}

spinner::spinner(init const& wi)
    : widget {wi}
    , Min {{[this](f32 val) -> f32 { return std::min(val, *Max); }}}
    , Max {{[this](f32 val) -> f32 { return std::max(val, *Min); }}}
    , Value {{[this](f32 val) -> f32 { return std::clamp(val, *Min, *Max); }}}
{
    _edit.Invalidated.connect([this] { queue_redraw(); });
    _edit.TextChanged.connect([this] {
        _needsFormat = true;
        queue_redraw();
    });

    Min.Changed.connect([this](f32 val) {
        Value = std::max(val, *Value);
        queue_redraw();
    });
    Min(0);

    Max.Changed.connect([this](f32 val) {
        Value = std::min(val, *Value);
        queue_redraw();
    });
    Max(100);

    Step.Changed.connect([this](auto const&) { queue_redraw(); });
    Step(5);

    Value.Changed.connect([this](auto const&) { queue_redraw(); });
    Value(0);

    Precision(2);

    Class("spinner");
}

void spinner::enter_edit_mode()
{
    _editing = true;
    _edit.set_text({});

    _edit.start_blinking(_style.Caret.BlinkRate);
    _needsFormat = true;
    queue_redraw();
}

void spinner::commit_edit()
{
    auto const parsed {helper::to_number<f32>(_edit.get_text())};

    if (parsed) {
        Value = *parsed;
    }

    cancel_edit();
}

void spinner::cancel_edit()
{
    _editing = false;
    _edit.stop_blinking();
    _needsFormat = true;
    queue_redraw();
}

void spinner::on_draw(widget_painter& painter)
{
    rect_f const         rect {draw_base(_style, painter)};
    scoped_scissor const guard {painter, this};

    if (_editing) {
        if (_style.Text.Font) {
            if (_needsFormat) {
                _formatResult = painter.format_text(_style.Text, rect.Size, _edit.get_text());
                _needsFormat  = false;
            }
            _edit.draw(painter, rect, _formatResult, _style.Text, _style.Caret);
        }
        return;
    }

    // normal mode — arrows + value text
    auto const&      fls {flags()};
    nav_arrows_style incArrowStyle {};
    prepare_sub_style(incArrowStyle, 0, _style.NavArrowClass,
                      {.Active = fls.Active && _hoverArrow == arrow::Increase,
                       .Hover  = !fls.Active && _hoverArrow == arrow::Increase});
    nav_arrows_style decArrowStyle {};
    prepare_sub_style(decArrowStyle, 1, _style.NavArrowClass,
                      {.Active = fls.Active && _hoverArrow == arrow::Decrease,
                       .Hover  = !fls.Active && _hoverArrow == arrow::Decrease});

    rect_f incRect {rect};
    incRect.Size.Height /= 2;
    _rectCache.first = painter.draw_nav_arrow(incArrowStyle.NavArrow, incRect, direction::Up, gfx::horizontal_alignment::Right);
    rect_f decRect {incRect};
    decRect.Position.Y += decRect.height();
    _rectCache.second = painter.draw_nav_arrow(decArrowStyle.NavArrow, decRect, direction::Down, gfx::horizontal_alignment::Right);

    // text
    if (_style.Text.Font) {
        string const text {std::format("{:.{}f}", *Value, *Precision)};
        painter.draw_text(_style.Text, {rect.left(), rect.top(), rect.width() - _rectCache.first.width(), rect.height()}, text);
    }
}

void spinner::on_mouse_leave()
{
    if (_editing) { return; }
    _mouseDown  = false;
    _holdCount  = 1;
    _hoverArrow = arrow::None;
}

void spinner::on_mouse_hover(input::mouse::motion_event const& ev)
{
    if (_editing) { return; }
    auto const mp {screen_to_local(*this, ev.Position)};
    if (_rectCache.first.contains(mp)) {
        if (_hoverArrow != arrow::Increase) {
            _hoverArrow = arrow::Increase;
            ev.Handled  = true;
        }
    } else if (_rectCache.second.contains(mp)) {
        if (_hoverArrow != arrow::Decrease) {
            _hoverArrow = arrow::Decrease;
            ev.Handled  = true;
        }
    } else if (_hoverArrow != arrow::None) {
        _hoverArrow = arrow::None;
        ev.Handled  = true;
    }
}

void spinner::on_mouse_button_down(input::mouse::button_event const& ev)
{
    if (ev.Button != controls().PrimaryMouseButton) { return; }

    if (_editing) { return; }
    if (_hoverArrow == arrow::None) { return; }

    if (_hoverArrow == arrow::Increase) {
        Value += *Step;
    } else if (_hoverArrow == arrow::Decrease) {
        Value -= *Step;
    }

    ev.Handled = true;
    _mouseDown = true;
    _holdTime.restart();
}

void spinner::on_mouse_button_up(input::mouse::button_event const& ev)
{
    if (ev.Button != controls().PrimaryMouseButton) { return; }

    _mouseDown = false;
    _holdCount = 1;
}

void spinner::on_mouse_wheel(input::mouse::wheel_event const& ev)
{
    if (_editing) { return; }
    if (!is_focused()) { return; }

    if (ev.Scroll.Y > 0) {
        Value += *Step;
    } else if (ev.Scroll.Y < 0) {
        Value -= *Step;
    }

    ev.Handled = true;
}

void spinner::on_key_down(input::keyboard::event const& ev)
{
    auto const& controls {form().Controls};

    if (_editing) {
        if (ev.KeyCode == controls->SubmitKey) {
            commit_edit();
            ev.Handled = true;
            return;
        }
        if (ev.ScanCode == input::scan_code::ESCAPE) {
            cancel_edit();
            ev.Handled = true;
            return;
        }
        _edit.key_down(ev, controls, false);
        ev.Handled = true;
        return;
    }

    // enter edit mode on backspace
    if (ev.KeyCode == controls->BackwardDeleteKey) {
        enter_edit_mode();
        _edit.delete_backward();
        ev.Handled = true;
        return;
    }

    if (ev.Keyboard->is_key_down(controls->ActivateKey)) {
        if (ev.KeyCode == controls->NavDownKey) {
            Value -= *Step;
            ev.Handled = true;
        } else if (ev.KeyCode == controls->NavUpKey) {
            Value += *Step;
            ev.Handled = true;
        }
    }
}

void spinner::on_key_up(input::keyboard::event const& ev)
{
    if (_editing) {
        _edit.key_up(ev, form().Controls);
        ev.Handled = true;
    }
}

void spinner::on_text_input(input::keyboard::text_input_event const& ev)
{
    if (!_editing) { enter_edit_mode(); } // clear, user is typing fresh

    bool const valid {std::ranges::all_of(ev.Text, [](char c) {
        return std::isdigit(c) || c == '-' || c == '.';
    })};
    if (valid) {
        _edit.insert_text(ev.Text);
    }
    ev.Handled = true;
}

void spinner::on_controller_button_down(input::controller::button_event const& ev)
{
    auto const& controls {form().Controls};
    if (ev.Controller->is_button_pressed(controls->ActivateButton)) {
        if (ev.Button == controls->NavDownButton) {
            Value -= *Step;
            ev.Handled = true;
        } else if (ev.Button == controls->NavUpButton) {
            Value += *Step;
            ev.Handled = true;
        }
    }
}

void spinner::on_update(milliseconds deltaTime)
{
    if (_editing) {
        _edit.update(deltaTime);
        return;
    }
    if (_mouseDown && _holdTime.elapsed_milliseconds() > (250.0f / _holdCount)) {
        if (_hoverArrow == arrow::Increase) {
            Value += *Step;
        } else if (_hoverArrow == arrow::Decrease) {
            Value -= *Step;
        }
        _holdTime.restart();
        _holdCount += 0.1f;
    }
}

void spinner::on_focus_lost()
{
    if (_editing) { commit_edit(); }
}

auto spinner::attributes() const -> widget_attributes
{
    auto retValue {widget::attributes()};

    retValue["min"]       = *Min;
    retValue["max"]       = *Max;
    retValue["value"]     = *Value;
    retValue["step"]      = *Step;
    retValue["precision"] = *Precision;

    return retValue;
}

}
