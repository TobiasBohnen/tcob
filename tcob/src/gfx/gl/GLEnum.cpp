// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/gfx/gl/GLEnum.hpp>

#include <glad/gl.h>

auto tcob::gl::enumToGL(tcob::gl::BufferUsage usage) -> u32
{
    switch (usage) {
    case tcob::gl::BufferUsage::DynamicDraw:
        return GL_DYNAMIC_DRAW;
    case tcob::gl::BufferUsage::StaticDraw:
        return GL_STATIC_DRAW;
    case tcob::gl::BufferUsage::StreamDraw:
        return GL_STREAM_DRAW;
    default:
        return GL_INVALID_ENUM;
    }
}

auto tcob::gl::enumToGL(tcob::gl::BlendFunc blendfunc) -> u32
{
    switch (blendfunc) {
    case tcob::gl::BlendFunc::Zero:
        return GL_ZERO;
    case tcob::gl::BlendFunc::One:
        return GL_ONE;
    case tcob::gl::BlendFunc::SrcColor:
        return GL_SRC_COLOR;
    case tcob::gl::BlendFunc::OneMinusScrColor:
        return GL_ONE_MINUS_SRC_COLOR;
    case tcob::gl::BlendFunc::DstColor:
        return GL_DST_COLOR;
    case tcob::gl::BlendFunc::OneMinusDstColor:
        return GL_ONE_MINUS_DST_COLOR;
    case tcob::gl::BlendFunc::SrcAlpha:
        return GL_SRC_ALPHA;
    case tcob::gl::BlendFunc::OneMinusSrcAlpha:
        return GL_ONE_MINUS_SRC_ALPHA;
    case tcob::gl::BlendFunc::DstAlpha:
        return GL_DST_ALPHA;
    case tcob::gl::BlendFunc::OneMinusDstAlpha:
        return GL_ONE_MINUS_DST_ALPHA;
    case tcob::gl::BlendFunc::ConstantColor:
        return GL_CONSTANT_COLOR;
    case tcob::gl::BlendFunc::OneMinusConstantColor:
        return GL_ONE_MINUS_CONSTANT_COLOR;
    case tcob::gl::BlendFunc::ConstantAlpha:
        return GL_CONSTANT_ALPHA;
    case tcob::gl::BlendFunc::OneMinusConstantAlpha:
        return GL_ONE_MINUS_CONSTANT_ALPHA;
    default:
        return GL_INVALID_ENUM;
    }
}

auto tcob::gl::enumToGL(tcob::gl::BlendEquation equ) -> u32
{
    switch (equ) {
    case tcob::gl::BlendEquation::Add:
        return GL_FUNC_ADD;
    case tcob::gl::BlendEquation::Subtract:
        return GL_FUNC_SUBTRACT;
    case tcob::gl::BlendEquation::ReverseSubtract:
        return GL_FUNC_REVERSE_SUBTRACT;
    case tcob::gl::BlendEquation::Min:
        return GL_MIN;
    case tcob::gl::BlendEquation::Max:
        return GL_MAX;
    default:
        return GL_INVALID_ENUM;
    }
}