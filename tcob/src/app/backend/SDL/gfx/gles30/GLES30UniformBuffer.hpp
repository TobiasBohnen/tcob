// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "GLES30Object.hpp"

#include "tcob/gfx/RenderSystemImpl.hpp"

namespace tcob::gfx::gles30 {
////////////////////////////////////////////////////////////

class gl_uniform_buffer : public gl_object, public tcob::gfx::render_backend::uniform_buffer_base {
public:
    explicit gl_uniform_buffer(usize size);
    ~gl_uniform_buffer() override;

    auto update(bool data, usize offset) const -> usize;

    template <POD T>
    auto update(T data, usize offset) const -> usize;

    void bind_base(u32 index) const override;

protected:
    void do_destroy() override;

private:
    void update(void const* data, usize size, usize offset) const override;
};

template <POD T>
inline auto gl_uniform_buffer::update(T data, usize offset) const -> usize
{
    update(&data, sizeof(data), offset);
    return sizeof(data);
}

}
