// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/gfx/drawables/TileMap.hpp>

#include <cassert>

#include <tcob/gfx/gl/GLTexture.hpp>

namespace tcob {
void TileMap::tile_set(const std::unordered_map<u16, std::string>& set)
{
    _tileSet = set;
    _isDirty = true;
}

void TileMap::modify_layer(isize layer, const PointU& pos, u16 setIdx)
{
    if (layer >= _layers.size()) {
        //TODO: log error
        return;
    }

    SizeU& size = _layers[layer].Size;
    if (pos.X >= size.Width || pos.Y >= size.Height) {
        //TODO: log error
        return;
    }

    isize tilepos { pos.X + pos.Y * size.Width + _layers[layer].TileMapBegin };

    _tileMap[tilepos] = setIdx;
    if (_material) {
        auto& regions { _material->Texture->regions() };

        Quad q {};
        if (setIdx != 0) {
            q.position({ { _position.X + _tileSize.Width * (pos.X + _layers[layer].Offset.X),
                             _position.Y + _tileSize.Height * (pos.Y + _layers[layer].Offset.Y) },
                _tileSize });
            q.color(Colors::White);
            q.texcoords(regions[_tileSet[setIdx]]);
        }

        _renderer.modify_geometry(&q, 1, tilepos);
    }
}

void TileMap::modify_tile_set(u16 setIdx, const std::string& newTex)
{
    if (setIdx == 0) {
        //TODO: log error
        return;
    }

    _tileSet[setIdx] = newTex;

    if (_material) {
        const auto& region { _material->Texture->regions()[newTex] };
        for (auto& layer : _layers) {
            for (isize i { 0 }; i < layer.TileCount; ++i) {
                const isize tileIdx { i + layer.TileMapBegin };
                if (_tileMap[tileIdx] == setIdx) {
                    Quad q {};
                    q.position({ { _position.X + _tileSize.Width * (i % layer.Size.Width + layer.Offset.X),
                                     _position.Y + _tileSize.Height * (i / layer.Size.Width + layer.Offset.Y) },
                        _tileSize });
                    q.color(Colors::White);
                    q.texcoords(region);

                    _renderer.modify_geometry(&q, 1, tileIdx);
                }
            }
        }
    }
}

auto TileMap::material() const -> ResourcePtr<Material>
{
    return _material;
}

void TileMap::material(ResourcePtr<Material> material)
{
    _material = std::move(material);
    if (_material)
        _renderer.material(_material.object());
}

void TileMap::tile_size(const SizeF& size)
{
    _tileSize = size;
    _isDirty = true;
}

auto TileMap::position() const -> PointF
{
    return _position;
}

void TileMap::position(const PointF& pos)
{
    _position = pos;
    _isDirty = true;
}

void TileMap::update(f64)
{
    if (_tileMap.empty() || !_material) {
        return;
    }

    if (_isDirty) {
        std::vector<Quad> quads;
        auto& regions { _material->Texture->regions() };

        for (auto& layer : _layers) {
            for (isize i { 0 }; i < layer.TileCount; ++i) {
                Quad& q { quads.emplace_back() };
                const u16 setIdx { _tileMap[i + layer.TileMapBegin] };
                if (setIdx != 0) {
                    q.position({ { _position.X + _tileSize.Width * (i % layer.Size.Width + layer.Offset.X),
                                     _position.Y + _tileSize.Height * (i / layer.Size.Width + layer.Offset.Y) },
                        _tileSize });
                    q.color(Colors::White);
                    q.texcoords(regions[_tileSet[setIdx]]);
                }
            }
        }
        _renderer.set_geometry(quads.data(), quads.size());
        _isDirty = false;
    }
}

void TileMap::draw(gl::RenderTarget& target)
{
    if (!is_visible() || _tileMap.empty() || !_material) {
        return;
    }

    _renderer.render_to_target(target);
}
}