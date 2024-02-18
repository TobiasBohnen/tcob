// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/core/Color.hpp"
#include "tcob/core/Signal.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/core/assets/Asset.hpp"
#include "tcob/gfx/RenderTarget.hpp"
#include "tcob/gfx/Renderer.hpp"
#include "tcob/gfx/drawables/Cursor.hpp"

////////////////////////////////////////////////////////////

using SDL_Event = union SDL_Event;
struct SDL_Window;

namespace tcob {
class game;
namespace gfx::render_backend {
    class window_base;
}
}

namespace tcob::gfx {
////////////////////////////////////////////////////////////

enum class messagebox_type : u8 {
    Error,
    Warning,
    Info
};

////////////////////////////////////////////////////////////

class TCOB_API window final : public render_target {
    friend class tcob::game;

public:
    struct event {
        u32 WindowID {0};
        i32 Data1 {0};
        i32 Data2 {0};
    };

    ~window() override;

    signal<event const> WindowShown;
    signal<event const> WindowHidden;
    signal<event const> WindowExposed;
    signal<event const> WindowMoved;
    signal<event const> WindowResized;
    signal<event const> WindowSizeChanged;
    signal<event const> WindowMinimized;
    signal<event const> WindowMaximized;
    signal<event const> WindowRestored;
    signal<event const> WindowEnter;
    signal<event const> WindowLeave;
    signal<event const> WindowFocusGained;
    signal<event const> WindowFocusLost;
    signal<event const> WindowClose;
    signal<event const> WindowTakeFocus;
    signal<event const> WindowHitTest;

    prop_fn<bool>                   FullScreen;
    prop_fn<string>                 Title;
    prop_fn<bool>                   VSync;
    prop<assets::asset_ptr<cursor>> Cursor;

    void load_icon(path const& file);

    void hide_system_cursor(bool value);

    auto has_focus() const -> bool;
    void grab_input(bool grab);

    void draw_to(render_target& target);
    void swap_buffer() const;

    void process_events(SDL_Event* ev);

protected:
    auto get_size() const -> size_i override;
    void set_size(size_i newsize) override;
    void on_clear(color c) const override;

private:
    explicit window(std::unique_ptr<render_backend::window_base> window, assets::manual_asset_ptr<texture> const& texture = {});

    auto get_fullscreen() const -> bool;
    void set_fullscreen(bool value);

    auto get_title() const -> string;
    void set_title(string const& value);

    assets::manual_asset_ptr<texture>  _texture;
    assets::manual_asset_ptr<material> _material {};

    std::unique_ptr<render_backend::window_base> _impl;
    SDL_Window*                                  _window;
    quad_renderer                                _renderer {buffer_usage_hint::StaticDraw};
};

}
