// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <unordered_map>

#include <tcob/assets/Resource.hpp>
#include <tcob/core/data/Point.hpp>
#include <tcob/core/data/Rect.hpp>
#include <tcob/core/data/Size.hpp>
#include <tcob/gfx/Material.hpp>
#include <tcob/gfx/Quad.hpp>
#include <tcob/gfx/drawables/Drawable.hpp>
#include <tcob/gfx/gl/GLRenderer.hpp>

namespace tcob {
class TileMap final : public Drawable {
public:
    template <isize Width, isize Height>
    auto add_layer(const std::array<u16, Width * Height>& map, const PointU& tileOffset = PointU::Zero) -> isize;
    void modify_layer(isize layer, const PointU& pos, u16 setIdx);

    void tile_set(const std::unordered_map<u16, std::string>& set);
    void modify_tile_set(u16 setIdx, const std::string& newTex);

    auto material() const -> ResourcePtr<Material>;
    void material(ResourcePtr<Material> material);

    void tile_size(const SizeF& size);

    auto position() const -> PointF;
    void position(const PointF& position);

    void update(f64 deltaTime) override;

    void draw(gl::RenderTarget& target) override;

private:
    std::vector<u16> _tileMap;

    struct Layer {
        SizeU Size;
        isize TileCount;
        isize TileMapBegin;
        PointU Offset;
    };

    std::vector<Layer> _layers {};
    std::unordered_map<u16, std::string> _tileSet {};

    PointF _position { PointF::Zero };
    SizeF _tileSize { SizeF::Zero };

    gl::StaticQuadRenderer _renderer {};
    ResourcePtr<Material> _material;
    bool _isDirty { false };
};

template <isize Width, isize Height>
auto TileMap::add_layer(const std::array<u16, Width * Height>& map, const PointU& tileOffset) -> isize
{
    Layer layer {
        .Size = { Width, Height },
        .TileCount = Width * Height,
        .TileMapBegin = _tileMap.size(),
        .Offset = tileOffset
    };
    _layers.push_back(layer);
    _tileMap.insert(_tileMap.end(), map.begin(), map.end());

    _isDirty = true;
    return _layers.size() - 1;
}

}
