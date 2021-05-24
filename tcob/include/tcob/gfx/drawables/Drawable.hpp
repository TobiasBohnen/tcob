// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <tcob/gfx/gl/GLRenderTarget.hpp>

namespace tcob {
class Drawable {
public:
    virtual ~Drawable() = default;

    auto is_visible() const -> bool;
    void show();
    void hide();

    virtual void update(f64 deltaTime) = 0;

    virtual void draw(gl::RenderTarget& target) = 0;

private:
    bool _visible { true };
};
}