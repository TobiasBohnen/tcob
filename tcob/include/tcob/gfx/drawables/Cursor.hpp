// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <unordered_map>

#include <tcob/assets/Resource.hpp>
#include <tcob/gfx/Quad.hpp>
#include <tcob/gfx/drawables/Drawable.hpp>
#include <tcob/gfx/gl/GLRenderer.hpp>

namespace tcob {
class Cursor final : public Drawable {
public:
    Cursor();

    auto position() const -> PointI;

    auto material() const -> ResourcePtr<Material>;
    void material(ResourcePtr<Material> material);

    void define_mode(const std::string& name, const std::string& tex, const PointI& hotspot = PointI::Zero);
    void active_mode(const std::string& name);

    void update(f64 deltaTime) override;

    void draw(gl::RenderTarget& target) override;

private:
    struct CursorMode {
        std::string TextureName;
        PointI Hotspot;
    };

    Vertex _vertex {};
    gl::StreamPointRenderer _renderer {};
    ResourcePtr<Material> _material;

    std::unordered_map<std::string, CursorMode> _modes;
    CursorMode _currentMode;
    i32 _size { 0 };
};
}