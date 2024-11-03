// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <vector>

#include "tcob/core/Size.hpp"
#include "tcob/gfx/animation/Tween.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::gfx::ui {
////////////////////////////////////////////////////////////

class TCOB_API terminal : public widget {
    static constexpr std::pair<color, color> DEFAULT_COLORS {std::pair {colors::White, colors::Black}};

public:
    class TCOB_API style : public ui::style {
    public:
        element::text  Text;
        element::caret Caret;
        milliseconds   FlashDuration {500};
    };
    struct border {
        utf8_string LeftSide {"│"};
        utf8_string RightSide {"│"};
        utf8_string TopSide {"─"};
        utf8_string BottomSide {"─"};
        utf8_string TopLeft {"┌"};
        utf8_string TopRight {"┐"};
        utf8_string BottomLeft {"└"};
        utf8_string BottomRight {"┘"};
    };

    explicit terminal(init const& wi);

    prop<size_i>  Size;
    prop<point_i> HoveredCell;

    void clear();

    void move(point_i pos);
    auto get_xy() const -> point_i;
    void curs_set(bool visible);
    void mouse_set(bool mouse);

    void color_set(color foreground, color background = colors::Transparent);

    void add_str(utf8_string_view string);
    void add_str(point_i pos, utf8_string_view string);
    void add_str(point_i pos, utf8_string_view string, color foreground, color background = colors::Transparent);

    void ins_str(utf8_string_view string);
    void ins_str(point_i pos, utf8_string_view string);
    void ins_str(point_i pos, utf8_string_view string, color foreground, color background = colors::Transparent);

    void insert_ln();
    void delete_ln();

    void echo(bool insertMode = false);
    void noecho();

    void flash();

    void rectangle(rect_i const& rect);
    void rectangle(rect_i const& rect, border const& b);
    void vline(utf8_string_view ch, i32 size);
    void vline(point_i pos, utf8_string_view ch, i32 size);
    void hline(utf8_string_view ch, i32 size);
    void hline(point_i pos, utf8_string_view ch, i32 size);

    auto restore [[nodiscard]] (io::istream& stream) -> bool;
    void dump(io::ostream& stream) const;

protected:
    void on_paint(widget_painter& painter) override;

    void on_update(milliseconds deltaTime) override;

    void on_key_down(input::keyboard::event const& ev) override;
    void on_key_up(input::keyboard::event const& ev) override;

    void on_text_input(input::keyboard::text_input_event const& ev) override;

    void on_mouse_leave() override;
    void on_mouse_hover(input::mouse::motion_event const& ev) override;
    void on_mouse_down(input::mouse::button_event const& ev) override;

    void on_focus_gained() override;
    void on_focus_lost() override;

private:
    struct cell {
        utf8_string             Text;
        std::pair<color, color> Colors {DEFAULT_COLORS};
    };

    void insert_buffer_at(i32 offset, i32 width);
    void erase_buffer_at(i32 offset, i32 width);

    auto get_front_buffer() const -> std::vector<cell> const&;
    auto get_back_buffer() -> std::vector<cell>&;

    void swap_buffers();
    void clear_buffer();
    auto get_offset(point_i p) const -> i32;

    void set(utf8_string_view text, bool insert);
    void cursor_line_break();

    void parse_esc(char code, std::vector<string> const& seq);

    point_i                 _currentCursor {point_u::Zero};
    std::pair<color, color> _currentColors {DEFAULT_COLORS};

    bool _cursorVisible {false};
    bool _showCursor {false};
    bool _useMouse {false};
    bool _echoKeys {false};
    bool _echoInsertMode {false};
    bool _backbufferDirty {false};

    std::array<std::vector<cell>, 2> _buffers {};
    i32                              _bufferSize {0};

    std::unique_ptr<square_wave_tween<bool>> _cursorTween;
    std::unique_ptr<square_wave_tween<bool>> _flashTween;
};

}
