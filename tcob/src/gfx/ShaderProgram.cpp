// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ShaderProgram.hpp"

#include <cassert>
#include <span>

#include "tcob/core/ServiceLocator.hpp"
#include "tcob/gfx/RenderSystem.hpp"
#include "tcob/gfx/RenderSystemImpl.hpp"

namespace tcob::gfx {

shader::shader(std::span<char const> vert, std::span<char const> frag)
    : _impl {locate_service<render_system>().create_shader()}
{
    [[maybe_unused]] bool const success {_impl->compile(vert, frag)};
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

}
