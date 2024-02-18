// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <span>
#include <vector>

#include "tcob/core/Property.hpp"
#include "tcob/core/assets/Asset.hpp"
#include "tcob/gfx/Geometry.hpp"
#include "tcob/gfx/Material.hpp"
#include "tcob/gfx/Renderer.hpp"
#include "tcob/gfx/drawables/Drawable.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

class TCOB_API static_point_cloud final : public drawable {
public:
    explicit static_point_cloud(std::span<vertex> points);

    prop<assets::asset_ptr<material>> Material;

protected:
    void on_update(milliseconds deltaTime) override;

    auto can_draw() const -> bool override;
    void on_draw_to(render_target& target) override;

private:
    point_renderer _renderer {buffer_usage_hint::StaticDraw};
};

////////////////////////////////////////////////////////////

class TCOB_API point_cloud final : public drawable {
public:
    explicit point_cloud(i32 reservedSize);

    prop<assets::asset_ptr<material>> Material;

    auto create_point() -> vertex&;

    auto get_point_count() const -> i32;
    auto get_point_at(i32 index) -> vertex&;

protected:
    void on_update(milliseconds deltaTime) override;

    auto can_draw() const -> bool override;
    void on_draw_to(render_target& target) override;

private:
    std::vector<vertex> _points {};
    point_renderer      _renderer {buffer_usage_hint::DynamicDraw};
};

}
