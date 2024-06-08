// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "GLESObject.hpp"

#include "tcob/gfx/RenderSystemImpl.hpp"

namespace tcob::gfx::gles30 {
////////////////////////////////////////////////////////////

class gl_uniform_buffer : public gl_object, public tcob::gfx::render_backend::uniform_buffer_base {
public:
    explicit gl_uniform_buffer(usize size);
    ~gl_uniform_buffer() override;

    void update(void const* data, usize size, usize offset) const override;

    void bind_base(u32 index) const override;

protected:
    void do_destroy() override;
};
}
