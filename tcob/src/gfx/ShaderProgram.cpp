// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ShaderProgram.hpp"

#include "tcob/core/ServiceLocator.hpp"
#include "tcob/gfx/RenderSystem.hpp"
#include "tcob/gfx/RenderSystemImpl.hpp"

namespace tcob::gfx {

shader::shader()
    : _impl {locate_service<render_system>().create_shader()}
{
}

shader::~shader() = default;

shader::operator bool() const
{
    return _impl->is_valid();
}

auto shader::create(string const& vertexShaderSource, string const& fragmentShaderSource) -> bool
{
    return _impl->compile(vertexShaderSource, fragmentShaderSource);
}

auto shader::get_uniform_block_binding(string const& name) -> u32
{
    if (!_uniformBlockBindings.contains(name)) {
        return _uniformBlockBindings[name] = _impl->get_uniform_block_binding(name);
    }

    return _uniformBlockBindings[name];
}

} // namespace tcob
