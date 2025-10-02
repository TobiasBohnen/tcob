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

#include "tcob/core/Rect.hpp"
#include "tcob/core/Serialization.hpp"
#include "tcob/core/Size.hpp"

namespace tcob::Cfg::Video {
constexpr char const* const Name {"video"};
constexpr char const* const fullscreen {"fullscreen"};
constexpr char const* const use_desktop_resolution {"use_desktop_resolution"};
constexpr char const* const resolution {"resolution"};
constexpr char const* const frame_limit {"frame_limit"};
constexpr char const* const vsync {"vsync"};
constexpr char const* const render_system {"render_system"};
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

class TCOB_API blend_funcs final {
public:
    blend_func SourceColorBlendFunc {blend_func::SrcAlpha};
    blend_func DestinationColorBlendFunc {blend_func::OneMinusSrcAlpha};
    blend_func SourceAlphaBlendFunc {blend_func::SrcAlpha};
    blend_func DestinationAlphaBlendFunc {blend_func::OneMinusSrcAlpha};

    auto operator==(blend_funcs const& other) const -> bool = default;

    static auto constexpr Members()
    {
        return std::tuple {
            member<&blend_funcs::SourceColorBlendFunc> {"src_color"},
            member<&blend_funcs::DestinationColorBlendFunc> {"dst_color"},
            member<&blend_funcs::SourceAlphaBlendFunc> {"src_alpha"},
            member<&blend_funcs::DestinationAlphaBlendFunc> {"dst_alpha"}};
    }
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

class TCOB_API uv final {
public:
    f32 U {0.0f};
    f32 V {0.0f};
    f32 Level {0.0f};

    auto operator==(uv const& other) const -> bool = default;

    static auto constexpr Members()
    {
        return std::tuple {
            member<&uv::U> {"u"},
            member<&uv::V> {"v"},
            member<&uv::Level> {"level"}};
    }
};

////////////////////////////////////////////////////////////

class TCOB_API texture_region final {
public:
    rect_f UVRect {rect_f::Zero};
    u32    Level {0};

    auto operator==(texture_region const& other) const -> bool = default;

    static auto constexpr Members()
    {
        return std::tuple {
            member<&texture_region::UVRect> {"rect"},
            member<&texture_region::Level> {"level"}};
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

    static auto constexpr Members()
    {
        return std::tuple {
            member<&alignments::Horizontal> {"horizontal"},
            member<&alignments::Vertical> {"vertical"}};
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

    static auto constexpr Members()
    {
        return std::tuple {
            member<&video_config::FullScreen> {Cfg::Video::fullscreen},
            member<&video_config::UseDesktopResolution> {Cfg::Video::use_desktop_resolution},
            member<&video_config::Resolution> {Cfg::Video::resolution},
            member<&video_config::FrameLimit> {Cfg::Video::frame_limit},
            member<&video_config::VSync> {Cfg::Video::vsync},
            member<&video_config::RenderSystem> {Cfg::Video::render_system}};
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
    class tilemap_base;
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
