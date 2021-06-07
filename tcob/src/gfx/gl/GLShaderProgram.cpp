// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/gfx/gl/GLShaderProgram.hpp>

#include <cassert>

#include <glad/gl.h>

#include <tcob/core/io/Logger.hpp>

namespace tcob::gl {
ShaderProgram::ShaderProgram()
{
    ID = glCreateProgram();
}

ShaderProgram::~ShaderProgram()
{
    destroy();
}

auto ShaderProgram::create(const char* vertexShaderSource, const char* fragmentShaderSource) -> bool
{
    GLint success {};
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (success) { // already linked -> destroy and recreate
        do_destroy();
        ID = glCreateProgram();
    }

    // Build compile VERTEX_SHADER
    const GLuint vertexShader { glCreateShader(GL_VERTEX_SHADER) };
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    // Check for compile errors VERTEX_SHADER
    std::array<GLchar, 512> infoLog {};
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, sizeof(infoLog), nullptr, infoLog.data());
        Log("vertex shader compilation error: " + std::string(infoLog.data()), LogLevel::Error);
        return false;
    }

    // Build compile FRAGMENT_SHADER
    const GLuint fragmentShader = { glCreateShader(GL_FRAGMENT_SHADER) };
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    // Check for compile errors FRAGMENT_SHADER
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, sizeof(infoLog), nullptr, infoLog.data());
        Log("fragment shader compilation error: " + std::string(infoLog.data()), LogLevel::Error);
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
        Log("linking error: " + std::string(infoLog.data()), LogLevel::Error);
        return false;
    }

    // Delete shaders
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return true;
}

void ShaderProgram::do_destroy()
{
    glDeleteProgram(ID);
}

void ShaderProgram::use() const
{
    assert(ID);
    glUseProgram(ID);
}

void ShaderProgram::set_uniform_impl(i32 loc, i32 x) const
{
    assert(ID);
    glProgramUniform1i(ID, loc, x);
}

void ShaderProgram::set_uniform_impl(i32 loc, u32 x) const
{
    assert(ID);
    glProgramUniform1ui(ID, loc, x);
}

void ShaderProgram::set_uniform_impl(i32 loc, f32 x) const
{
    assert(ID);
    glProgramUniform1f(ID, loc, x);
}

void ShaderProgram::set_uniform_impl(i32 loc, const ivec2& x) const
{
    assert(ID);
    glProgramUniform2i(ID, loc, x[0], x[1]);
}

void ShaderProgram::set_uniform_impl(i32 loc, const uvec2& x) const
{
    assert(ID);
    glProgramUniform2ui(ID, loc, x[0], x[1]);
}

void ShaderProgram::set_uniform_impl(i32 loc, const vec2& x) const
{
    assert(ID);
    glProgramUniform2f(ID, loc, x[0], x[1]);
}

void ShaderProgram::set_uniform_impl(i32 loc, const ivec3& x) const
{
    assert(ID);
    glProgramUniform3i(ID, loc, x[0], x[1], x[2]);
}

void ShaderProgram::set_uniform_impl(i32 loc, const uvec3& x) const
{
    assert(ID);
    glProgramUniform3ui(ID, loc, x[0], x[1], x[2]);
}

void ShaderProgram::set_uniform_impl(i32 loc, const vec3& x) const
{
    assert(ID);
    glProgramUniform3f(ID, loc, x[0], x[1], x[2]);
}

void ShaderProgram::set_uniform_impl(i32 loc, const ivec4& x) const
{
    assert(ID);
    glProgramUniform4i(ID, loc, x[0], x[1], x[2], x[3]);
}

void ShaderProgram::set_uniform_impl(i32 loc, const uvec4& x) const
{
    assert(ID);
    glProgramUniform4ui(ID, loc, x[0], x[1], x[2], x[3]);
}

void ShaderProgram::set_uniform_impl(i32 loc, const vec4& x) const
{
    assert(ID);
    glProgramUniform4f(ID, loc, x[0], x[1], x[2], x[3]);
}

void ShaderProgram::set_uniform_impl(i32 loc, const SizeF& x) const
{
    assert(ID);
    glProgramUniform2f(ID, loc, x.Width, x.Height);
}

void ShaderProgram::set_uniform_impl(i32 loc, const PointF& x) const
{
    assert(ID);
    glProgramUniform2f(ID, loc, x.X, x.Y);
}

void ShaderProgram::set_uniform_impl(i32 loc, const SizeI& x) const
{
    assert(ID);
    glProgramUniform2i(ID, loc, x.Width, x.Height);
}

void ShaderProgram::set_uniform_impl(i32 loc, const PointI& x) const
{
    assert(ID);
    glProgramUniform2i(ID, loc, x.X, x.Y);
}

void ShaderProgram::set_uniform_impl(i32 loc, const SizeU& x) const
{
    assert(ID);
    glProgramUniform2ui(ID, loc, x.Width, x.Height);
}

void ShaderProgram::set_uniform_impl(i32 loc, const PointU& x) const
{
    assert(ID);
    glProgramUniform2ui(ID, loc, x.X, x.Y);
}

void ShaderProgram::set_uniform_matrix4(const std::string& name, const mat4& x) const
{
    assert(ID);
    const i32 loc { uniform_location(name) };
    if (loc != -1)
        glProgramUniformMatrix4fv(ID, loc, 1, GL_FALSE, x.data());
}

auto ShaderProgram::uniform_location(const std::string& name) const -> GLint
{
    assert(ID);
    if (!_uniformLocations.contains(name)) {
        _uniformLocations[name] = glGetUniformLocation(ID, name.c_str());
    }

    return _uniformLocations[name];
}
}