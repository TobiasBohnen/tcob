// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "GLESWindow.hpp"

#include <SDL.h>
#include <glad/gles30.h>

#include "tcob/core/Logger.hpp"

#include "GLES30.hpp"
#include "GLESContext.hpp"

namespace tcob::gfx::gles30 {

gl_window::gl_window(size_i size)
{
    // Set attributes
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);

    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);

    i32 const flags {SDL_WINDOW_OPENGL};

    // Create window
    logger::Info("GLESWindow: creating window");
    _window = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, size.Width, size.Height, flags);
    if (!_window) {
        logger::Error("GLESWindow: Window creation failed!");
        throw std::runtime_error("Window creation failed");
    }

    _context = std::make_unique<gl_context>(_window);
}

gl_window::~gl_window()
{
    _context = nullptr;
    if (_window) {
        SDL_DestroyWindow(_window);
    }
}

auto gl_window::get_vsync() const -> bool
{
    return SDL_GL_GetSwapInterval() == 1;
}

void gl_window::set_vsync(bool value)
{
    value ? SDL_GL_SetSwapInterval(1)
          : SDL_GL_SetSwapInterval(0);
}

void gl_window::clear(color c) const
{
    vec4 const color {c.R / 255.0f, c.G / 255.0f, c.B / 255.0f, c.A / 255.0f};
    GLCHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    GLCHECK(glClearBufferfv(GL_COLOR, 0, color.data()));
    GLCHECK(glClearBufferfi(GL_DEPTH_STENCIL, 0, 1, 0));
}

void gl_window::set_viewport(rect_i const& rect)
{
    GLCHECK(glViewport(rect.left(), rect.top(), rect.Width, rect.Height));
}

void gl_window::swap_buffer() const
{
    SDL_GL_SwapWindow(_window);
}

}
