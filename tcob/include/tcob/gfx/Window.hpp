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
#include "tcob/core/Size.hpp"
#include "tcob/core/assets/Asset.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/RenderTarget.hpp"
#include "tcob/gfx/Renderer.hpp"
#include "tcob/gfx/ShaderProgram.hpp"
#include "tcob/gfx/Texture.hpp"
#include "tcob/gfx/drawables/Cursor.hpp"

////////////////////////////////////////////////////////////

using SDL_Event = union SDL_Event;
struct SDL_Window;

namespace tcob::gfx {

////////////////////////////////////////////////////////////

class TCOB_API window final : public render_target {
    friend class tcob::gfx::render_system;

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
    signal<event const> SizeChanged;
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

    prop<assets::asset_ptr<cursor>> Cursor;
    prop<bool>                      SystemCursorEnabled;

    prop<assets::asset_ptr<shader>> Shader;

    auto bounds() const -> rect_i;

    void load_icon(path const& file);

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
    explicit window(std::unique_ptr<render_backend::window_base> window, assets::owning_asset_ptr<texture> const& texture = {});

    auto get_fullscreen() const -> bool;
    void set_fullscreen(bool value);

    auto get_title() const -> string;
    void set_title(string const& value);

    assets::owning_asset_ptr<texture>  _texture;
    assets::owning_asset_ptr<material> _material {};

    std::unique_ptr<render_backend::window_base> _impl;
    quad_renderer                                _renderer {buffer_usage_hint::StaticDraw};
};

}
