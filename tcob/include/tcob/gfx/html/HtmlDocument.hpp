// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#if defined(TCOB_ENABLE_ADDON_GFX_LITEHTML)

    #include <memory>

    #include "tcob/core/Signal.hpp"
    #include "tcob/core/assets/AssetLibrary.hpp"
    #include "tcob/core/input/Input.hpp"
    #include "tcob/gfx/Canvas.hpp"
    #include "tcob/gfx/Font.hpp"
    #include "tcob/gfx/Geometry.hpp"
    #include "tcob/gfx/Material.hpp"
    #include "tcob/gfx/Renderer.hpp"
    #include "tcob/gfx/Transformable.hpp"
    #include "tcob/gfx/Window.hpp"
    #include "tcob/gfx/drawables/Drawable.hpp"
    #include "tcob/gfx/html/HtmlElementPainter.hpp"

namespace litehtml {
class document;
};

namespace tcob::gfx::html {
////////////////////////////////////////////////////////////

namespace detail {
    class container;
}

////////////////////////////////////////////////////////////
class TCOB_API document final : public entity, public rect_transformable {
public:
    ////////////////////////////////////////////////////////////
    struct config {
        assets::group*                 AssetGroup {nullptr};
        assets::asset_ptr<font_family> Fonts;
        i32                            DefaultFontSize {0};
        window*                        Window {nullptr};
        string                         MasterCSSPath;
    };

    ////////////////////////////////////////////////////////////

    explicit document(config c);
    ~document() override;

    signal<string const> AnchorClick;

    auto get_mouse_position() const -> point_i;
    auto is_button_down() const -> bool;

    void from_string(string const& html);
    auto load(path const& file) noexcept -> load_status;

    void change_language(string const& language, string const& culture);

    void force_redraw();

protected:
    void on_update(milliseconds deltaTime) override;
    void on_fixed_update(milliseconds deltaTime) override;

    auto can_draw() const -> bool override;
    void on_draw_to(render_target& target) override;

    void on_transform_changed() override;

    void on_mouse_motion(input::mouse::motion_event& ev) override;
    void on_mouse_button_down(input::mouse::button_event& ev) override;
    void on_mouse_button_up(input::mouse::button_event& ev) override;

    auto convert_screen_to_world(point_i pos) const -> point_i;

private:
    canvas _canvas;
    config _config;

    std::unique_ptr<element_painter>    _painter;
    std::shared_ptr<detail::container>  _container;
    std::shared_ptr<litehtml::document> _lhdoc;

    quad_renderer _renderer {buffer_usage_hint::DynamicDraw};
    quad          _quad {};
    bool          _isTransformDirty {true};
    bool          _needsRedraw {true};

    bool    _isMouseOver {false};
    point_i _mousePosition;
    bool    _buttonDown {false};

    assets::manual_asset_ptr<material> _material {};
};
}

#endif
