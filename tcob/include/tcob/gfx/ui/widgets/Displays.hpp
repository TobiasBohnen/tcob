// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <bitset>
#include <vector>

#include "tcob/core/FlatMap.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::gfx::ui {

////////////////////////////////////////////////////////////

class TCOB_API dot_matrix_display : public widget {
public:
    struct dot {
        enum class type {
            Disc,
            Square
        };

        length               Size;
        flat_map<u16, color> Colors;
        type                 Type {type::Disc};
    };

    class TCOB_API style : public background_style {
    public:
        dot Dot;
    };

    explicit dot_matrix_display(init const& wi);

    prop<size_i>           Size;
    prop<std::vector<u16>> Dots;

protected:
    void on_paint(widget_painter& painter) override;

    void on_update(milliseconds deltaTime) override;
};

////////////////////////////////////////////////////////////

class TCOB_API seven_segment_display : public widget {
public:
    struct segment {
        length Size;
        color  ActiveColor {colors::Black};
        color  InactiveColor {colors::Transparent};
    };

    struct style : public background_style {
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
    prop<degree_f> BaseHue;

protected:
    void on_paint(widget_painter& painter) override;

    void on_mouse_down(input::mouse::button_event& ev) override;

    void on_update(milliseconds deltaTime) override;

private:
    auto static GetGradient() -> color_gradient const&;
};

}
