// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <tcob/core/Updatable.hpp>
#include <tcob/gfx/gl/GLRenderTarget.hpp>

namespace tcob {
class Drawable : public Updatable {
public:
    virtual ~Drawable() = default;

    auto is_visible() const -> bool;
    void show();
    void hide();

    virtual void draw(gl::RenderTarget& target) = 0;

private:
    bool _visible { true };
};
}