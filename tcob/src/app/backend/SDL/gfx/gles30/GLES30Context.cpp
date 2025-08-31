// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "GLES30Context.hpp"

#include <memory>
#include <stdexcept>

#include <SDL3/SDL.h>
#include <glad/gles30.h>

#include "GLES30Object.hpp"
#include "GLES30ShaderProgram.hpp"

#include "tcob/core/Logger.hpp"

namespace tcob::gfx::gles30 {

u32 gl_context::DefaultShader {0};
u32 gl_context::DefaultTexturedShader {0};
u32 gl_context::DefaultFontShader {0};

static char const* defaultVertShader {
#include "shaders/default.vert"
};
static char const* defaultFragShader {
#include "shaders/default.frag"
};
static char const* defaultTexturedFragShader {
#include "shaders/default-textured.frag"
};
static char const* defaultFontFragShader {
#include "shaders/default-font.frag"
};

extern "C" {
static auto load(char const* c) -> GLADapiproc
{
    return reinterpret_cast<GLADapiproc>(SDL_GL_GetProcAddress(c));
}
}

gl_context::gl_context(SDL_Window* window)
{
    // Create OpenGL context
    i32 const glMajor {3}, glMinor {0};
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, glMajor);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, glMinor);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#if defined(TCOB_DEBUG)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif

    logger::Info("GLESContext: want OpenGLES version: {}.{}", glMajor, glMinor);

    _context = SDL_GL_CreateContext(window); // NOLINT(cppcoreguidelines-prefer-member-initializer)
    if (!_context) {
        logger::Error("GLESContext: OpenGL context creation failed!");
        throw std::runtime_error("OpenGL context creation failed");
    }

    if (!gladLoadGLES2(&load)) {
        SDL_GL_DestroyContext(static_cast<SDL_GLContext>(_context));
        logger::Error("GLESContext: OpenGL loading failed!");
        throw std::runtime_error("OpenGL loading failed");
    }

    i32 checkMajor {}, checkMinor {};
    glGetIntegerv(GL_MAJOR_VERSION, &checkMajor);
    glGetIntegerv(GL_MINOR_VERSION, &checkMinor);

    logger::Info("GLESContext: have OpenGLES version: {}.{}", checkMajor, checkMinor);

    // init default shaders
    _defaultShader = std::make_shared<gl_shader>();
    _defaultShader->compile(defaultVertShader, defaultFragShader);
    DefaultShader = _defaultShader->ID;

    _defaultTexShader = std::make_shared<gl_shader>();
    _defaultTexShader->compile(defaultVertShader, defaultTexturedFragShader);
    DefaultTexturedShader = _defaultTexShader->ID;

    _defaultFontShader = std::make_shared<gl_shader>();
    _defaultFontShader->compile(defaultVertShader, defaultFontFragShader);
    DefaultFontShader = _defaultFontShader->ID;
}

gl_context::~gl_context()
{
    gl_object::DestroyAll();
    SDL_GL_DestroyContext(static_cast<SDL_GLContext>(_context));
}

}
