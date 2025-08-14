// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <compare>
#include <functional>
#include <set>
#include <tuple>
#include <utility>

#include "tcob/core/Rect.hpp"
#include "tcob/core/Size.hpp"

namespace tcob::Cfg::Video {
char const* const Name {"video"};
char const* const fullscreen {"fullscreen"};
char const* const use_desktop_resolution {"use_desktop_resolution"};
char const* const resolution {"resolution"};
char const* const frame_limit {"frame_limit"};
char const* const vsync {"vsync"};
char const* const render_system {"render_system"};
}

namespace tcob::gfx {

////////////////////////////////////////////////////////////

enum class winding : u8 {
    CCW = 1, // Winding for solid shapes
    CW  = 2, // Winding for holes
};

enum class buffer_usage_hint : u8 {
    DynamicDraw,
    StaticDraw,
    StreamDraw
};

enum class blend_func : u8 {
    Invalid,
    Zero,
    One,
    SrcColor,
    OneMinusSrcColor,
    DstColor,
    OneMinusDstColor,
    SrcAlpha,
    OneMinusSrcAlpha,
    DstAlpha,
    OneMinusDstAlpha,
    ConstantColor,
    OneMinusConstantColor,
    ConstantAlpha,
    OneMinusConstantAlpha
};

struct blend_funcs {
    blend_func SourceColorBlendFunc {blend_func::SrcAlpha};
    blend_func DestinationColorBlendFunc {blend_func::OneMinusSrcAlpha};
    blend_func SourceAlphaBlendFunc {blend_func::SrcAlpha};
    blend_func DestinationAlphaBlendFunc {blend_func::OneMinusSrcAlpha};

    auto operator==(blend_funcs const& other) const -> bool = default;
};

enum class blend_equation : u8 {
    Add,
    Subtract,
    ReverseSubtract,
    Min,
    Max
};

enum class stencil_func : u8 {
    Never,
    Equal,
    NotEqual,
    Less,
    LessEqual,
    Greater,
    GreaterEqual,
    Always
};

enum class stencil_op : u8 {
    Keep,
    Zero,
    Replace,
    Increase,
    Decrease,
    Invert,
    IncreaseWrap,
    DecreaseWrap
};

enum class primitive_type : u8 {
    Points,
    LineStrip,
    LineLoop,
    Lines,
    TriangleStrip,
    TriangleFan,
    Triangles
};

////////////////////////////////////////////////////////////

struct uv {
    f32 U {0.f};
    f32 V {0.f};
    f32 Level {0.f};

    auto operator==(uv const& other) const -> bool = default;
};

////////////////////////////////////////////////////////////

class TCOB_API texture_region final {
public:
    rect_f UVRect {rect_f::Zero};
    u32    Level {0};

    auto operator==(texture_region const& other) const -> bool = default;

    auto constexpr static Members()
    {
        return std::tuple {
            std::pair {"rect", &texture_region::UVRect},
            std::pair {"level", &texture_region::Level}};
    }
};

////////////////////////////////////////////////////////////

enum class horizontal_alignment : u8 {
    Left,
    Right,
    Centered
};

enum class vertical_alignment : u8 {
    Top,
    Middle,
    Bottom
};

////////////////////////////////////////////////////////////

class TCOB_API alignments {
public:
    horizontal_alignment Horizontal {horizontal_alignment::Left};
    vertical_alignment   Vertical {vertical_alignment::Top};

    auto operator==(alignments const& other) const -> bool = default;

    auto constexpr static Members()
    {
        return std::tuple {
            std::pair {"horizontal", &alignments::Horizontal},
            std::pair {"vertical", &alignments::Vertical}};
    }
};

////////////////////////////////////////////////////////////

class TCOB_API video_config {
public:
#if defined(TCOB_DEBUG)
    bool   FullScreen {false};
    bool   UseDesktopResolution {false};
    size_i Resolution {1600, 900};
#else
    bool   FullScreen {true};
    bool   UseDesktopResolution {true};
    size_i Resolution {};
#endif
    i32  FrameLimit {6000};
    bool VSync {false};
#if defined(__EMSCRIPTEN__)
    string RenderSystem {"OPENGLES30"};
#else
    string RenderSystem {"OPENGL45"};
#endif

    auto operator==(video_config const& other) const -> bool = default;

    auto constexpr static Members()
    {
        return std::tuple {
            std::pair {Cfg::Video::fullscreen, &video_config::FullScreen},
            std::pair {Cfg::Video::use_desktop_resolution, &video_config::UseDesktopResolution},
            std::pair {Cfg::Video::resolution, &video_config::Resolution},
            std::pair {Cfg::Video::frame_limit, &video_config::FrameLimit},
            std::pair {Cfg::Video::vsync, &video_config::VSync},
            std::pair {Cfg::Video::render_system, &video_config::RenderSystem}};
    }
};

////////////////////////////////////////////////////////////

class TCOB_API display_mode {
public:
    size_i Size {size_i::Zero};
    f32    PixelDensity {0.0f};
    f32    RefreshRate {0.0f};

    auto operator==(display_mode const& other) const -> bool = default;
    auto operator<=>(display_mode const& other) const -> std::partial_ordering;
};

auto inline display_mode::operator<=>(display_mode const& other) const -> std::partial_ordering
{
    if (auto const cmp {Size.Width <=> other.Size.Width}; cmp != 0) { return cmp; }
    if (auto const cmp {Size.Height <=> other.Size.Height}; cmp != 0) { return cmp; }
    if (auto const cmp {RefreshRate <=> other.RefreshRate}; cmp != 0) { return cmp; }
    return other.PixelDensity <=> PixelDensity;
}

struct display {
    std::set<display_mode, std::greater<>> Modes;
    display_mode                           DesktopMode;
};
}

////////////////////////////////////////////////////////////
// forward declarations

namespace tcob {
namespace gfx {
    class background;
    class canvas;
    class font;
    class material;
    class render_system;
    class render_target;
    struct render_properties;
    class window;
}
namespace gfx::render_backend {
    class canvas_base;
    class render_target_base;
    class shader_base;
    class texture_base;
    class uniform_buffer_base;
    class vertex_array_base;
    class window_base;
}
}
