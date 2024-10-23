// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

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

struct texture_region final {
    rect_f UVRect {rect_f::Zero};
    u32    Level {0};
};

void Serialize(texture_region const& v, auto&& s)
{
    s["level"] = v.Level;
    Serialize(v.UVRect, s);
}

auto Deserialize(texture_region& v, auto&& s) -> bool
{
    return s.try_get(v.Level, "level") && Deserialize(v.UVRect, s);
}

////////////////////////////////////////////////////////////

enum class orientation : u8 {
    Horizontal,
    Vertical
};

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

struct alignments {
    horizontal_alignment Horizontal {horizontal_alignment::Left};
    vertical_alignment   Vertical {vertical_alignment::Top};

    auto operator==(alignments const& other) const -> bool = default;
};

void Serialize(alignments const& v, auto&& s)
{
    s["horizontal"] = v.Horizontal;
    s["vertical"]   = v.Vertical;
}

auto Deserialize(alignments& v, auto&& s) -> bool
{
    return s.try_get(v.Horizontal, "horizontal") && s.try_get(v.Vertical, "vertical");
}

////////////////////////////////////////////////////////////

struct video_config {
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
};

void Serialize(video_config const& v, auto&& s)
{
    s[Cfg::Video::fullscreen]             = v.FullScreen;
    s[Cfg::Video::use_desktop_resolution] = v.UseDesktopResolution;
    s[Cfg::Video::resolution]             = v.Resolution;
    s[Cfg::Video::frame_limit]            = v.FrameLimit;
    s[Cfg::Video::vsync]                  = v.VSync;
    s[Cfg::Video::render_system]          = v.RenderSystem;
}

auto Deserialize(video_config& v, auto&& s) -> bool
{
    return s.try_get(v.FullScreen, Cfg::Video::fullscreen)
        && s.try_get(v.UseDesktopResolution, Cfg::Video::use_desktop_resolution)
        && s.try_get(v.Resolution, Cfg::Video::resolution)
        && s.try_get(v.FrameLimit, Cfg::Video::frame_limit)
        && s.try_get(v.VSync, Cfg::Video::vsync)
        && s.try_get(v.RenderSystem, Cfg::Video::render_system);
}

}

////////////////////////////////////////////////////////////
// forward declarations

namespace tcob {
namespace gfx {
    class canvas;
    struct canvas_paint;
    struct canvas_scissor;
    struct canvas_path;
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
