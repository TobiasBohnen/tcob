// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/core/Size.hpp"
#include "tcob/gfx/RenderSystemImpl.hpp"

#include "GLESObject.hpp"

namespace tcob::gfx::gles30 {
////////////////////////////////////////////////////////////

class gl_shader : public gl_object, public tcob::gfx::render_backend::shader_base {
public:
    gl_shader();
    ~gl_shader() override;

    auto compile(string const& vertexShaderSource, string const& fragmentShaderSource) -> bool override;

    auto get_uniform_block_binding(string const& name) const -> i32 override;

    auto get_uniform_location(string const& name) const -> i32;

    void set_uniform(i32 loc, i32 x) const;
    void set_uniform(i32 loc, ivec2 const& x) const;
    void set_uniform(i32 loc, ivec3 const& x) const;
    void set_uniform(i32 loc, ivec4 const& x) const;
    void set_uniform(i32 loc, size_i x) const;
    void set_uniform(i32 loc, point_i x) const;

    void set_uniform(i32 loc, u32 x) const;
    void set_uniform(i32 loc, uvec2 const& x) const;
    void set_uniform(i32 loc, uvec3 const& x) const;
    void set_uniform(i32 loc, uvec4 const& x) const;
    void set_uniform(i32 loc, size_u x) const;
    void set_uniform(i32 loc, point_u x) const;

    void set_uniform(i32 loc, f32 x) const;
    void set_uniform(i32 loc, vec2 const& x) const;
    void set_uniform(i32 loc, vec3 const& x) const;
    void set_uniform(i32 loc, vec4 const& x) const;
    void set_uniform(i32 loc, size_f x) const;
    void set_uniform(i32 loc, point_f x) const;

    void set_uniform(i32 loc, mat4 const& x) const;

    auto is_valid() const -> bool override;
    auto get_id() const -> u32;

protected:
    void do_destroy() override;

private:
    void use() const;
};
}
