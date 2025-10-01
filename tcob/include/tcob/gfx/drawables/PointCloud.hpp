// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <vector>

#include "tcob/core/Property.hpp"
#include "tcob/core/assets/Asset.hpp"
#include "tcob/gfx/Geometry.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/Material.hpp"
#include "tcob/gfx/RenderTarget.hpp"
#include "tcob/gfx/Renderer.hpp"
#include "tcob/gfx/drawables/Drawable.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

class TCOB_API point_cloud final : public drawable {
public:
    explicit point_cloud(i32 reservedSize);

    prop<asset_ptr<material>> Material;

    auto create_point() -> vertex&;
    void clear();

    auto size() const -> i32;
    auto get_point_at(i32 index) -> vertex&;

protected:
    auto can_draw() const -> bool override;
    void on_draw_to(render_target& target) override;

private:
    std::vector<vertex> _points {};
    point_renderer      _renderer {buffer_usage_hint::DynamicDraw};
};

}
