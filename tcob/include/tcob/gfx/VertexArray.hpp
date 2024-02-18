// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <span>

#include "tcob/gfx/Geometry.hpp"
#include "tcob/gfx/Gfx.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

namespace render_backend {
    class vertex_array_base;
}

////////////////////////////////////////////////////////////

class TCOB_API vertex_array {
public:
    explicit vertex_array(buffer_usage_hint usage);

    void resize(usize vertCount, usize indCount);

    void update_data(std::span<vertex const> verts, usize vertOffset) const;
    void update_data(std::span<quad const> quads, usize quadOffset) const;
    void update_data(std::span<u32 const> inds, usize indOffset) const;

    void draw_elements(primitive_type mode, usize count, u32 offset) const;
    void draw_arrays(primitive_type mode, i32 first, usize count) const;

private:
    std::unique_ptr<render_backend::vertex_array_base> _impl;
};

}
