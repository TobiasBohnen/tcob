// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "GLES30Enum.hpp"

#include <glad/gles30.h>

#include "tcob/gfx/Gfx.hpp"

namespace tcob::gfx::gles30 {

auto convert_enum(buffer_usage_hint usage) -> u32
{
    switch (usage) {
    case buffer_usage_hint::DynamicDraw:
        return GL_DYNAMIC_DRAW;
    case buffer_usage_hint::StaticDraw:
        return GL_STATIC_DRAW;
    case buffer_usage_hint::StreamDraw:
        return GL_STREAM_DRAW;
    }

    return GL_INVALID_ENUM;
}

auto convert_enum(blend_func blendfunc) -> u32
{
    switch (blendfunc) {
    case blend_func::Zero:
        return GL_ZERO;
    case blend_func::One:
        return GL_ONE;
    case blend_func::SrcColor:
        return GL_SRC_COLOR;
    case blend_func::OneMinusSrcColor:
        return GL_ONE_MINUS_SRC_COLOR;
    case blend_func::DstColor:
        return GL_DST_COLOR;
    case blend_func::OneMinusDstColor:
        return GL_ONE_MINUS_DST_COLOR;
    case blend_func::SrcAlpha:
        return GL_SRC_ALPHA;
    case blend_func::OneMinusSrcAlpha:
        return GL_ONE_MINUS_SRC_ALPHA;
    case blend_func::DstAlpha:
        return GL_DST_ALPHA;
    case blend_func::OneMinusDstAlpha:
        return GL_ONE_MINUS_DST_ALPHA;
    case blend_func::ConstantColor:
        return GL_CONSTANT_COLOR;
    case blend_func::OneMinusConstantColor:
        return GL_ONE_MINUS_CONSTANT_COLOR;
    case blend_func::ConstantAlpha:
        return GL_CONSTANT_ALPHA;
    case blend_func::OneMinusConstantAlpha:
        return GL_ONE_MINUS_CONSTANT_ALPHA;
    default:
        return GL_INVALID_ENUM;
    }
}

auto convert_enum(blend_equation equ) -> u32
{
    switch (equ) {
    case blend_equation::Add:
        return GL_FUNC_ADD;
    case blend_equation::Subtract:
        return GL_FUNC_SUBTRACT;
    case blend_equation::ReverseSubtract:
        return GL_FUNC_REVERSE_SUBTRACT;
    case blend_equation::Min:
        return GL_MIN;
    case blend_equation::Max:
        return GL_MAX;
    }

    return GL_INVALID_ENUM;
}

auto convert_enum(primitive_type type) -> u32
{
    switch (type) {
    case primitive_type::Points:
        return GL_POINTS;
    case primitive_type::LineStrip:
        return GL_LINE_STRIP;
    case primitive_type::LineLoop:
        return GL_LINE_LOOP;
    case primitive_type::Lines:
        return GL_LINES;
    case primitive_type::TriangleStrip:
        return GL_TRIANGLE_STRIP;
    case primitive_type::TriangleFan:
        return GL_TRIANGLE_FAN;
    case primitive_type::Triangles:
        return GL_TRIANGLES;
    }

    return GL_INVALID_ENUM;
}

}
