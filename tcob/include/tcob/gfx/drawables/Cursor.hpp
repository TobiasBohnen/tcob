// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/core/FlatMap.hpp"
#include "tcob/core/Property.hpp"
#include "tcob/core/assets/Asset.hpp"
#include "tcob/gfx/Geometry.hpp"
#include "tcob/gfx/Renderer.hpp"
#include "tcob/gfx/drawables/Drawable.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

class TCOB_API cursor final : public drawable {
public:
    cursor();

    prop<assets::asset_ptr<material>> Material;
    prop_fn<point_i>                  Position;
    prop<string>                      ActiveMode;

    void add_mode(string const& name, point_i hotspot = point_i::Zero);

    auto get_bounds() const -> rect_i;

    static inline char const* asset_name {"cursor"};

protected:
    void on_update(milliseconds deltaTime) override;

    auto can_draw() const -> bool override;
    void on_draw_to(render_target& target) override;

private:
    struct cursor_mode {
        point_i Hotspot {point_i::Zero};
    };

    flat_map<string, cursor_mode> _modes {};
    cursor_mode                   _currentMode {};
    vertex                        _vertex {};
    point_renderer                _renderer {buffer_usage_hint::StreamDraw};
    i32                           _size {0};
};
}
