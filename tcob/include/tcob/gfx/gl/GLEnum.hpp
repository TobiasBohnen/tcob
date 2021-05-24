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

auto enumToGL(tcob::gl::BufferUsage usage) -> u32;

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

auto enumToGL(tcob::gl::BlendFunc blendfunc) -> u32;

enum class BlendEquation : u8 {
    Add,
    Subtract,
    ReverseSubtract,
    Min,
    Max
};

auto enumToGL(tcob::gl::BlendEquation equ) -> u32;
}