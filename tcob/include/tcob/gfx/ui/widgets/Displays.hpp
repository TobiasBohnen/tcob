// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <array>
#include <bitset>
#include <optional>
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

        void static Transition(style& target, style const& from, style const& to, f64 step);
    };

    explicit dot_matrix_display(init const& wi);

    prop<grid<u8>> Dots;

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

        void static Transition(style& target, style const& from, style const& to, f64 step);
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
    struct style : public widget_style {
        length HueSelectWidth {0.1f, length::type::Relative};
        color  MarkerColor {colors::White};
        length MarkerSize {2, length::type::Absolute};

        void static Transition(style& target, style const& from, style const& to, f64 step);
    };

    explicit color_picker(init const& wi);

    prop<hsx> SelectedColor;

protected:
    void on_draw(widget_painter& painter) override;

    void on_mouse_hover(input::mouse::motion_event const& ev) override;
    void on_mouse_drag(input::mouse::motion_event const& ev) override;
    void on_mouse_button_down(input::mouse::button_event const& ev) override;

    void on_update(milliseconds deltaTime) override;

private:
    auto hover_color(point_i mp) -> bool;
    auto select_color() -> bool;

    auto static GetGradient() -> gfx::color_gradient const&;
    auto static GetColors() -> std::array<color, 256> const&;

    std::optional<hsx>      _hoveredColor;
    std::optional<degree_f> _hoveredBaseHue;

    color_picker::style _style;
};

}
