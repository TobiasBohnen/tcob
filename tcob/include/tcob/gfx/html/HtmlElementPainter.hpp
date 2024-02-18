// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#if defined(TCOB_ENABLE_ADDON_GFX_LITEHTML)

    #include <optional>

    #include "tcob/core/AngleUnits.hpp"
    #include "tcob/core/Color.hpp"
    #include "tcob/core/Interfaces.hpp"
    #include "tcob/core/Rect.hpp"
    #include "tcob/core/Size.hpp"
    #include "tcob/gfx/Canvas.hpp"

namespace tcob::gfx::html {
////////////////////////////////////////////////////////////

struct linear_gradient {
    degree_f       Angle;
    color_gradient Colors;
};

////////////////////////////////////////////////////////////

enum class background_repeat {
    Repeat,
    RepeatX,
    RepeatY,
    NoRepeat
};

////////////////////////////////////////////////////////////

enum class list_marker_type {
    Image,
    Circle,
    Disc,
    Square
};

////////////////////////////////////////////////////////////

enum class font_decorations {
    None        = 0x00,
    Underline   = 0x01,
    Linethrough = 0x02,
    Overline    = 0x04
};

////////////////////////////////////////////////////////////

enum class border_style {
    None,
    Hidden,
    Dotted,
    Dashed,
    Solid,
    Double,
    Groove,
    Ridge,
    Inset,
    Outset
};

struct border_radii {
    f32 TopLeft {0};
    f32 TopRight {0};
    f32 BottomRight {0};
    f32 BottomLeft {0};
};

struct border {
    f32          Width {0};
    border_style Style {};
    color        Color {};
};

struct borders {
    border       Left {};
    border       Right {};
    border       Top {};
    border       Bottom {};
    border_radii BorderRadii {};
    rect_f       DrawBox {};
};

class TCOB_API margins {
public:
    auto width() const -> i32;
    auto height() const -> i32;

    i32 Left {0};
    i32 Right {0};
    i32 Top {0};
    i32 Bottom {0};
};

auto constexpr operator==(margins const& left, margins const& right) -> bool
{
    return (left.Left == right.Left) && (left.Top == right.Top)
        && (left.Right == right.Right) && (left.Bottom == right.Bottom);
}

////////////////////////////////////////////////////////////

struct background_draw_context {
    rect_f                         ClipBox {};
    rect_f                         OriginBox {};
    texture*                       Image {nullptr};
    size_f                         ImageSize {};
    std::optional<linear_gradient> Gradient;
    background_repeat              Repeat {};
    color                          BackgroundColor {};
    border_radii                   BorderRadii {};
};

struct text_draw_context {
    string           Text;
    rect_f           TextBox {};
    font*            Font {nullptr};
    color            TextColor {};
    font_decorations FontDecorations {};
};

struct list_marker_draw_context {
    texture*         Image {nullptr};
    list_marker_type Type {};
    color            Color;
    rect_f           Box;
    i32              Index {0};
};

////////////////////////////////////////////////////////////

class TCOB_API element_painter : public non_copyable {
public:
    explicit element_painter(canvas& c);
    virtual ~element_painter() = default;

    void virtual draw_background(background_draw_context const& ctx);
    void virtual draw_borders(borders const& brds);
    void virtual draw_text(text_draw_context const& ctx);
    void virtual draw_list_marker(list_marker_draw_context const& ctx);

protected:
    auto get_canvas() -> canvas&;

private:
    void draw_left_border(borders const& brds);
    void draw_top_border(borders const& brds);
    void draw_right_border(borders const& brds);
    void draw_bottom_border(borders const& brds);

    canvas& _canvas;
};

////////////////////////////////////////////////////////////
}

#endif
