// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <memory>

#include "tcob/core/Color.hpp"
#include "tcob/core/Property.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Signal.hpp"
#include "tcob/core/assets/Asset.hpp"
#include "tcob/gfx/Geometry.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/RenderTarget.hpp"
#include "tcob/gfx/Renderer.hpp"
#include "tcob/gfx/ShaderProgram.hpp"
#include "tcob/gfx/Texture.hpp"
#include "tcob/gfx/drawables/Cursor.hpp"

namespace tcob::gfx {

////////////////////////////////////////////////////////////

class TCOB_API window : public render_target {
public:
    struct event : event_base {
        u32 WindowID {0};
        i32 Data1 {0};
        i32 Data2 {0};
    };

    ~window() override;

    signal<event const> Shown;
    signal<event const> Hidden;
    signal<event const> Exposed;
    signal<event const> Moved;
    signal<event const> Resized;
    signal<event const> Minimized;
    signal<event const> Maximized;
    signal<event const> Restored;
    signal<event const> Enter;
    signal<event const> Leave;
    signal<event const> FocusGained;
    signal<event const> FocusLost;
    signal<event const> Close;
    signal<event const> HitTest;

    prop_fn<bool>   FullScreen;
    prop_fn<string> Title;
    prop_fn<bool>   VSync;

    prop<asset_ptr<cursor>> Cursor;
    prop<bool>              SystemCursorEnabled;

    prop<asset_ptr<shader>> Shader;

    auto bounds() const -> rect_i;

    virtual void load_icon(path const& file) = 0;

    virtual auto has_focus() const -> bool = 0;
    virtual void grab_input(bool grab)     = 0;

    void draw_to(render_target& target);
    void swap_buffer() const;

    virtual void process_events(void* ev) = 0;

    auto get_impl() const -> render_backend::window_base*;

protected:
    explicit window(std::unique_ptr<render_backend::window_base> windowBase, asset_owner_ptr<texture> const& texture = {});

    void on_clear(color c) const override;

    void init_renderer(quad const& q);

private:
    virtual auto get_fullscreen() const -> bool = 0;
    virtual void set_fullscreen(bool value)     = 0;

    virtual auto get_title() const -> string    = 0;
    virtual void set_title(string const& value) = 0;

    asset_owner_ptr<texture>  _texture;
    asset_owner_ptr<material> _material {};

    std::unique_ptr<render_backend::window_base> _impl;
    quad_renderer                                _renderer {buffer_usage_hint::StaticDraw};
};

}
