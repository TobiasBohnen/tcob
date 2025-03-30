// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <bitset>
#include <span>
#include <unordered_map>
#include <vector>

#include "tcob/core/AngleUnits.hpp"
#include "tcob/core/Color.hpp"
#include "tcob/core/Grid.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Property.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/ColorGradient.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui {

////////////////////////////////////////////////////////////

class TCOB_API dot_matrix_display : public widget {
public:
    enum class dot_type : u8 {
        Disc,
        Square
    };

    class TCOB_API style : public widget_style {
    public:
        std::unordered_map<u8, color> Colors;
        dot_type                      Type {dot_type::Disc};

        void static Transition(style& target, style const& left, style const& right, f64 step);
    };

    explicit dot_matrix_display(init const& wi);

    prop<grid<u8> const> Dots;

protected:
    void on_draw(widget_painter& painter) override;

    void on_update(milliseconds deltaTime) override;

private:
    bool                                         _isDirty {false};
    std::unordered_map<u8, std::vector<point_i>> _sortedDots;

    dot_matrix_display::style _style;
};

////////////////////////////////////////////////////////////

class TCOB_API seven_segment_display : public widget {
public:
    struct segment {
        bool A {false};
        bool B {false};
        bool C {false};
        bool D {false};
        bool E {false};
        bool F {false};
        bool G {false};
    };

    struct style : public widget_style {
        length Size;
        color  ActiveColor {colors::Black};
        color  InactiveColor {colors::Transparent};

        void static Transition(style& target, style const& left, style const& right, f64 step);
    };

    explicit seven_segment_display(init const& wi);

    void draw_text(string const& text);
    void draw_segments(std::span<segment const> segments);

protected:
    void on_draw(widget_painter& painter) override;

    void on_update(milliseconds deltaTime) override;

private:
    auto get_segment(char c) -> std::bitset<7>;
    auto get_segment(segment segments) -> std::bitset<7>;

    seven_segment_display::style _style;
    std::vector<std::bitset<7>>  _segments;
};

////////////////////////////////////////////////////////////

class TCOB_API color_picker : public widget {
public:
    explicit color_picker(init const& wi);

    prop<color>    SelectedColor;
    prop<degree_f> BaseHue {degree_f {0}};

protected:
    void on_draw(widget_painter& painter) override;

    void on_mouse_button_down(input::mouse::button_event const& ev) override;

    void on_update(milliseconds deltaTime) override;

private:
    auto static GetGradient() -> gfx::color_gradient const&;

    point_f _selectedColorPos {point_f {-1, -1}};
    f32     _selectedHuePos {-1};
};

}
