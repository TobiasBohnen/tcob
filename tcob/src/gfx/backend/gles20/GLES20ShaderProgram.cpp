// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "GLES20ShaderProgram.hpp"

#include <array>
#include <cassert>

#include <glad/gles20.h>

#include "GLES20.hpp"

#include "tcob/core/Logger.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Size.hpp"

namespace tcob::gfx::gles20 {
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

    char const* cstr {nullptr};

    // Build compile VERTEX_SHADER
    GLuint const vertexShader {glCreateShader(GL_VERTEX_SHADER)};
    cstr = vertexShaderSource.c_str();
    GLCHECK(glShaderSource(vertexShader, 1, &cstr, nullptr));
    GLCHECK(glCompileShader(vertexShader));

    // Check for compile errors VERTEX_SHADER
    std::array<GLchar, 512> infoLog {};
    GLCHECK(glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success));
    if (!success) {
        GLCHECK(glGetShaderInfoLog(vertexShader, sizeof(infoLog), nullptr, infoLog.data()));
        logger::Error("Shader: vertex shader compilation error: {}", infoLog.data());
        return false;
    }

    // Build compile FRAGMENT_SHADER
    GLuint const fragmentShader {glCreateShader(GL_FRAGMENT_SHADER)};
    cstr = fragmentShaderSource.c_str();
    GLCHECK(glShaderSource(fragmentShader, 1, &cstr, nullptr));
    GLCHECK(glCompileShader(fragmentShader));

    // Check for compile errors FRAGMENT_SHADER
    GLCHECK(glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success));
    if (!success) {
        GLCHECK(glGetShaderInfoLog(fragmentShader, sizeof(infoLog), nullptr, infoLog.data()));
        logger::Error("Shader: fragment shader compilation error: {}", infoLog.data());
        return false;
    }

    // Link shaders
    GLCHECK(glAttachShader(ID, vertexShader));
    GLCHECK(glAttachShader(ID, fragmentShader));
    GLCHECK(glLinkProgram(ID));

    // Check the linking errors
    GLCHECK(glGetProgramiv(ID, GL_LINK_STATUS, &success));
    if (!success) {
        GLCHECK(glGetProgramInfoLog(ID, sizeof(infoLog), nullptr, infoLog.data()));
        logger::Error("Shader: linking error: {}", infoLog.data());
        return false;
    }

    logger::Debug("Shader: linked ID {}", ID);

    // Delete shaders
    GLCHECK(glDeleteShader(vertexShader));
    GLCHECK(glDeleteShader(fragmentShader));

    return true;
}

void gl_shader::use() const
{
    assert(ID);
    GLCHECK(glUseProgram(ID));
}

void gl_shader::do_destroy()
{
    assert(ID);
    GLCHECK(glDeleteProgram(ID));
}

void gl_shader::set_uniform(i32 loc, i32 x) const
{
    use();
    GLCHECK(glUniform1i(loc, x));
}

void gl_shader::set_uniform(i32 loc, u32 x) const
{
    use();
    set_uniform(loc, static_cast<i32>(x));
}

void gl_shader::set_uniform(i32 loc, f32 x) const
{
    use();
    GLCHECK(glUniform1f(loc, x));
}

void gl_shader::set_uniform(i32 loc, ivec2 const& x) const
{
    use();
    GLCHECK(glUniform2i(loc, x[0], x[1]));
}

void gl_shader::set_uniform(i32 loc, uvec2 const& x) const
{
    use();
    GLCHECK(glUniform2i(loc, static_cast<i32>(x[0]), static_cast<i32>(x[1])));
}

void gl_shader::set_uniform(i32 loc, vec2 const& x) const
{
    use();
    GLCHECK(glUniform2f(loc, x[0], x[1]));
}

void gl_shader::set_uniform(i32 loc, ivec3 const& x) const
{
    use();
    GLCHECK(glUniform3i(loc, x[0], x[1], x[2]));
}

void gl_shader::set_uniform(i32 loc, uvec3 const& x) const
{
    use();
    GLCHECK(glUniform3i(loc, static_cast<i32>(x[0]), static_cast<i32>(x[1]), static_cast<i32>(x[2])));
}

void gl_shader::set_uniform(i32 loc, vec3 const& x) const
{
    use();
    GLCHECK(glUniform3f(loc, x[0], x[1], x[2]));
}

void gl_shader::set_uniform(i32 loc, ivec4 const& x) const
{
    use();
    GLCHECK(glUniform4i(loc, x[0], x[1], x[2], x[3]));
}

void gl_shader::set_uniform(i32 loc, uvec4 const& x) const
{
    use();
    GLCHECK(glUniform4i(loc, static_cast<i32>(x[0]), static_cast<i32>(x[1]), static_cast<i32>(x[2]), static_cast<i32>(x[3])));
}

void gl_shader::set_uniform(i32 loc, vec4 const& x) const
{
    use();
    GLCHECK(glUniform4f(loc, x[0], x[1], x[2], x[3]));
}

void gl_shader::set_uniform(i32 loc, size_f x) const
{
    use();
    GLCHECK(glUniform2f(loc, x.Width, x.Height));
}

void gl_shader::set_uniform(i32 loc, point_f x) const
{
    use();
    GLCHECK(glUniform2f(loc, x.X, x.Y));
}

void gl_shader::set_uniform(i32 loc, size_i x) const
{
    use();
    GLCHECK(glUniform2i(loc, x.Width, x.Height));
}

void gl_shader::set_uniform(i32 loc, point_i x) const
{
    use();
    GLCHECK(glUniform2i(loc, x.X, x.Y));
}

void gl_shader::set_uniform(i32 loc, size_u x) const
{
    use();
    GLCHECK(glUniform2i(loc, static_cast<i32>(x.Width), static_cast<i32>(x.Height)));
}

void gl_shader::set_uniform(i32 loc, point_u x) const
{
    use();
    GLCHECK(glUniform2i(loc, static_cast<i32>(x.X), static_cast<i32>(x.Y)));
}

void gl_shader::set_uniform(i32 loc, mat4 const& x) const
{
    use();
    GLCHECK(glUniformMatrix4fv(loc, 1, GL_FALSE, x.data()));
}

auto gl_shader::is_valid() const -> bool
{
    return ID != 0;
}

auto gl_shader::get_uniform_location(string const& name) const -> i32
{
    assert(ID);
    return glGetUniformLocation(ID, name.c_str());
}
}
