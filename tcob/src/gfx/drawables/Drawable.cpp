// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/drawables/Drawable.hpp"

#include "tcob/core/Interfaces.hpp"
#include "tcob/gfx/RenderTarget.hpp"

namespace tcob::gfx {

auto drawable::is_visible() const -> bool
{
    return _visible && can_draw();
}

void drawable::show()
{
    if (_visible) { return; }

    _visible = true;
    on_visiblity_changed();
    VisibilityChanged(true);
}

void drawable::hide()
{
    if (!_visible) { return; }

    _visible = false;
    on_visiblity_changed();
    VisibilityChanged(false);
}

void drawable::draw_to(render_target& target)
{
    if (target.camera().VisibilityMask & VisibilityMask) {
        if (is_visible()) {
            on_draw_to(target);
        }
    }
}

void drawable::on_visiblity_changed()
{
}

////////////////////////////////////////////////////////////

entity::entity(update_mode mode)
    : _mode {mode}
{
}

void entity::update(milliseconds deltaTime)
{
    if (_mode == update_mode::Fixed) { return; }

    on_update(deltaTime);
}

void entity::fixed_update(milliseconds deltaTime)
{
    if (_mode == update_mode::Normal) { return; }

    on_fixed_update(deltaTime);
}

auto entity::can_draw() const -> bool
{
    return true;
}

////////////////////////////////////////////////////////////

}
