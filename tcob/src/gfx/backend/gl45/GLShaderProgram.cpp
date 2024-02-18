// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "GLShaderProgram.hpp"

#include <cassert>

#include <glad/gl45.h>

#include "tcob/core/Logger.hpp"

namespace tcob::gfx::gl45 {
gl_shader::gl_shader()
{
    ID = glCreateProgram();
}

gl_shader::~gl_shader()
{
    destroy();
}

auto gl_shader::compile(string const& vertexShaderSource, string const& fragmentShaderSource) -> bool
{
    if (vertexShaderSource.empty() || fragmentShaderSource.empty()) {
        return false;
    }

    GLint success {};
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (success) { // already linked -> destroy and recreate
        do_destroy();
        ID = glCreateProgram();
    }

    char const* cstr {nullptr};

    // Build compile VERTEX_SHADER
    GLuint const vertexShader {glCreateShader(GL_VERTEX_SHADER)};
    cstr = vertexShaderSource.c_str();
    glShaderSource(vertexShader, 1, &cstr, nullptr);
    glCompileShader(vertexShader);

    // Check for compile errors VERTEX_SHADER
    std::array<GLchar, 512> infoLog {};
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, sizeof(infoLog), nullptr, infoLog.data());
        logger::Error("Shader: vertex shader compilation error: {}", infoLog.data());
        return false;
    }

    // Build compile FRAGMENT_SHADER
    GLuint const fragmentShader {glCreateShader(GL_FRAGMENT_SHADER)};
    cstr = fragmentShaderSource.c_str();
    glShaderSource(fragmentShader, 1, &cstr, nullptr);
    glCompileShader(fragmentShader);

    // Check for compile errors FRAGMENT_SHADER
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, sizeof(infoLog), nullptr, infoLog.data());
        logger::Error("Shader: fragment shader compilation error: {}", infoLog.data());
        return false;
    }

    // Link shaders
    glAttachShader(ID, vertexShader);
    glAttachShader(ID, fragmentShader);
    glLinkProgram(ID);

    // Check the linking errors
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(ID, sizeof(infoLog), nullptr, infoLog.data());
        logger::Error("Shader: linking error: {}", infoLog.data());
        return false;
    }

    logger::Debug("Shader: linked ID {}", ID);

    // Delete shaders
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return true;
}

auto gl_shader::get_uniform_block_binding(string const& name) const -> i32
{
    assert(ID);
    GLint i {};
    glGetActiveUniformBlockiv(ID, glGetUniformBlockIndex(ID, name.c_str()), GL_UNIFORM_BLOCK_BINDING, &i);
    return i;
}

void gl_shader::do_destroy()
{
    assert(ID);
    glDeleteProgram(ID);
}

void gl_shader::set_uniform(i32 loc, i32 x) const
{
    assert(ID);
    glProgramUniform1i(ID, loc, x);
}

void gl_shader::set_uniform(i32 loc, u32 x) const
{
    assert(ID);
    glProgramUniform1ui(ID, loc, x);
}

void gl_shader::set_uniform(i32 loc, f32 x) const
{
    assert(ID);
    glProgramUniform1f(ID, loc, x);
}

void gl_shader::set_uniform(i32 loc, ivec2 const& x) const
{
    assert(ID);
    glProgramUniform2i(ID, loc, x[0], x[1]);
}

void gl_shader::set_uniform(i32 loc, uvec2 const& x) const
{
    assert(ID);
    glProgramUniform2ui(ID, loc, x[0], x[1]);
}

void gl_shader::set_uniform(i32 loc, vec2 const& x) const
{
    assert(ID);
    glProgramUniform2f(ID, loc, x[0], x[1]);
}

void gl_shader::set_uniform(i32 loc, ivec3 const& x) const
{
    assert(ID);
    glProgramUniform3i(ID, loc, x[0], x[1], x[2]);
}

void gl_shader::set_uniform(i32 loc, uvec3 const& x) const
{
    assert(ID);
    glProgramUniform3ui(ID, loc, x[0], x[1], x[2]);
}

void gl_shader::set_uniform(i32 loc, vec3 const& x) const
{
    assert(ID);
    glProgramUniform3f(ID, loc, x[0], x[1], x[2]);
}

void gl_shader::set_uniform(i32 loc, ivec4 const& x) const
{
    assert(ID);
    glProgramUniform4i(ID, loc, x[0], x[1], x[2], x[3]);
}

void gl_shader::set_uniform(i32 loc, uvec4 const& x) const
{
    assert(ID);
    glProgramUniform4ui(ID, loc, x[0], x[1], x[2], x[3]);
}

void gl_shader::set_uniform(i32 loc, vec4 const& x) const
{
    assert(ID);
    glProgramUniform4f(ID, loc, x[0], x[1], x[2], x[3]);
}

void gl_shader::set_uniform(i32 loc, size_f x) const
{
    assert(ID);
    glProgramUniform2f(ID, loc, x.Width, x.Height);
}

void gl_shader::set_uniform(i32 loc, point_f x) const
{
    assert(ID);
    glProgramUniform2f(ID, loc, x.X, x.Y);
}

void gl_shader::set_uniform(i32 loc, size_i x) const
{
    assert(ID);
    glProgramUniform2i(ID, loc, x.Width, x.Height);
}

void gl_shader::set_uniform(i32 loc, point_i x) const
{
    assert(ID);
    glProgramUniform2i(ID, loc, x.X, x.Y);
}

void gl_shader::set_uniform(i32 loc, size_u x) const
{
    assert(ID);
    glProgramUniform2ui(ID, loc, x.Width, x.Height);
}

void gl_shader::set_uniform(i32 loc, point_u x) const
{
    assert(ID);
    glProgramUniform2ui(ID, loc, x.X, x.Y);
}

void gl_shader::set_uniform(i32 loc, mat4 const& x) const
{
    assert(ID);
    glProgramUniformMatrix4fv(ID, loc, 1, GL_FALSE, x.data());
}

auto gl_shader::is_valid() const -> bool
{
    return ID != 0;
}

auto gl_shader::get_id() const -> u32
{
    return ID;
}

auto gl_shader::get_uniform_location(string const& name) const -> i32
{
    assert(ID);
    return glGetUniformLocation(ID, name.c_str());
}
}
