// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <tcob/gfx/gl/GLObject.hpp>

namespace tcob::gl {
class PixelPackBuffer final : public Object {
public:
    PixelPackBuffer() = default;
    ~PixelPackBuffer() override;

    void create(isize size);

    void bind() const;
    static void BindDefault();

    auto map() const -> void*;
    void unmap() const;

protected:
    void do_destroy() override;
};
}