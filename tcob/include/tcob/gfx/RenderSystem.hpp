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
#include "tcob/core/Stats.hpp"
#include "tcob/core/TypeFactory.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/RenderSystemImpl.hpp"
#include "tcob/gfx/RenderTarget.hpp"
#include "tcob/gfx/Texture.hpp"
#include "tcob/gfx/Window.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

class TCOB_API render_system : public non_copyable {
public:
    struct factory : public type_factory<std::shared_ptr<render_system>> {
        static inline char const* ServiceName {"gfx::render_system::factory"};
    };

    struct capabilities {
        struct point_size {
            std::pair<f32, f32> Range;
            f32                 Granularity {};
        } PointSize;

        struct texture {
            i32 MaxSize {};
            i32 MaxLayers {};
        } Texture;
    };

    render_system();
    virtual ~render_system();

    template <typename T>
    auto init_window(video_config const& config, string const& windowTitle, size_i desktopResolution) -> window&;

    auto virtual name() const -> string        = 0;
    auto virtual device_name() const -> string = 0;
    auto virtual caps() const -> capabilities  = 0;
    auto virtual rtt_coords() const -> rect_f  = 0;

    auto stats() -> statistics&;

    auto window() const -> window&;
    auto default_target() const -> default_render_target&;

    auto virtual create_canvas [[nodiscard]] () -> std::unique_ptr<render_backend::canvas_base>                                    = 0;
    auto virtual create_render_target [[nodiscard]] (texture* tex) -> std::unique_ptr<render_backend::render_target_base>          = 0;
    auto virtual create_shader [[nodiscard]] () -> std::unique_ptr<render_backend::shader_base>                                    = 0;
    auto virtual create_texture [[nodiscard]] () -> std::unique_ptr<render_backend::texture_base>                                  = 0;
    auto virtual create_uniform_buffer [[nodiscard]] (usize size) -> std::unique_ptr<render_backend::uniform_buffer_base>          = 0;
    auto virtual create_vertex_array [[nodiscard]] (buffer_usage_hint usage) -> std::unique_ptr<render_backend::vertex_array_base> = 0;
    auto virtual create_window [[nodiscard]] (size_i size) -> std::unique_ptr<render_backend::window_base>                         = 0;

    static inline char const* ServiceName {"render_system"};

private:
    statistics                             _stats;
    std::unique_ptr<gfx::window>           _window;
    std::unique_ptr<default_render_target> _defaultTarget;
};

////////////////////////////////////////////////////////////

template <typename T>
inline auto render_system::init_window(video_config const& config, string const& windowTitle, size_i desktopResolution) -> gfx::window&
{
    size_i const resolution {config.UseDesktopResolution ? desktopResolution : config.Resolution};

    _window = std::make_unique<T>(create_window(resolution));

    _window->FullScreen(config.FullScreen || config.UseDesktopResolution);
    _window->VSync(config.VSync);
    _window->Size(resolution);
    _window->Title = windowTitle;

    _defaultTarget = std::make_unique<gfx::default_render_target>(_window.get());

    _window->clear();
    _window->draw_to(*_defaultTarget);
    _window->swap_buffer();

    return *_window;
}

}
