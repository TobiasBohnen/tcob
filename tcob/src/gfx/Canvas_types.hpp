// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

// based on NanoVG/NanoSVG
// original license:
// Copyright (c) 2013 Mikko Mononen memon@inside.org
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//
#pragma once
#include "tcob/tcob_config.hpp"

#include <stack>
#include <vector>

#include "tcob/gfx/Canvas.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

enum point_flags : u8 {
    Corner     = 0x01,
    Left       = 0x02,
    Bevel      = 0x04,
    InnerBevel = 0x08,
};

enum commands : u8 {
    MoveTo   = 0,
    LineTo   = 1,
    BezierTo = 2,
    Close    = 3,
    Winding  = 4,
};

auto constexpr EPSILON {1e-6f};

auto Normalize(f32& x, f32& y) -> f32;

struct canvas_point {
    f32   X {0}, Y {0};
    f32   DX {0}, DY {0};
    f32   Length {0};
    f32   DMX {0}, DMY {0};
    ubyte Flags {0};
};

////////////////////////////////////////////////////////////

struct state {
    blend_funcs      CompositeOperation {};
    bool             ShapeAntiAlias {true};
    canvas::paint    Fill {};
    canvas::paint    Stroke {};
    f32              StrokeWidth {1};
    f32              MiterLimit {10.0f};
    line_join        LineJoin {line_join::Miter};
    line_cap         LineCap {line_cap::Butt};
    f32              Alpha {1};
    transform        XForm {transform::Identity};
    canvas::scissor  Scissor {};
    alignments       TextAlign {};
    font*            Font {nullptr};
    std::vector<f32> Dash;
};

////////////////////////////////////////////////////////////

class states {
public:
    auto get() -> state&;
    auto get() const -> state const&;

    void save();
    void restore();

    void reset();

private:
    std::stack<state> _states {};
};

////////////////////////////////////////////////////////////

class path_cache {
public:
    void clear();

    void append_commands(std::span<f32 const> vals, transform const& xform);

    void flatten_paths(bool enforceWinding);

    void expand_stroke(f32 w, f32 fringe, line_cap lineCap, line_join lineJoin, f32 miterLimit);
    void expand_fill(f32 w, line_join lineJoin, f32 miterLimit, f32 fringeWidth);

    auto alloc_temp_verts(usize nverts) -> vertex*;

    auto paths() const -> std::vector<canvas::path> const&;
    auto has_commands() const -> bool;

    auto command_point() const -> point_f const&;
    auto bounds() const -> vec4 const&;

    void set_tolerances(f32 dist, f32 tess);
    auto is_degenerate_arc(point_f pos1, point_f pos2, f32 radius) const -> bool;

private:
    void add_path();
    auto get_last_path() -> canvas::path&;
    void add_point(f32 x, f32 y, i32 flags);
    auto get_last_point() -> canvas_point&;
    void tesselate_bezier(f32 x1, f32 y1, f32 x2, f32 y2, f32 x3, f32 y3, f32 x4, f32 y4, i32 level, i32 type);
    void calculate_joins(f32 w, line_join lineJoin, f32 miterLimit);

    std::vector<vertex>       _verts;
    std::vector<canvas_point> _points;

    std::vector<canvas::path> _paths;
    std::vector<f32>          _commands;

    point_f _commandPoint;
    vec4    _bounds {};

    f32 _distTolerance {0};
    f32 _tessTolerance {0};
};

}
