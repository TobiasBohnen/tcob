// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <optional>

#include "tcob/gfx/Geometry.hpp"
#include "tcob/gfx/drawables/Shape.hpp"

namespace tcob::gfx::detail {
////////////////////////////////////////////////////////////

class obj_loader final : public mesh_loader {
public:
    auto load(io::istream& in) -> std::optional<geometry_store> override;
};

}
