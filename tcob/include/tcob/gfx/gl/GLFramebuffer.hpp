// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <tcob/core/data/Size.hpp>
#include <tcob/gfx/gl/GLObject.hpp>

namespace tcob::gl {
class Framebuffer final : public Object {
public:
    Framebuffer() = default;
    ~Framebuffer() override;

    void create();

    void bind() const;
    static void BindDefault();

    void clear(const Color& c) const;

    void attach_texture(const Texture2D* tex);

    void blit_to(u32 target);

protected:
    void do_destroy() override;

private:
    u32 _rbo { 0 };
    SizeU _size { SizeU::Zero };
};
}