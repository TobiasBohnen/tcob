// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <tcob/gfx/gl/GLEnum.hpp>
#include <tcob/gfx/gl/GLObject.hpp>

namespace tcob::gl {
class ShaderStorageBuffer : public Object {
public:
    ShaderStorageBuffer(isize size, BufferUsage usage);
    ~ShaderStorageBuffer() override;

    void update(const void* data, isize size, isize offset) const;

    void resize(isize size, BufferUsage usage);

    void bind_base(u32 index) const;

protected:
    void do_destroy() override;

private:
    isize _bufferSize { 0 };
};
}