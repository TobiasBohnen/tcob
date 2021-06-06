// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/gfx/gl/GLWindow.hpp>

#include <SDL2/SDL.h>
#include <glad/gl.h>

#include <tcob/core/io/Logger.hpp>
#include <tcob/gfx/Material.hpp>
#include <tcob/gfx/gl/GLTexture.hpp>

namespace tcob::gl {

Window::Window(const PointU& loc, const SizeU& size, u32 aa)
{
    // Set attributes
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, aa);

    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);

    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);

    i32 flags { SDL_WINDOW_OPENGL };
    flags |= SDL_WINDOW_RESIZABLE;

    // Create window
    _window = SDL_CreateWindow("", loc.X, loc.Y, size.Width, size.Height, flags);
    if (!_window) {
        Log("Error: Window creation failed!");
        throw std::runtime_error("Window creation failed");
    }

    // Create OpenGL context
    _context = std::make_unique<gl::Context>(_window);
    on_resize(size);

    Quad q {};
    q.color(Colors::White);
    q.position({ 0, 0, 1.f, 1.f });
    q.texcoords({ { 0, 0, 1, -1 }, 0 });
    _renderer = std::make_unique<StaticQuadRenderer>();
    _renderer->set_geometry(&q, 1);
}

Window::~Window()
{
    _context = nullptr;
    SDL_DestroyWindow(_window);
}

auto Window::settings() const -> WindowSettings
{
    return _settings;
}

void Window::settings(const WindowSettings& settings)
{
    settings.Fullscreen ? SDL_SetWindowFullscreen(_window, SDL_WINDOW_FULLSCREEN)
                        : SDL_SetWindowFullscreen(_window, 0);
    _settings.Fullscreen = settings.Fullscreen;
    settings.VSync ? SDL_GL_SetSwapInterval(1)
                   : SDL_GL_SetSwapInterval(0);
    _settings.VSync = settings.VSync;
    title(settings.Title);
}

void Window::title(const std::string& title)
{
    SDL_SetWindowTitle(_window, title.c_str());
    _settings.Title = title;
}

void Window::load_icon(const std::string& filename)
{
    Image img { Image::Load(filename) };
    auto info { img.info() };
    SDL_Surface* surface { SDL_CreateRGBSurfaceFrom(
        img.buffer(),
        info.SizeInPixels.Width, info.SizeInPixels.Height, 32, info.Stride,
        0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000) };

    // The icon is attached to the window pointer
    SDL_SetWindowIcon(_window, surface);

    // ...and the surface containing the icon pixel data is no longer required.
    SDL_FreeSurface(surface);
}

auto Window::has_focus() const -> bool
{
    return SDL_GetWindowFlags(_window) & SDL_WINDOW_MOUSE_FOCUS
        && SDL_GetWindowFlags(_window) & SDL_WINDOW_INPUT_FOCUS;
}

auto Window::cursor() const -> ResourcePtr<Cursor>
{
    return _cursor;
}

void Window::cursor(ResourcePtr<Cursor> cursor)
{
    SDL_ShowCursor(cursor ? SDL_DISABLE : SDL_ENABLE);
    _cursor = std::move(cursor);
}

void Window::clear(const Color& c) const
{
    RenderTarget::clear(c);
    // clear default framebuffer
    vec4 color { c.R / 255.f, c.G / 255.f, c.B / 255.f, c.A / 255.f };
    glClearNamedFramebufferfv(0, GL_COLOR, 0, color.data());
    glClearNamedFramebufferfi(0, GL_DEPTH_STENCIL, 0, 1, 0);
}

auto Window::size() const -> SizeU
{
    GLint vpWidth {}, vpHeight {};
    SDL_GL_GetDrawableSize(_window, &vpWidth, &vpHeight);
    return { static_cast<u32>(vpWidth), static_cast<u32>(vpHeight) };
}

void Window::process_events(SDL_Event* ev)
{
    WindowEvent event {
        .WindowID = ev->window.windowID,
        .Data1 = ev->window.data1,
        .Data2 = ev->window.data2
    };

    switch (ev->window.event) {
    case SDL_WINDOWEVENT_SHOWN:
        WindowShown(event);
        break;
    case SDL_WINDOWEVENT_HIDDEN:
        WindowHidden(event);
        break;
    case SDL_WINDOWEVENT_EXPOSED:
        WindowExposed(event);
        break;
    case SDL_WINDOWEVENT_MOVED:
        WindowMoved(event);
        break;
    case SDL_WINDOWEVENT_RESIZED:
        WindowResized(event);
        break;
    case SDL_WINDOWEVENT_SIZE_CHANGED:
        on_resize({ static_cast<u32>(event.Data1), static_cast<u32>(event.Data2) });
        WindowSizeChanged(event);
        break;
    case SDL_WINDOWEVENT_MINIMIZED:
        WindowMinimized(event);
        break;
    case SDL_WINDOWEVENT_MAXIMIZED:
        WindowMaximized(event);
        break;
    case SDL_WINDOWEVENT_RESTORED:
        WindowRestored(event);
        break;
    case SDL_WINDOWEVENT_ENTER:
        WindowEnter(event);
        break;
    case SDL_WINDOWEVENT_LEAVE:
        WindowLeave(event);
        break;
    case SDL_WINDOWEVENT_FOCUS_GAINED:
        WindowFocusGained(event);
        break;
    case SDL_WINDOWEVENT_FOCUS_LOST:
        WindowFocusLost(event);
        break;
    case SDL_WINDOWEVENT_CLOSE:
        WindowClose(event);
        break;
    case SDL_WINDOWEVENT_TAKE_FOCUS:
        WindowTakeFocus(event);
        break;
    case SDL_WINDOWEVENT_HIT_TEST:
        WindowHitTest(event);
        break;
    }
}

void Window::on_resize(const SizeU& newsize)
{
    const f32 aspectRatio { static_cast<f32>(newsize.Width) / newsize.Height };
    camera().aspect_ratio(aspectRatio);
    setup_framebuffer(newsize);
    _defaultTarget.size(newsize);
}

void Window::swap()
{
    auto mat { material() };
    if (!mat->Shader) {
        mat->Shader = DefaultShader;
    }

    if (mat) {
        const SizeI s { size() };
        glViewport(0, 0, s.Width, s.Height);

        _renderer->material(mat.object());
        _renderer->render_to_target(_defaultTarget);

        if (_cursor) {
            _cursor->draw(_defaultTarget);
            _cursor->active_mode("default"); //set cursor to default mode if available
        }
    }

    SDL_GL_SwapWindow(_window);
}
}