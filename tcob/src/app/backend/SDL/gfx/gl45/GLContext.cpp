// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "GLContext.hpp"

#include <memory>
#include <stdexcept>

#include "GLObject.hpp"
#include "GLShaderProgram.hpp"

#include <SDL3/SDL.h>
#include <glad/gl45.h>

#include "tcob/core/Logger.hpp"

namespace tcob::gfx::gl45 {

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
#if defined(TCOB_DEBUG)
static void GLAD_API_PTR debugCallback(GLenum /*source*/, GLenum /*type*/, GLuint /*id*/, GLenum severity, GLsizei, GLchar const* message, void const*)
{
    if (severity != GL_DEBUG_SEVERITY_NOTIFICATION) {
        logger::Error("GL: error {}", message);
    }
}

#endif

static auto load(char const* c) -> GLADapiproc
{
    return reinterpret_cast<GLADapiproc>(SDL_GL_GetProcAddress(c));
}
}

gl_context::gl_context(SDL_Window* window)
{
    // Create OpenGL context
    i32 const glMajor {4}, glMinor {5};
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, glMajor);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, glMinor);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#if defined(TCOB_DEBUG)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif

    logger::Info("GLContext: want OpenGL version: {}.{}", glMajor, glMinor);

    _context = SDL_GL_CreateContext(window); // NOLINT(cppcoreguidelines-prefer-member-initializer)
    if (!_context) {
        logger::Error("GLContext: OpenGL context creation failed!");
        throw std::runtime_error("OpenGL context creation failed");
    }

    if (!gladLoadGL(&load)) {
        SDL_GL_DestroyContext(static_cast<SDL_GLContext>(_context));
        logger::Error("GLContext: OpenGL loading failed!");
        throw std::runtime_error("OpenGL loading failed");
    }

    i32 checkMajor {}, checkMinor {};
    glGetIntegerv(GL_MAJOR_VERSION, &checkMajor);
    glGetIntegerv(GL_MINOR_VERSION, &checkMinor);

    logger::Info("GLContext: have OpenGL version: {}.{}", checkMajor, checkMinor);

#if defined(TCOB_DEBUG)
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(&debugCallback, nullptr);
#endif

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

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

gl_context::~gl_context()
{
    gl_object::DestroyAll();
    SDL_GL_DestroyContext(static_cast<SDL_GLContext>(_context));
}

}
