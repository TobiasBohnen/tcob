// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Mesh_ply.hpp"

#include <bit>
#include <optional>
#include <unordered_map>
#include <vector>

#include "tcob/core/Color.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/StringUtils.hpp"
#include "tcob/core/io/Stream.hpp"
#include "tcob/gfx/Geometry.hpp"

namespace tcob::gfx::detail {

auto ply_loader::load(io::istream& in) -> std::optional<geometry_store>
{
    if (!in) { return std::nullopt; }
    if (!parse_header(in)) { return std::nullopt; }
    return read_data(in);
}

auto ply_loader::parse_header(io::istream& in) -> bool
{
    string line {in.read_string_until('\n')};
    if (line != "ply") { return false; } // Check signature

    element* currentElement {nullptr};

    while (!in.is_eof()) {
        line = in.read_string_until('\n');
        if (line.empty()) { continue; }

        auto const parts {helper::split(line, ' ')};
        if (parts.empty()) { continue; }

        if (parts[0] == "format") {
            if (parts.size() < 3) { return false; }

            if (parts[1] == "ascii") {
                _header.Format = format_type::ASCII;
            } else if (parts[1] == "binary_little_endian") {
                _header.Format = format_type::BinaryLittleEndian;
            } else if (parts[1] == "binary_big_endian") {
                _header.Format = format_type::BinaryBigEndian;
            } else {
                return false;
            }
        } else if (parts[0] == "element") {
            if (parts.size() < 3) { return false; }

            element& elem {_header.Elements.emplace_back()};
            elem.Name = parts[1];
            if (!helper::try_to_number<i32>(parts[2], elem.Count)) { return false; }
            currentElement = &elem;
        } else if (parts[0] == "property") {
            if (!currentElement || parts.size() < 3) { return false; }

            property prop {};

            if (parts[1] == "list") {
                if (parts.size() < 5) { return false; }
                prop.IsList        = true;
                prop.ListCountType = parts[2];
                prop.ListType      = parts[3];
                prop.Name          = parts[4];
            } else {
                prop.Type = parts[1];

                auto const str {parts[2]};
                if (str == "x") {
                    prop.Attribute = vertex_attrib::X;
                } else if (str == "y") {
                    prop.Attribute = vertex_attrib::Y;
                } else if (str == "s") {
                    prop.Attribute = vertex_attrib::S;
                } else if (str == "t") {
                    prop.Attribute = vertex_attrib::T;
                } else if (str == "red") {
                    prop.Attribute = vertex_attrib::Red;
                } else if (str == "green") {
                    prop.Attribute = vertex_attrib::Green;
                } else if (str == "blue") {
                    prop.Attribute = vertex_attrib::Blue;
                } else if (str == "alpha") {
                    prop.Attribute = vertex_attrib::Alpha;
                } else {
                    prop.Attribute = vertex_attrib::Unused;
                }
                prop.Name = str;
            }

            currentElement->Properties.push_back(prop);
        } else if (parts[0] == "end_header") {
            break;
        }
    }

    return true;
}

auto ply_loader::read_data(io::istream& in) -> std::optional<geometry_store>
{
    for (auto const& el : _header.Elements) {
        if (el.Name == "vertex") {
            _verts.reserve(el.Count);
            std::unordered_map<vertex_attrib, u32> attribs;
            for (u32 i {0}; i < el.Properties.size(); ++i) {
                attribs[el.Properties[i].Attribute] = i;
            }

            for (i32 v {0}; v < el.Count; ++v) {
                if (_header.Format == format_type::ASCII) {
                    if (!read_ascii_vertex(in, attribs, el)) { return std::nullopt; }
                } else {
                    if (!read_binary_vertex(in, attribs, el)) { return std::nullopt; }
                }
            }
        } else if (el.Name == "face") {
            _inds.reserve(el.Count * 3);

            isize listIdx {-1};
            for (u32 i {0}; i < el.Properties.size(); ++i) {
                if (el.Properties[i].IsList && (el.Properties[i].Name == "vertex_indices" || el.Properties[i].Name == "vertex_index")) {
                    listIdx = i;
                    break;
                }
            }

            if (listIdx < 0) { return std::nullopt; }

            for (i32 f {0}; f < el.Count; ++f) {
                if (_header.Format == format_type::ASCII) {
                    if (!read_ascii_face(in)) { return std::nullopt; }
                } else {
                    if (!read_binary_face(in, el, listIdx)) { return std::nullopt; }
                }
            }
        }
    }

    geometry_store store;
    store.set(0, _verts, _inds);
    return store;
}

auto ply_loader::read_ascii_vertex(io::istream& in, std::unordered_map<vertex_attrib, u32>& attribs, element const& el) -> bool
{
    string const line {in.read_string_until('\n')};
    auto const   parts {helper::split(line, ' ')};

    if (std::ssize(parts) < std::ssize(el.Properties)) { return false; }

    vertex& vert {_verts.emplace_back()};
    if (!helper::try_to_number(parts[attribs[vertex_attrib::X]], vert.Position.X)) { return false; }
    if (!helper::try_to_number(parts[attribs[vertex_attrib::Y]], vert.Position.Y)) { return false; }
    helper::try_to_number(parts[attribs[vertex_attrib::S]], vert.TexCoords.U);
    helper::try_to_number(parts[attribs[vertex_attrib::T]], vert.TexCoords.V);
    helper::try_to_number(parts[attribs[vertex_attrib::Red]], vert.Color.R);
    helper::try_to_number(parts[attribs[vertex_attrib::Green]], vert.Color.G);
    helper::try_to_number(parts[attribs[vertex_attrib::Blue]], vert.Color.B);
    helper::try_to_number(parts[attribs[vertex_attrib::Alpha]], vert.Color.A);

    return true;
}

auto ply_loader::read_ascii_face(io::istream& in) -> bool
{
    string const line {in.read_string_until('\n')};
    auto const   parts {helper::split(line, ' ')};

    if (parts.empty()) { return false; }

    i32 vertCount {0};
    if (!helper::try_to_number<i32>(parts[0], vertCount)) { return false; }

    if (vertCount == 3 && parts.size() >= 4) { // Triangle
        u32 v0 {0}, v1 {0}, v2 {0};
        if (!helper::try_to_number<u32>(parts[1], v0)) { return false; }
        if (!helper::try_to_number<u32>(parts[2], v1)) { return false; }
        if (!helper::try_to_number<u32>(parts[3], v2)) { return false; }

        _inds.push_back(v0);
        _inds.push_back(v1);
        _inds.push_back(v2);
    } else if (vertCount == 4 && parts.size() >= 5) { // Quad
        u32 v0 {0}, v1 {0}, v2 {0}, v3 {0};
        if (!helper::try_to_number<u32>(parts[1], v0)) { return false; }
        if (!helper::try_to_number<u32>(parts[2], v1)) { return false; }
        if (!helper::try_to_number<u32>(parts[3], v2)) { return false; }
        if (!helper::try_to_number<u32>(parts[4], v3)) { return false; }

        _inds.push_back(v0);
        _inds.push_back(v1);
        _inds.push_back(v2);

        _inds.push_back(v0);
        _inds.push_back(v2);
        _inds.push_back(v3);
    } else {
        return false;
    }

    return true;
}

auto ply_loader::read_binary_vertex(io::istream& in, std::unordered_map<vertex_attrib, u32>& attribs, element const& el) -> bool
{
    bool const       littleEndian {_header.Format == format_type::BinaryLittleEndian};
    std::vector<f32> values;
    values.resize(el.Properties.size());

    for (u32 i {0}; i < el.Properties.size(); ++i) {
        auto const& prop {el.Properties[i]};
        if (prop.IsList) {
            i32 const count {static_cast<i32>(read_value(in, prop.ListCountType, littleEndian))};
            for (i32 j {0}; j < count; ++j) {
                read_value(in, prop.ListType, littleEndian);
            }
        } else {
            values[i] = read_value(in, prop.Type, littleEndian);
        }
    }

    vertex& vert {_verts.emplace_back()};
    if (attribs.contains(vertex_attrib::X)) {
        vert.Position.X = values[attribs[vertex_attrib::X]];
    } else {
        return false;
    }
    if (attribs.contains(vertex_attrib::Y)) {
        vert.Position.Y = values[attribs[vertex_attrib::Y]];
    } else {
        return false;
    }
    if (attribs.contains(vertex_attrib::S)) {
        vert.TexCoords.U = values[attribs[vertex_attrib::S]];
    }
    if (attribs.contains(vertex_attrib::T)) {
        vert.TexCoords.V = values[attribs[vertex_attrib::T]];
    }
    if (attribs.contains(vertex_attrib::Red)) {
        vert.Color.R = static_cast<u8>(values[attribs[vertex_attrib::Red]]);
    }
    if (attribs.contains(vertex_attrib::Green)) {
        vert.Color.G = static_cast<u8>(values[attribs[vertex_attrib::Green]]);
    }
    if (attribs.contains(vertex_attrib::Blue)) {
        vert.Color.B = static_cast<u8>(values[attribs[vertex_attrib::Blue]]);
    }
    if (attribs.contains(vertex_attrib::Alpha)) {
        vert.Color.A = static_cast<u8>(values[attribs[vertex_attrib::Alpha]]);
    }

    return true;
}

auto ply_loader::read_binary_face(io::istream& in, element const& el, isize listIdx) -> bool
{
    bool const littleEndian {_header.Format == format_type::BinaryLittleEndian};
    for (isize i {0}; i < std::ssize(el.Properties); ++i) {
        auto const& prop {el.Properties[i]};

        if (i == listIdx) {
            i32 const        vertCount {static_cast<i32>(read_value(in, prop.ListCountType, littleEndian))};
            std::vector<u32> faceIndices;

            for (i32 j {0}; j < vertCount; ++j) {
                faceIndices.push_back(static_cast<u32>(read_value(in, prop.ListType, littleEndian)));
            }

            if (faceIndices.size() == 3) {
                _inds.push_back(faceIndices[0]);
                _inds.push_back(faceIndices[1]);
                _inds.push_back(faceIndices[2]);
            } else if (faceIndices.size() == 4) {
                _inds.push_back(faceIndices[0]);
                _inds.push_back(faceIndices[1]);
                _inds.push_back(faceIndices[2]);

                _inds.push_back(faceIndices[0]);
                _inds.push_back(faceIndices[2]);
                _inds.push_back(faceIndices[3]);
            } else {
                return false;
            }
        } else if (prop.IsList) {
            i32 const count {static_cast<i32>(read_value(in, prop.ListCountType, littleEndian))};
            for (i32 j {0}; j < count; ++j) {
                read_value(in, prop.ListType, littleEndian);
            }
        } else {
            read_value(in, prop.Type, littleEndian);
        }
    }

    return true;
}

auto ply_loader::read_value(io::istream& in, string const& type, bool littleEndian) const -> f32
{
    if (type == "uchar" || type == "uint8") {
        return static_cast<f32>(littleEndian ? in.read<u8, std::endian::little>() : in.read<u8, std::endian::big>());
    }
    if (type == "short" || type == "int16") {
        return static_cast<f32>(littleEndian ? in.read<i16, std::endian::little>() : in.read<i16, std::endian::big>());
    }
    if (type == "ushort" || type == "uint16") {
        return static_cast<f32>(littleEndian ? in.read<u16, std::endian::little>() : in.read<u16, std::endian::big>());
    }
    if (type == "int" || type == "int32") {
        return static_cast<f32>(littleEndian ? in.read<i32, std::endian::little>() : in.read<i32, std::endian::big>());
    }
    if (type == "uint" || type == "uint32") {
        return static_cast<f32>(littleEndian ? in.read<u32, std::endian::little>() : in.read<u32, std::endian::big>());
    }
    if (type == "float" || type == "float32") {
        return littleEndian ? in.read<f32, std::endian::little>() : in.read<f32, std::endian::big>();
    }
    return 0.0f;
}

}
