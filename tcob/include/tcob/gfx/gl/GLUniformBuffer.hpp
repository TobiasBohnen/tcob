// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <tcob/gfx/gl/GLObject.hpp>

namespace tcob::gl {
class UniformBuffer : public Object {
public:
    UniformBuffer() = default;
    explicit UniformBuffer(isize size);
    ~UniformBuffer() override;

    void create(isize size);

    void update(const void* data, isize size, isize offset) const;

    void bind_base(u32 index) const;

protected:
    void do_destroy() override;
};
}