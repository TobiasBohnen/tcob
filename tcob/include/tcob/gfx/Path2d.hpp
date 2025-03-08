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

#include <optional>
#include <variant>
#include <vector>

#include "tcob/core/AngleUnits.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/Polygon.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

class TCOB_API path2d {
    friend class canvas;

public:
    std::vector<f32> Commands;

    void move_to(point_f pos);
    void line_to(point_f pos);
    void cubic_bezier_to(point_f cp0, point_f cp1, point_f end);
    void quad_bezier_to(point_f cp, point_f end);
    void arc_to(f32 radiusX, f32 radiusY, degree_f rotX, bool largeArc, bool sweep, point_f end);
    void close();

    auto polygonize() -> std::vector<polygon>;

    auto static Parse(string_view path) -> std::optional<path2d>;

private:
    auto static CommandsMoveTo(point_f pos) -> std::vector<f32>;
    auto static CommandsLineTo(point_f pos) -> std::vector<f32>;
    auto static CommandsCubicTo(point_f cp0, point_f cp1, point_f end) -> std::vector<f32>;
    auto static CommandsQuadTo(point_f start, point_f cp, point_f end) -> std::vector<f32>;

    auto static GetCommands(string_view path) -> std::optional<std::vector<std::variant<char, f32>>>;

    point_f _lastPoint {point_f::Zero};
    point_f _lastQuadControlPoint {point_f::Zero};
    point_f _lastCubicControlPoint {point_f::Zero};
};

}
