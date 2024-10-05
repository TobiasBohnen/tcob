// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <span>

#include <glad/gl45.h>

#include "tcob/gfx/Geometry.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/RenderSystemImpl.hpp"

#include "GLObject.hpp"

namespace tcob::gfx::gl45 {
////////////////////////////////////////////////////////////

class gl_vertex_array final : public gl_object, public tcob::gfx::render_backend::vertex_array_base {
public:
    gl_vertex_array(buffer_usage_hint usage);
    ~gl_vertex_array() override;

    void resize(usize vertCount, usize indCount) override;

    void update_data(std::span<vertex const> verts, usize vertOffset) const override;
    void update_data(std::span<quad const> quads, usize quadOffset) const override;
    void update_data(std::span<u32 const> inds, usize indOffset) const override;

    void draw_elements(primitive_type mode, usize count, u32 offset) const override;
    void draw_arrays(primitive_type mode, i32 first, usize count) const override;

protected:
    void do_destroy() override;

private:
    void setup_attributes() const;

    u32    _vbo {0};
    u32    _ebo {0};
    usize  _vboSize {0};
    usize  _eboSize {0};
    GLenum _usage {};
};
}
