// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <memory>
#include <unordered_set>

#include "tcob/core/Point.hpp"
#include "tcob/core/Property.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Signal.hpp"
#include "tcob/core/Transform.hpp"
#include "tcob/core/assets/Asset.hpp"
#include "tcob/core/assets/Assets.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/Canvas.hpp"
#include "tcob/gfx/Geometry.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/Material.hpp"
#include "tcob/gfx/Renderer.hpp"
#include "tcob/gfx/Transformable.hpp"
#include "tcob/gfx/Window.hpp"
#include "tcob/gfx/drawables/Drawable.hpp"

namespace litehtml {
class document;
};

namespace tcob::gfx::html {
////////////////////////////////////////////////////////////

namespace detail {
    class container;
}

////////////////////////////////////////////////////////////

struct config {
    assets::group*             AssetGroup {nullptr};
    std::unordered_set<string> Fonts;
    string                     DefaultFont;
    i32                        DefaultFontSize {0};
    canvas*                    Canvas {nullptr};
};

////////////////////////////////////////////////////////////

class TCOB_API document final : public entity, public transformable {
public:
    document(config c, window& window);
    ~document() override;

    signal<string const> AnchorClick;

    prop<rect_f> Bounds;

    void create_from_string(string const& html, string const& css);

    auto bounds() const -> rect_f override;

    void force_redraw();

protected:
    void on_update(milliseconds deltaTime) override;

    auto can_draw() const -> bool override;
    void on_draw_to(render_target& target, transform const& xform) override;

    auto pivot() const -> point_f override;
    void on_transform_changed() override;

    void on_mouse_motion(input::mouse::motion_event const& ev) override;
    void on_mouse_button_down(input::mouse::button_event const& ev) override;
    void on_mouse_button_up(input::mouse::button_event const& ev) override;

    auto convert_screen_to_world(point_i pos) const -> point_i;

private:
    config  _config;
    window& _window;

    std::shared_ptr<detail::container>  _container;
    std::shared_ptr<litehtml::document> _lhdoc;

    renderer _renderer {buffer_usage_hint::DynamicDraw};
    quad     _quad {};
    bool     _isTransformDirty {true};
    bool     _needsRedraw {true};

    bool _isMouseOver {false};
    bool _buttonDown {false};

    asset_owner_ptr<material> _material {};
};
}
