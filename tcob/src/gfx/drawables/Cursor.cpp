// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/gfx/drawables/Cursor.hpp>

#include <SDL2/SDL_mouse.h>
#include <tcob/gfx/gl/GLTexture.hpp>
#include <tcob/gfx/gl/GLWindow.hpp>

namespace tcob {
Cursor::Cursor()
{
    _vertex.Color = { 255, 255, 255, 255 };
    _renderer.set_geometry(&_vertex, 1);
}

auto Cursor::position() const -> PointI
{
    i32 x { 0 }, y { 0 };
    SDL_GetMouseState(&x, &y);
    return { x, y };
}

void Cursor::define_mode(const std::string& name, const std::string& tex, const PointI& hotspot)
{
    _modes[name] = { tex, hotspot };
}

void Cursor::active_mode(const std::string& name)
{
    if (!_modes.contains(name)) {
        //TODO: log error
        return;
    }

    _currentMode = _modes[name];
    auto tex { _material->Texture.object() };

    auto region { tex->regions()[_currentMode.TextureName] };
    _vertex.TexCoords = { region.UVRect.Left, region.UVRect.Top, static_cast<f32>(region.Level) };

    _size = static_cast<i32>(_material->Texture->size().Width);
    _renderer.material(_material.object(), static_cast<f32>(_size));
}

void Cursor::update([[maybe_unused]] MilliSeconds deltaTime)
{
}

auto Cursor::material() const -> ResourcePtr<Material>
{
    return _material;
}

void Cursor::material(ResourcePtr<Material> material)
{
    _material = std::move(material);
}

void Cursor::draw(gl::RenderTarget& target)
{
    if (!is_visible() || !_material) {
        return;
    }

    PointI pos { position() - _currentMode.Hotspot + PointI { _size / 2, _size / 2 } };
    SizeF winSize { target.size() };
    _vertex.Position = { pos.X / winSize.Width, pos.Y / winSize.Height };

    _renderer.set_geometry(&_vertex, 1);

    _renderer.render_to_target(target);
}
}