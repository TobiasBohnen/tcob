// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <bitset>
#include <unordered_map>
#include <vector>

#include "tcob/core/Size.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::gfx::ui {

////////////////////////////////////////////////////////////

class TCOB_API dot_matrix_display : public widget {
public:
    struct dot {
        enum class type : u8 {
            Disc,
            Square
        };

        std::unordered_map<u16, color> Colors;
        type                           Type {type::Disc};

        auto operator==(dot const& other) const -> bool = default;
    };

    class TCOB_API style : public widget_style {
    public:
        dot Dot;
    };

    explicit dot_matrix_display(init const& wi);

    prop<size_i>                 Size;
    prop<std::vector<u16> const> Dots;

protected:
    void on_paint(widget_painter& painter) override;

    void on_update(milliseconds deltaTime) override;

private:
    bool                                          _isDirty {false};
    std::unordered_map<u16, std::vector<point_i>> _sortedDots;
};

////////////////////////////////////////////////////////////

class TCOB_API seven_segment_display : public widget {
public:
    struct segment {
        length Size;
        color  ActiveColor {colors::Black};
        color  InactiveColor {colors::Transparent};
    };

    struct style : public widget_style {
        segment Segment;
    };

    explicit seven_segment_display(init const& wi);

    prop<string> Text;

protected:
    void on_paint(widget_painter& painter) override;

    void on_update(milliseconds deltaTime) override;

private:
    auto get_segment(char c) -> std::bitset<7>;
};

////////////////////////////////////////////////////////////

class TCOB_API color_picker : public widget {
public:
    explicit color_picker(init const& wi);

    prop<color>    SelectedColor;
    prop<degree_f> BaseHue {degree_f {0}};

protected:
    void on_paint(widget_painter& painter) override;

    void on_mouse_down(input::mouse::button_event const& ev) override;

    void on_update(milliseconds deltaTime) override;

private:
    auto static GetGradient() -> color_gradient const&;
};

}
