// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "MeshFormat_obj.hpp"

#include <optional>
#include <vector>

#include "tcob/core/Point.hpp"
#include "tcob/core/StringUtils.hpp"
#include "tcob/core/io/Stream.hpp"
#include "tcob/gfx/Geometry.hpp"

namespace tcob::gfx::detail {

auto obj_loader::load(io::istream& in) -> std::optional<geometry_store>
{
    std::vector<point_f> pos;
    std::vector<point_f> uv;
    std::vector<u32>     posInds;
    std::vector<u32>     uvInds;

    while (!in.is_eof()) {
        string const line {in.read_string_until('\n')};
        if (line.empty() || line[0] == '#') { continue; }

        auto const split {helper::split(line, ' ')};
        if (split.empty()) { continue; }

        if (split[0] == "v" && split.size() >= 4) {
            // Vertex position: v x y z
            auto const x {helper::to_number<f32>(split[1])};
            auto const y {helper::to_number<f32>(split[2])};
            if (!x || !y) { return std::nullopt; }
            pos.emplace_back(*x, *y);
        } else if (split[0] == "vt" && split.size() >= 3) {
            // Texture coordinate: vt u v
            auto const u {helper::to_number<f32>(split[1])};
            auto const v {helper::to_number<f32>(split[2])};
            if (!u || !v) { return std::nullopt; }
            uv.emplace_back(*u, *v);
        } else if (split[0] == "f" && split.size() >= 4) {
            // Face: f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3
            for (i32 i {1}; i <= 3; ++i) {
                auto const& faceStr {split[i]};
                auto const  slashes {helper::split(faceStr, '/')};
                if (!slashes.empty()) {
                    auto const posIdx {helper::to_number<u32>(slashes[0])};
                    if (!posIdx) { return std::nullopt; }
                    posInds.push_back(*posIdx - 1);

                    if (slashes.size() > 1 && !slashes[1].empty()) {
                        auto const uvIdx {helper::to_number<u32>(slashes[1])};
                        if (!uvIdx) { return std::nullopt; }
                        uvInds.push_back(*uvIdx - 1);
                    }
                }
            }
        }
    }

    geometry_store      store;
    std::vector<vertex> verts;
    verts.reserve(pos.size());

    for (usize i {0}; i < pos.size(); ++i) {
        point_f const texcoord {(i < uv.size()) ? uv[i] : point_f {0, 0}};
        verts.push_back({.Position  = pos[i],
                         .Color     = colors::White,
                         .TexCoords = {.U = texcoord.X, .V = texcoord.Y, .Level = 0}});
    }

    store.set(0, verts, posInds);
    return store;
}

}
