// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ShaderProgram.hpp"

#include <cassert>

#include "tcob/core/ServiceLocator.hpp"
#include "tcob/gfx/RenderSystem.hpp"
#include "tcob/gfx/RenderSystemImpl.hpp"

namespace tcob::gfx {

shader::shader(string const& vertexShaderSource, string const& fragmentShaderSource)
    : _impl {locate_service<render_system>().create_shader()}
{
    [[maybe_unused]] bool const success {compile(vertexShaderSource, fragmentShaderSource)};
    assert(success);
}

shader::~shader() = default;

shader::operator bool() const
{
    return is_valid();
}

auto shader::is_valid() const -> bool
{
    return _impl->is_valid();
}

auto shader::compile(string const& vertexShaderSource, string const& fragmentShaderSource) -> bool
{
    return _impl->compile(vertexShaderSource, fragmentShaderSource);
}

} // namespace tcob
