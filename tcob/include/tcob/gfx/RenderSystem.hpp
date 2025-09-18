// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <memory>
#include <utility>

#include "tcob/core/Interfaces.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/core/TypeFactory.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/RenderSystemImpl.hpp"
#include "tcob/gfx/RenderTarget.hpp"
#include "tcob/gfx/Stats.hpp"
#include "tcob/gfx/Texture.hpp"
#include "tcob/gfx/Window.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

struct render_capabilities {
    struct point_size {
        std::pair<f32, f32> Range;
        f32                 Granularity {};
    } PointSize;

    struct texture {
        i32 MaxSize {};
        i32 MaxLayers {};
    } Texture;

    rect_f RenderTextureUVRect;
};

////////////////////////////////////////////////////////////

class TCOB_API render_system : public non_copyable {
public:
    struct factory : public type_factory<std::shared_ptr<render_system>> {
        static inline char const* ServiceName {"gfx::render_system::factory"};
    };

    render_system();
    virtual ~render_system();

    auto init_window(video_config const& config, string const& windowTitle, size_i desktopResolution) -> window&;

    virtual auto name() const -> string                      = 0;
    virtual auto device_name() const -> string               = 0;
    virtual auto capabilities() const -> render_capabilities = 0;

    auto statistics() -> render_statistics&;

    auto window() const -> window&;
    auto default_target() const -> default_render_target&;

    virtual auto create_canvas [[nodiscard]] () -> std::unique_ptr<render_backend::canvas_base>                                    = 0;
    virtual auto create_render_target [[nodiscard]] (texture* tex) -> std::unique_ptr<render_backend::render_target_base>          = 0;
    virtual auto create_shader [[nodiscard]] () -> std::unique_ptr<render_backend::shader_base>                                    = 0;
    virtual auto create_texture [[nodiscard]] () -> std::unique_ptr<render_backend::texture_base>                                  = 0;
    virtual auto create_uniform_buffer [[nodiscard]] (usize size) -> std::unique_ptr<render_backend::uniform_buffer_base>          = 0;
    virtual auto create_vertex_array [[nodiscard]] (buffer_usage_hint usage) -> std::unique_ptr<render_backend::vertex_array_base> = 0;
    virtual auto create_window [[nodiscard]] (size_i size) -> std::unique_ptr<gfx::window>                                         = 0;

    static inline char const* ServiceName {"render_system"};

private:
    render_statistics                      _stats;
    std::unique_ptr<gfx::window>           _window;
    std::unique_ptr<default_render_target> _defaultTarget;
};

}
