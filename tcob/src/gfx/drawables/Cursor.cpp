// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/drawables/Cursor.hpp"

#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/RenderTarget.hpp"

namespace tcob::gfx {

cursor::cursor()
    : Position {{[]() -> point_i { return input::system::GetMousePosition(); },
                 [](point_i value) { input::system::SetMousePosition(value); }}}
{
    ActiveMode.Changed.connect([&](string const& name) {
        if (!_modes.contains(name)) {
            // TODO: log error
            return;
        }

        _currentMode = _modes[name];

        auto const  tex {Material->Texture};
        auto const& region {tex->get_region(name)};
        _vertex.TexCoords   = {region.UVRect.left(), region.UVRect.top(), static_cast<f32>(region.Level)};
        _size               = tex->get_size().Width;
        Material->PointSize = static_cast<f32>(_size);

        _renderer.set_material(Material().ptr());
    });

    Material.Changed.connect([&](auto const&) {
        Material->PointSize = static_cast<f32>(_size);
    });

    _vertex.Color = {255, 255, 255, 255};
    _renderer.set_geometry(_vertex);
}

void cursor::add_mode(string const& name, point_i hotspot)
{
    _modes[name] = {hotspot};
}

auto cursor::get_bounds() const -> rect_i
{
    return rect_i {{Position() - _currentMode.Hotspot}, {_size, _size}};
}

void cursor::on_update(milliseconds /* deltaTime */)
{
    point_f const pos {get_bounds().center()};
    _vertex.Position = {pos.X, pos.Y};
}

auto cursor::can_draw() const -> bool
{
    return !Material().is_expired();
}

void cursor::on_draw_to(render_target& target)
{
    _renderer.set_geometry(_vertex);
    _renderer.render_to_target(target);
}
}
