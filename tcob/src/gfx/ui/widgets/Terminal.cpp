// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/Terminal.hpp"

#include "tcob/core/StringUtils.hpp"
#include "tcob/gfx/ui/Form.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"

#include <charconv>

namespace tcob::gfx::ui {

////////////////////////////////////////////////////////////

terminal::terminal(init const& wi)
    : widget {wi}
{
    Size.Changed.connect([&](auto const&) { clear_buffer(); });

    Class("terminal");
}

void terminal::clear()
{
    clear_buffer();
}

void terminal::move(point_i pos)
{
    if (_currentCursor != pos) {
        _currentCursor = pos;
        if (_cursorVisible) {
            force_redraw(get_name() + ": cursor moved");
        }
    }
}

auto terminal::get_xy() const -> point_i
{
    return _currentCursor;
}

void terminal::curs_set(bool visible)
{
    if (_showCursor != visible) {
        _showCursor = visible;
        force_redraw(get_name() + ": cursor visibility changed");

        if (!visible) {
            _cursorTween = nullptr;
        }
    }
}

void terminal::mouse_set(bool mouse)
{
    _useMouse = mouse;
}

void terminal::color_set(color foreground, color background)
{
    _currentColors = {foreground, background};
}

void terminal::add_str(utf8_string_view string)
{
    set(string, false);
}

void terminal::add_str(point_i pos, utf8_string_view string)
{
    move(pos);
    add_str(string);
}

void terminal::add_str(point_i pos, utf8_string_view string, color foreground, color background)
{
    color_set(foreground, background);
    add_str(pos, string);
}

void terminal::ins_str(utf8_string_view string)
{
    set(string, true);
}

void terminal::ins_str(point_i pos, utf8_string_view string)
{
    move(pos);
    ins_str(string);
}

void terminal::ins_str(point_i pos, utf8_string_view string, color foreground, color background)
{
    color_set(foreground, background);
    ins_str(pos, string);
}

void terminal::insert_ln()
{
    i32 const offset {get_offset({0, _currentCursor.Y})};
    if (offset >= _bufferSize) {
        return;
    }

    insert_buffer_at(offset, Size->Width);

    force_redraw(get_name() + ": line inserted");
}

void terminal::delete_ln()
{
    i32 const offset {get_offset({0, _currentCursor.Y})};
    if (offset >= _bufferSize) {
        return;
    }

    erase_buffer_at(offset, Size->Width);

    force_redraw(get_name() + ": line deleted");
}

void terminal::echo(bool insertMode)
{
    _echoInsertMode = insertMode;
    _echoKeys       = true;
}

void terminal::noecho()
{
    _echoKeys = false;
}

void terminal::flash()
{
    using namespace tcob::tweening;

    if (auto const style {get_style<terminal::style>()}) {
        _flashTween = make_unique_tween<square_wave_tween<bool>>(style->FlashDuration, 1.0f, 0.0f);
        _flashTween->Value.Changed.connect([&](auto) {
            for (auto& cell : get_back_buffer()) {
                cell.Colors = {cell.Colors.second, cell.Colors.first};
            }
            force_redraw(get_name() + ": flashing");
        });
        _flashTween->start(playback_style::Normal);
    }
}

void terminal::rectangle(rect_i const& rect)
{
    rectangle(rect, {});
}

void terminal::rectangle(rect_i const& rect, border const& b)
{
    hline({rect.X + 1, rect.Y}, b.TopSide, rect.Width - 1);
    hline({rect.X + 1, rect.bottom()}, b.BottomSide, rect.Width - 1);
    vline({rect.X, rect.Y + 1}, b.LeftSide, rect.Height - 1);
    vline({rect.right(), rect.Y + 1}, b.RightSide, rect.Height - 1);

    add_str(rect.top_left(), b.TopLeft);
    add_str(rect.top_right(), b.TopRight);
    add_str(rect.bottom_left(), b.BottomLeft);
    add_str(rect.bottom_right(), b.BottomRight);
}

void terminal::vline(utf8_string_view ch, i32 size)
{
    i32 const x {_currentCursor.X};
    i32 const y {_currentCursor.Y};
    for (i32 i {0}; i < size; ++i) {
        add_str({x, y + i}, ch);
    }
}

void terminal::vline(point_i pos, utf8_string_view ch, i32 size)
{
    move(pos);
    vline(ch, size);
}

void terminal::hline(utf8_string_view ch, i32 size)
{
    for (i32 i {0}; i < size; ++i) {
        add_str(ch);
    }
}

void terminal::hline(point_i pos, utf8_string_view ch, i32 size)
{
    move(pos);
    hline(ch, size);
}

auto terminal::restore(istream& stream) -> bool
{
    std::array<char, 6> sig {};
    stream.read_to<char>(sig);

    if (sig == std::array<char, 6> {'T', 'T', 'E', 'R', 'M', 1}) {
        i32 const width {stream.read<i32>()};
        i32 const height {stream.read<i32>()};
        Size = {width, height};
        for (i32 i {0}; i < width * height; ++i) {
            auto& cell {get_back_buffer()[i]};
            cell.Text = stream.read_string_until('\0');
            color const first {stream.read<color>()};
            color const second {stream.read<color>()};
            cell.Colors = {first, second};
        }

        swap_buffers();
        return true;
    }

    return false;
}

void terminal::dump(ostream& stream) const
{
    stream.write("TTERM");
    stream.write<u8>(1);
    stream.write<i32>(Size->Width);
    stream.write<i32>(Size->Height);
    for (auto const& cell : get_front_buffer()) {
        stream.write(cell.Text);
        stream.write('\0');
        stream.write(cell.Colors.first);
        stream.write(cell.Colors.second);
    }
}

auto static get_font_width(font* font) -> f32
{
    auto qs {font->shape_text(" ", false, true)};
    if (qs.empty()) { return 0; }

    return qs[0].AdvanceX;
}

void terminal::on_paint(widget_painter& painter)
{
    if (Size->Width <= 0 || Size->Height <= 0) {
        return;
    }

    if (auto const style {get_style<terminal::style>()}) {
        swap_buffers();

        rect_f const rect {get_content_bounds()};

        scissor_guard const guard {painter, this};

        u32 const   fontSize {style->Text.calc_font_size(rect)};
        auto* const font {style->Text.Font->get_font(style->Text.Style, fontSize).get_obj()};
        f32 const   fontHeight {font->get_info().LineHeight};
        f32 const   fontWidth {get_font_width(font)};

        auto& canvas {painter.get_canvas()};
        canvas.save();
        canvas.set_shape_antialias(false);

        // cells
        canvas.set_fill_style(DEFAULT_COLORS.second);
        canvas.fill_rect(rect);
        auto const& buffer {get_front_buffer()};
        for (isize i {_bufferSize - 1}; i >= 0; --i) {
            auto const& cell {buffer[i]};

            rect_f const cellRect {
                rect.X + (i % Size->Width) * fontWidth,
                rect.Y + (i / Size->Width) * fontHeight,
                fontWidth,
                fontHeight};

            if (cell.Colors.second != DEFAULT_COLORS.second) {
                canvas.set_fill_style(cell.Colors.second);
                canvas.fill_rect(cellRect);
            }

            if (cell.Text.empty()) { continue; }
            assert(utf8::length(cell.Text) == 1);

            element::text textStyle {style->Text};
            textStyle.Color = cell.Colors.first;
            painter.draw_text(textStyle, cellRect, cell.Text);
        }

        // cursor
        if (_cursorVisible) {
            rect_f        caretRect {rect};
            point_f const offset {_currentCursor.X * fontWidth, _currentCursor.Y * fontHeight};
            caretRect.Height = fontHeight;
            painter.draw_caret(style->Caret, caretRect, offset);
        }

        canvas.restore();
    }
}

void terminal::on_update(milliseconds deltaTime)
{
    if (_cursorTween) {
        _cursorTween->update(deltaTime);
    }
    if (_flashTween) {
        _flashTween->update(deltaTime);
    }
}

void terminal::on_key_down(input::keyboard::event& ev)
{
    if (_cursorTween) {
        _cursorTween->pause();
        _cursorVisible = true;
    }

    auto const& controls {get_form()->Controls};
    if (ev.KeyCode == controls->NavLeftKey) {
        if (_currentCursor.X > 0) {
            move({_currentCursor.X - 1, _currentCursor.Y});
        } else if (_currentCursor.Y > 0) {
            move({Size->Width - 1, _currentCursor.Y - 1});
        }
    } else if (ev.KeyCode == controls->NavRightKey) {
        if (_currentCursor.X < Size->Width - 1) {
            move({_currentCursor.X + 1, _currentCursor.Y});
        } else {
            cursor_line_break();
        }
    } else if (ev.KeyCode == controls->NavUpKey) {
        if (_currentCursor.Y > 0) {
            move({_currentCursor.X, _currentCursor.Y - 1});
        }
    } else if (ev.KeyCode == controls->NavDownKey) {
        if (_currentCursor.Y < Size->Height - 1) {
            move({_currentCursor.X, _currentCursor.Y + 1});
        }
    }

    if (_echoKeys) {
        if (ev.KeyCode == controls->ForwardDeleteKey) {
            i32 const offset {get_offset(_currentCursor)};
            if (offset >= 0 && offset < _bufferSize) {
                if (_echoInsertMode) {
                    erase_buffer_at(offset, 1);
                } else {
                    get_back_buffer()[offset] = {};
                }
                force_redraw(get_name() + ": delete");
            }
        } else if (ev.KeyCode == controls->BackwardDeleteKey) {
            i32 const offset {get_offset(_currentCursor) - 1};
            if (offset >= 0 && offset < _bufferSize) {
                if (_echoInsertMode) {
                    erase_buffer_at(offset, 1);
                } else {
                    get_back_buffer()[offset] = {};
                }
                if (_currentCursor.X > 0) {
                    move({_currentCursor.X - 1, _currentCursor.Y});
                } else if (_currentCursor.Y > 0) {
                    move({Size->Width - 1, _currentCursor.Y - 1});
                }
                force_redraw(get_name() + ": backspace");
            }
        } else if (ev.KeyCode == controls->SubmitKey) {
            if (_echoInsertMode) {
                ins_str("\n");
            } else {
                add_str("\n");
            }
        }
    }

    ev.Handled = true;
}

void terminal::on_key_up(input::keyboard::event& ev)
{
    if (_cursorTween) {
        _cursorTween->resume();
        ev.Handled = true;
    }
}

void terminal::on_text_input(input::keyboard::text_input_event& ev)
{
    if (!_echoKeys) {
        return;
    }

    if (_echoInsertMode) {
        ins_str(ev.Text);
    } else {
        add_str(ev.Text);
    }

    ev.Handled = true;
}

void terminal::on_mouse_leave()
{
    HoveredCell = {-1, -1};
}

void terminal::on_mouse_hover(input::mouse::motion_event& ev)
{
    if (!_useMouse) { return; }

    if (auto const style {get_style<terminal::style>()}) {
        rect_f const rect {get_global_content_bounds()};
        if (rect.contains(ev.Position)) {
            u32 const   fontSize {style->Text.calc_font_size(rect)};
            auto* const font {style->Text.Font->get_font(style->Text.Style, fontSize).get_obj()};
            f32 const   fontWidth {get_font_width(font)};
            f32 const   fontHeight {font->get_info().LineHeight};

            i32 const x {static_cast<i32>(std::floor(((ev.Position.X - rect.X) / fontWidth) + 0.5f))};
            i32 const y {static_cast<i32>((ev.Position.Y - rect.Y) / fontHeight)};
            HoveredCell = {x, y};

            ev.Handled = true;
        }
    }
}

void terminal::on_mouse_down(input::mouse::button_event& ev)
{
    if (!_useMouse) { return; }

    if (ev.Button == get_form()->Controls->PrimaryMouseButton) {
        if (HoveredCell->X >= 0 && HoveredCell->Y >= 0) {
            move(HoveredCell);
            ev.Handled = true;
        }
    }
}

void terminal::on_focus_gained()
{
    if (!_showCursor) { return; }
    using namespace tcob::tweening;

    if (auto const style {get_style<terminal::style>()}) {
        _cursorTween = make_unique_tween<square_wave_tween<bool>>(style->Caret.BlinkRate * 2, 1.0f, 0.0f);
        _cursorTween->Value.Changed.connect([&](auto val) {
            _cursorVisible = val;
            force_redraw(get_name() + ": cursor blink");
        });
        _cursorTween->start(playback_style::Looped);
    }
}

void terminal::on_focus_lost()
{
    if (!_showCursor) { return; }

    _cursorTween   = nullptr;
    _cursorVisible = false;
}

void terminal::insert_buffer_at(i32 offset, i32 width)
{
    i32 const insertStart {std::clamp<i32>(offset, 0, _bufferSize - 1)};
    i32 const insertEnd {std::clamp<i32>(offset + width, 0, _bufferSize - 1)};
    width = insertEnd - insertStart;
    i32 const eraseStart {std::clamp<i32>(_bufferSize - width, 0, _bufferSize - 1)};
    if (width > 0) {
        auto& buffer {get_back_buffer()};
        buffer.erase(buffer.begin() + eraseStart, buffer.end()); // erase at end
        buffer.insert(buffer.begin() + insertStart, width, {});  // insert at offset
        assert(std::ssize(buffer) == _bufferSize);
    }
}

void terminal::erase_buffer_at(i32 offset, i32 width)
{
    i32 const eraseStart {std::clamp<i32>(offset, 0, _bufferSize - 1)};
    i32 const eraseEnd {std::clamp<i32>(offset + width, 0, _bufferSize - 1)};
    width = eraseEnd - eraseStart;
    if (width > 0) {
        auto& buffer {get_back_buffer()};
        buffer.erase(buffer.begin() + eraseStart, buffer.begin() + eraseEnd); // erase at offset
        buffer.insert(buffer.end(), width, {});                               // insert at end
        assert(std::ssize(buffer) == _bufferSize);
    }
}

auto terminal::get_back_buffer() -> std::vector<cell>&
{
    _backbufferDirty = true;
    return _buffers[0];
}

auto terminal::get_front_buffer() const -> std::vector<cell> const&
{
    return _buffers[1];
}

void terminal::swap_buffers()
{
    if (_backbufferDirty) {
        _buffers[1]      = _buffers[0];
        _backbufferDirty = false;
    }
}

void terminal::clear_buffer()
{
    _bufferSize = Size->Width * Size->Height;
    _buffers[0].clear();
    _buffers[0].resize(_bufferSize);
    _buffers[1].clear();
    _buffers[1].resize(_bufferSize);
    force_redraw(get_name() + ": text buffer cleared");
}

struct esc_seq {
    char                Code {};
    std::vector<string> Values;
};

auto static GetESC(utf8_string_view text, i32& i, i32 len) -> esc_seq
{
    esc_seq retValue;
    string  seq {};

    ++i;
    for (; i < len; ++i) {
        char const c {text[i]};
        if (std::isdigit(c) || c == ';') {
            seq += c;
        } else if (c != '[') {
            retValue.Code = c;
            break;
        }
    }

    retValue.Values = helper::split(seq, ';');
    return retValue;
}

void terminal::parse_esc(char code, std::vector<string> const& seq)
{
    auto const toInt {[](i32& valueInt, string const& val) {
        auto [p, ec] {std::from_chars(val.data(), val.data() + val.size(), valueInt)};
        return ec == std::errc {} && p == val.data() + val.size();
    }};

    switch (code) {
        // Color
    case 'm': {
        color foreground, background;
        for (auto const& val : seq) {
            i32 valueInt {0};
            if (toInt(valueInt, val)) {
                foreground = _currentColors.first;
                background = _currentColors.second;

                switch (valueInt) {
                case 0:
                    foreground = colors::White;
                    background = colors::Black;
                    break;

                case 30: foreground = colors::Black; break;
                case 31: foreground = colors::Red; break;
                case 32: foreground = colors::Green; break;
                case 33: foreground = colors::Yellow; break;
                case 34: foreground = colors::Blue; break;
                case 35: foreground = colors::Magenta; break;
                case 36: foreground = colors::Cyan; break;
                case 37: foreground = colors::White; break;

                case 40: background = colors::Black; break;
                case 41: background = colors::Red; break;
                case 42: background = colors::Green; break;
                case 43: background = colors::Yellow; break;
                case 44: background = colors::Blue; break;
                case 45: background = colors::Magenta; break;
                case 46: background = colors::Cyan; break;
                case 47: background = colors::White; break;

                case 90: foreground = colors::Gray; break;         // Bright Black
                case 91: foreground = colors::Salmon; break;       // Bright Red
                case 92: foreground = colors::LightGreen; break;   // Bright Green
                case 93: foreground = colors::LightYellow; break;  // Bright Yellow
                case 94: foreground = colors::LightBlue; break;    // Bright Blue
                case 95: foreground = colors::Violet; break;       // Bright Magenta
                case 96: foreground = colors::LightCyan; break;    // Bright Cyan
                case 97: foreground = colors::White; break;        // Bright White

                case 100: background = colors::Gray; break;        // Bright Black
                case 101: background = colors::Salmon; break;      // Bright Red
                case 102: background = colors::LightGreen; break;  // Bright Green
                case 103: background = colors::LightYellow; break; // Bright Yellow
                case 104: background = colors::LightBlue; break;   // Bright Blue
                case 105: background = colors::Violet; break;      // Bright Magenta
                case 106: background = colors::LightCyan; break;   // Bright Cyan
                case 107: background = colors::White; break;       // Bright White
                }

                color_set(foreground, background);
            }
        }
    } break;
        // Cursor
    case 'A': {
        i32 line {0};
        if (seq.size() == 1 && toInt(line, seq[0])) {
            move({_currentCursor.X, _currentCursor.Y - line});
        }
    } break;
    case 'B': {
        i32 line {0};
        if (seq.size() == 1 && toInt(line, seq[0])) {
            move({_currentCursor.X, _currentCursor.Y + line});
        }
    } break;
    case 'C': {
        i32 col {0};
        if (seq.size() == 1 && toInt(col, seq[0])) {
            move({_currentCursor.X - col, _currentCursor.Y});
        }
    } break;
    case 'D': {
        i32 col {0};
        if (seq.size() == 1 && toInt(col, seq[0])) {
            move({_currentCursor.X + col, _currentCursor.Y});
        }
    } break;
    case 'E': {
        i32 line {0};
        if (seq.size() == 1 && toInt(line, seq[0])) {
            move({0, _currentCursor.Y + line});
        }
    } break;
    case 'F': {
        i32 line {0};
        if (seq.size() == 1 && toInt(line, seq[0])) {
            move({0, _currentCursor.Y - line});
        }
    } break;
    case 'G': {
        i32 col {0};
        if (seq.size() == 1 && toInt(col, seq[0])) {
            move({col, _currentCursor.Y});
        }
    } break;
    case 'H':
    case 'f': {
        if (code == 'H' && seq.empty()) {
            move({0, 0});
        } else if (seq.size() == 2) {
            i32 line {0}, column {0};
            if (toInt(line, seq[0]) && toInt(column, seq[1])) {
                move({column, line});
            }
        }
        break;
    }
    case 'J': {
        auto const mode {seq.empty() ? "0" : seq[0]};
        auto&      buffer {get_back_buffer()};
        if (mode == "0") {
            i32 offset {std::max(0, get_offset(_currentCursor))};
            while (offset < _bufferSize) {
                buffer[offset++] = {};
            }
        } else if (mode == "1") {
            i32 offset {std::max(0, get_offset(_currentCursor))};
            while (offset >= 0) {
                buffer[offset--] = {};
            }
        } else if (mode == "2") {
            clear_buffer();
        }
        break;
    }
    case 'K': {
        auto const mode {seq.empty() ? "0" : seq[0]};
        auto&      buffer {get_back_buffer()};
        if (mode == "0") {
            i32       start {std::max(0, get_offset(_currentCursor))};
            i32 const end {std::max(0, get_offset({0, _currentCursor.Y + 1}))};
            while (start < end) {
                buffer[start++] = {};
            }
        } else if (mode == "1") {
            i32       start {std::max(0, get_offset({0, _currentCursor.Y}))};
            i32 const end {std::max(0, get_offset(_currentCursor))};
            while (start < end) {
                buffer[start++] = {};
            }
        } else if (mode == "2") {
            i32       start {std::max(0, get_offset({0, _currentCursor.Y}))};
            i32 const end {std::max(0, get_offset({0, _currentCursor.Y + 1}))};
            while (start < end) {
                buffer[start++] = {};
            }
        }
        break;
    }
    }
}

void terminal::set(utf8_string_view text, bool insert)
{
    i32 const len {static_cast<i32>(utf8::length(text))};

    for (i32 i {0}; i < len; ++i) {
        i32 const offset {get_offset(_currentCursor)};

        if (offset >= 0 && offset < _bufferSize) {
            auto const cellText {utf8::substr(text, i)};
            if (cellText == "\033") {
                auto const seq {GetESC(text, i, len)};
                parse_esc(seq.Code, seq.Values);
            } else if (cellText == "\n") {
                if (insert) {
                    insert_buffer_at(offset, Size->Width);
                    if (_currentCursor.Y < Size->Height - 1) {
                        _currentCursor.Y++;
                    }
                } else {
                    cursor_line_break();
                }
            } else {
                if (insert) {
                    insert_buffer_at(offset, 1);
                }
                auto& cell {get_back_buffer()[offset]};
                cell.Text   = cellText;
                cell.Colors = _currentColors;
                _currentCursor.X++;
                if (_currentCursor.X >= Size->Width) {
                    cursor_line_break();
                }
            }
        }
    }

    force_redraw(get_name() + ": text set");
}

void terminal::cursor_line_break()
{
    if (_currentCursor.Y < Size->Height - 1) {
        _currentCursor.X = 0;
        _currentCursor.Y++;
    } else {
        _currentCursor.X = Size->Width - 1;
    }
}

auto terminal::get_offset(point_i p) const -> i32
{
    return p.X + p.Y * Size->Width;
}
}
