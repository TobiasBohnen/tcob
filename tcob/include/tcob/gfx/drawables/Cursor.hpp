// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <unordered_map>

#include "tcob/core/Interfaces.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Property.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/core/assets/Asset.hpp"
#include "tcob/gfx/Geometry.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/Material.hpp"
#include "tcob/gfx/RenderTarget.hpp"
#include "tcob/gfx/Renderer.hpp"
#include "tcob/gfx/drawables/Drawable.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

class TCOB_API cursor final : public drawable, public updatable {
public:
    cursor();

    prop<assets::asset_ptr<material>> Material;
    prop_fn<point_i>                  Position;
    prop<string>                      ActiveMode;

    void add_mode(string const& name, point_i hotspot = point_i::Zero);

    auto bounds() const -> rect_i;

    static inline char const* AssetName {"cursor"};

protected:
    void on_update(milliseconds deltaTime) override;

    auto can_draw() const -> bool override;
    void on_draw_to(render_target& target) override;

private:
    struct cursor_mode {
        point_i Hotspot {point_i::Zero};
    };

    std::unordered_map<string, cursor_mode> _modes {};
    cursor_mode                             _currentMode {};

    size_i        _size {};
    quad          _quad {};
    quad_renderer _renderer {buffer_usage_hint::StreamDraw};
};
}
