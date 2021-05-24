// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT
#include <tcob/gfx/gl/GLContext.hpp>

#include <SDL2/SDL.h>
#include <glad/gl.h>

#include <tcob/core/io/Logger.hpp>
#include <tcob/gfx/gl/GLObject.hpp>

namespace tcob::gl {

void debugCallback([[maybe_unused]] GLenum source, [[maybe_unused]] GLenum type, [[maybe_unused]] GLuint id, GLenum severity, GLsizei, const GLchar* message, const void*)
{
    if (severity != GL_DEBUG_SEVERITY_NOTIFICATION)
        Log(message);
}

Context::Context(SDL_Window* window)
{
    constexpr i32 glMajor { 4 }, glMinor { 5 };
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, glMajor);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, glMinor);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#if defined(_DEBUG)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif

    _context = SDL_GL_CreateContext(window);
    if (!_context) {
        Log("Error: OpenGL context creation failed!");
        throw std::runtime_error("OpenGL context creation failed");
    }

    if (!gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress)) {
        Log("Error: OpenGL loading failed!");
        throw std::runtime_error("OpenGL loading failed");
    }

    i32 checkMajor {}, checkMinor {};
    glGetIntegerv(GL_MAJOR_VERSION, &checkMajor);
    glGetIntegerv(GL_MINOR_VERSION, &checkMinor);

    Log("want OpenGL version: " + std::to_string(glMajor) + "." + std::to_string(glMinor));
    Log("have OpenGL version: " + std::to_string(checkMajor) + "." + std::to_string(checkMinor));

#if defined(_DEBUG)
    // glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(&debugCallback, nullptr);
#endif
}

Context::~Context()
{
    Object::DestroyAll();
    SDL_GL_DeleteContext(_context);
}

} // namespace gl
