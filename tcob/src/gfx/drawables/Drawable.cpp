// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/drawables/Drawable.hpp"

namespace tcob::gfx {

auto drawable::is_visible() const -> bool
{
    return _visible && can_draw();
}

void drawable::show()
{
    if (!_visible) {
        _visible = true;
        on_visiblity_changed();
        VisibilityChanged(true);
    }
}

void drawable::hide()
{
    if (_visible) {
        _visible = false;
        on_visiblity_changed();
        VisibilityChanged(false);
    }
}

void drawable::draw_to(render_target& target)
{
    std::scoped_lock lock {_mutex};
    if (target.Camera->VisibilityMask & VisibilityMask) {
        if (is_visible()) {
            on_draw_to(target);
        }
    }
}

void drawable::update(milliseconds deltaTime)
{
    std::scoped_lock lock {_mutex};
    on_update(deltaTime);
}

void drawable::on_visiblity_changed()
{
}

////////////////////////////////////////////////////////////

auto entity::get_update_mode() const -> update_mode
{
    return update_mode::Normal;
}

void entity::fixed_update(milliseconds deltaTime)
{
    on_fixed_update(deltaTime);
}

auto entity::can_draw() const -> bool
{
    return true;
}

////////////////////////////////////////////////////////////

}
