// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "GLRenderSystem.hpp"

#include <SDL.h>

#include "GLRenderTarget.hpp"
#include "GLShaderProgram.hpp"
#include "GLTexture.hpp"
#include "GLUniformBuffer.hpp"
#include "GLVertexArray.hpp"
#include "GLWindow.hpp"
#include "nanovg/GLCanvas.hpp"

namespace tcob::gfx::gl45 {

auto gl_render_system::get_name() const -> string
{
    return "OPENGL45";
}

auto gl_render_system::get_capabilities() const -> capabilities
{
    capabilities retValue;

    std::array<f32, 2> val0 {};
    glGetFloatv(GL_POINT_SIZE_RANGE, val0.data());
    retValue.PointSizeRange = {val0[0], val0[1]};
    glGetFloatv(GL_POINT_SIZE_GRANULARITY, &retValue.PointSizeGranularity);
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &retValue.MaxTextureSize);
    glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &retValue.MaxArrayTextureLayers);

    return retValue;
}

auto gl_render_system::get_displays() const -> std::map<i32, display>
{
    std::map<i32, display> retValue;

    SDL_DisplayMode mode {};
    i32 const       numDisplays {SDL_GetNumVideoDisplays()};
    for (i32 i {0}; i < numDisplays; ++i) {
        i32 const numModes {SDL_GetNumDisplayModes(i)};
        for (i32 j {0}; j < numModes; ++j) {
            SDL_GetDisplayMode(i, j, &mode);
            retValue[i].Modes.push_back({.Size        = {mode.w, mode.h},
                                         .RefreshRate = mode.refresh_rate});
        }

        SDL_GetDesktopDisplayMode(i, &mode);
        retValue[i].DesktopMode = {.Size        = {mode.w, mode.h},
                                   .RefreshRate = mode.refresh_rate};
    }

    return retValue;
}

auto gl_render_system::get_rtt_coords() const -> rect_f
{
    return {0, 0, 1, -1};
}

auto gl_render_system::create_canvas() -> std::unique_ptr<render_backend::canvas_base>
{
    return std::make_unique<gl_canvas>();
}

auto gl_render_system::create_render_target(texture* tex) -> std::unique_ptr<render_backend::render_target_base>
{
    return std::make_unique<gl_render_target>(tex);
}

auto gl_render_system::create_shader() -> std::unique_ptr<render_backend::shader_base>
{
    return std::make_unique<gl_shader>();
}

auto gl_render_system::create_texture() -> std::unique_ptr<render_backend::texture_base>
{
    return std::make_unique<gl_texture>();
}

auto gl_render_system::create_uniform_buffer(usize size) -> std::unique_ptr<render_backend::uniform_buffer_base>
{
    return std::make_unique<gl_uniform_buffer>(size);
}

auto gl_render_system::create_vertex_array(buffer_usage_hint usage) -> std::unique_ptr<render_backend::vertex_array_base>
{
    return std::make_unique<gl_vertex_array>(usage);
}

auto gl_render_system::create_window(size_i size) -> std::unique_ptr<render_backend::window_base>
{
    return std::make_unique<gl_window>(size);
}

} // namespace gfx
