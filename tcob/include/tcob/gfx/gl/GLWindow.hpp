// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <tcob/assets/Resource.hpp>
#include <tcob/core/data/Point.hpp>
#include <tcob/gfx/Quad.hpp>
#include <tcob/gfx/drawables/Cursor.hpp>
#include <tcob/gfx/gl/GLContext.hpp>
#include <tcob/gfx/gl/GLRenderTarget.hpp>
#include <tcob/gfx/gl/GLRenderer.hpp>
#include <tcob/thirdparty/sigslot/signal.hpp>

typedef union SDL_Event SDL_Event;

namespace tcob {
struct WindowEvent {
    u32 WindowID;
    i32 Data1;
    i32 Data2;
};
}

////////////////////////////////////////////////////////////

namespace tcob::gl {
struct WindowSettings {
    bool Fullscreen { false };
    bool VSync { false };
    std::string Title { "" };
};

class Window final : public RenderTarget {
public:
    explicit Window(SDL_Window* window);
    ~Window() override;

    sigslot::signal<WindowEvent&> WindowShown;
    sigslot::signal<WindowEvent&> WindowHidden;
    sigslot::signal<WindowEvent&> WindowExposed;
    sigslot::signal<WindowEvent&> WindowMoved;
    sigslot::signal<WindowEvent&> WindowResized;
    sigslot::signal<WindowEvent&> WindowSizeChanged;
    sigslot::signal<WindowEvent&> WindowMinimized;
    sigslot::signal<WindowEvent&> WindowMaximized;
    sigslot::signal<WindowEvent&> WindowRestored;
    sigslot::signal<WindowEvent&> WindowEnter;
    sigslot::signal<WindowEvent&> WindowLeave;
    sigslot::signal<WindowEvent&> WindowFocusGained;
    sigslot::signal<WindowEvent&> WindowFocusLost;
    sigslot::signal<WindowEvent&> WindowClose;
    sigslot::signal<WindowEvent&> WindowTakeFocus;
    sigslot::signal<WindowEvent&> WindowHitTest;

    auto settings() const -> WindowSettings;
    void settings(const WindowSettings& settings);
    void title(const std::string& title);

    void load_icon(const std::string& filename);

    auto has_focus() const -> bool;

    auto cursor() const -> ResourcePtr<Cursor>;
    void cursor(ResourcePtr<Cursor> cursor);

    void clear(const Color& c) const override;
    void swap();

    auto size() const -> SizeU override;

    void process_events(SDL_Event* ev);

    static inline ResourcePtr<gl::ShaderProgram> DefaultShader;

protected:
    void on_resize(const SizeU& newsize);

private:
    SDL_Window* _window { nullptr };

    ResourcePtr<Cursor> _cursor;
    StaticQuadRenderer _renderer;
    DefaultRenderTarget _defaultTarget;
    WindowSettings _settings;
};
}