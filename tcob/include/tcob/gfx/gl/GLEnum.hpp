// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

namespace tcob::gl {
enum class BufferUsage : u8 {
    DynamicDraw,
    StaticDraw,
    StreamDraw
};

auto convert_enum(tcob::gl::BufferUsage usage) -> u32;

enum class BlendFunc : u8 {
    Zero,
    One,
    SrcColor,
    OneMinusScrColor,
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

auto convert_enum(tcob::gl::BlendFunc blendfunc) -> u32;

struct BlendFuncs {
    gl::BlendFunc SourceColorBlendFunc { gl::BlendFunc::SrcAlpha };
    gl::BlendFunc DestinationColorBlendFunc { gl::BlendFunc::OneMinusSrcAlpha };
    gl::BlendFunc SourceAlphaBlendFunc { gl::BlendFunc::SrcAlpha };
    gl::BlendFunc DestinationAlphaBlendFunc { gl::BlendFunc::OneMinusSrcAlpha };
};

inline auto operator==(const BlendFuncs& left, const BlendFuncs& right) -> bool
{
    return (left.SourceColorBlendFunc == right.SourceColorBlendFunc)
        && (left.DestinationColorBlendFunc == right.DestinationColorBlendFunc)
        && (left.SourceAlphaBlendFunc == right.SourceAlphaBlendFunc)
        && (left.DestinationAlphaBlendFunc == right.DestinationAlphaBlendFunc);
}

enum class BlendEquation : u8 {
    Add,
    Subtract,
    ReverseSubtract,
    Min,
    Max
};

auto convert_enum(tcob::gl::BlendEquation equ) -> u32;
}