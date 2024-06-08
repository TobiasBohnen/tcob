// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <map>
#include <utility>
#include <vector>

#include "tcob/core/Interfaces.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/core/TypeFactory.hpp"
#include "tcob/gfx/RenderSystemImpl.hpp"

namespace tcob::gfx {

////////////////////////////////////////////////////////////

struct display_mode {
    size_i Size {size_i::Zero};
    i32    RefreshRate {0};
};

struct display {
    std::vector<display_mode> Modes;
    display_mode              DesktopMode;
};

////////////////////////////////////////////////////////////

class TCOB_API render_system : public non_copyable {
public:
    struct factory : public type_factory<std::shared_ptr<render_system>> {
        static inline char const* service_name {"gfx::render_system::factory"};
    };

    struct capabilities {
        std::pair<f32, f32> PointSizeRange;
        f32                 PointSizeGranularity {};
        i32                 MaxTextureSize {};
        i32                 MaxArrayTextureLayers {};
    };

    render_system()          = default;
    virtual ~render_system() = default;

    auto virtual get_name() const -> string                     = 0;
    auto virtual get_capabilities() const -> capabilities       = 0;
    auto virtual get_displays() const -> std::map<i32, display> = 0;
    auto virtual get_rtt_coords() const -> rect_f               = 0;

    auto virtual create_canvas [[nodiscard]] () -> std::unique_ptr<render_backend::canvas_base>                                    = 0;
    auto virtual create_render_target [[nodiscard]] (texture* tex) -> std::unique_ptr<render_backend::render_target_base>          = 0;
    auto virtual create_shader [[nodiscard]] () -> std::unique_ptr<render_backend::shader_base>                                    = 0;
    auto virtual create_texture [[nodiscard]] () -> std::unique_ptr<render_backend::texture_base>                                  = 0;
    auto virtual create_uniform_buffer [[nodiscard]] (usize size) -> std::unique_ptr<render_backend::uniform_buffer_base>          = 0;
    auto virtual create_vertex_array [[nodiscard]] (buffer_usage_hint usage) -> std::unique_ptr<render_backend::vertex_array_base> = 0;
    auto virtual create_window [[nodiscard]] (size_i size) -> std::unique_ptr<render_backend::window_base>                         = 0;

    static inline char const* service_name {"render_system"};
};

}
