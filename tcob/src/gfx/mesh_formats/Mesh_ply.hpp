// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <optional>
#include <unordered_map>
#include <vector>

#include "tcob/core/io/Stream.hpp"
#include "tcob/gfx/Geometry.hpp"
#include "tcob/gfx/drawables/Shape.hpp"

namespace tcob::gfx::detail {

class ply_loader : public mesh_loader {
public:
    auto load(io::istream& in) -> std::optional<geometry_store> override;

private:
    enum class format_type : u8 {
        ASCII,
        BinaryLittleEndian,
        BinaryBigEndian
    };

    enum class vertex_attrib : u8 {
        Unused,
        X,
        Y,
        S,
        T,
        Red,
        Green,
        Blue,
        Alpha
    };

    struct property {
        vertex_attrib Attribute;
        string        Name;
        string        Type;
        bool          IsList {false};
        string        ListCountType;
        string        ListType;
    };

    struct element {
        string                Name;
        i32                   Count {0};
        std::vector<property> Properties;
    };

    struct header {
        format_type          Format;
        std::vector<element> Elements;
    };

    auto parse_header(io::istream& in) -> bool;
    auto read_data(io::istream& in) -> std::optional<geometry_store>;

    auto read_ascii_vertex(io::istream& in, std::unordered_map<vertex_attrib, u32>& attribs, element const& el) -> bool;
    auto read_ascii_face(io::istream& in) -> bool;
    auto read_binary_vertex(io::istream& in, std::unordered_map<vertex_attrib, u32>& attribs, element const& el) -> bool;
    auto read_binary_face(io::istream& in, element const& el, isize listIdx) -> bool;

    auto read_value(io::istream& in, string const& type, bool littleEndian) const -> f32;

    header              _header;
    std::vector<vertex> _verts;
    std::vector<u32>    _inds;
};

}
