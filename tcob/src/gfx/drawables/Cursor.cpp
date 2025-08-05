// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/drawables/Cursor.hpp"

#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/Geometry.hpp"
#include "tcob/gfx/RenderTarget.hpp"

namespace tcob::gfx {

cursor::cursor()
    : Position {{[]() -> point_i { return locate_service<input::system>().mouse()->get_position(); },
                 [](point_i value) { locate_service<input::system>().mouse()->set_position(value); }}}
{
    ActiveMode.Changed.connect([this](string const& name) {
        if (!_modes.contains(name)) {
            // TODO: log error
            return;
        }

        _currentMode = _modes[name];

        auto const  tex {Material->Texture};
        auto const& region {tex->regions()[name]};
        geometry::set_texcoords(_quad, region);
        _size = tex->info().Size;

        _renderer.set_material((*Material).ptr());
    });

    geometry::set_color(_quad, colors::White);
}

void cursor::add_mode(string const& name, point_i hotspot)
{
    _modes[name] = {hotspot};
}

auto cursor::bounds() const -> rect_i
{
    return rect_i {{*Position - _currentMode.Hotspot}, _size};
}

void cursor::on_update(milliseconds /* deltaTime */)
{
    geometry::set_position(_quad, rect_f {bounds()});
}

auto cursor::can_draw() const -> bool
{
    return !(*Material).is_expired();
}

void cursor::on_draw_to(render_target& target)
{
    _renderer.set_geometry(_quad);
    _renderer.render_to_target(target);
}
}
