// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/core/Rect.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

enum class buffer_usage_hint : u8 {
    DynamicDraw,
    StaticDraw,
    StreamDraw
};

enum class blend_func : u8 {
    Invalid,
    Zero,
    One,
    SrcColor,
    OneMinusSrcColor,
    DstColor,
    OneMinusDstColor,
    SrcAlpha,
    OneMinusSrcAlpha,
    DstAlpha,
    OneMinusDstAlpha,
    ConstantColor,
    OneMinusConstantColor,
    ConstantAlpha,
    OneMinusConstantAlpha
};

struct blend_funcs {
    blend_func SourceColorBlendFunc {blend_func::SrcAlpha};
    blend_func DestinationColorBlendFunc {blend_func::OneMinusSrcAlpha};
    blend_func SourceAlphaBlendFunc {blend_func::SrcAlpha};
    blend_func DestinationAlphaBlendFunc {blend_func::OneMinusSrcAlpha};
};

inline auto operator==(blend_funcs const& left, blend_funcs const& right) -> bool
{
    return (left.SourceColorBlendFunc == right.SourceColorBlendFunc)
        && (left.DestinationColorBlendFunc == right.DestinationColorBlendFunc)
        && (left.SourceAlphaBlendFunc == right.SourceAlphaBlendFunc)
        && (left.DestinationAlphaBlendFunc == right.DestinationAlphaBlendFunc);
}

enum class blend_equation : u8 {
    Add,
    Subtract,
    ReverseSubtract,
    Min,
    Max
};

enum class primitive_type : u8 {
    Points,
    LineStrip,
    LineLoop,
    Lines,
    TriangleStrip,
    TriangleFan,
    Triangles
};

////////////////////////////////////////////////////////////

class texture_region final {
public:
    rect_f UVRect {rect_f::Zero};
    u32    Level {0};

    void static Serialize(texture_region const& v, auto&& s);
    auto static Deserialize(texture_region& v, auto&& s) -> bool;
};

inline void texture_region::Serialize(texture_region const& v, auto&& s)
{
    s["level"] = v.Level;
    rect_f::Serialize(v.UVRect, s);
}

inline auto texture_region::Deserialize(texture_region& v, auto&& s) -> bool
{
    return s.try_get(v.Level, "level") && rect_f::Deserialize(v.UVRect, s);
}

////////////////////////////////////////////////////////////

enum class orientation : u8 {
    Horizontal,
    Vertical
};

enum class horizontal_alignment : u8 {
    Left,
    Right,
    Centered
};

enum class vertical_alignment : u8 {
    Top,
    Middle,
    Bottom
};

////////////////////////////////////////////////////////////

struct alignments {
    horizontal_alignment Horizontal {horizontal_alignment::Left};
    vertical_alignment   Vertical {vertical_alignment::Top};

    void static Serialize(alignments const& v, auto&& s);
    auto static Deserialize(alignments& v, auto&& s) -> bool;
};

inline void alignments::Serialize(alignments const& v, auto&& s)
{
    s["horizontal"] = v.Horizontal;
    s["vertical"]   = v.Vertical;
}

inline auto alignments::Deserialize(alignments& v, auto&& s) -> bool
{
    return s.try_get(v.Horizontal, "horizontal") && s.try_get(v.Vertical, "vertical");
}

inline auto operator==(alignments const& left, alignments const& right) -> bool
{
    return (left.Horizontal == right.Horizontal)
        && (left.Vertical == right.Vertical);
}

}
