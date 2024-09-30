// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

// based on NanoVG/NanoSVG
// original license:
// Copyright (c) 2013 Mikko Mononen memon@inside.org
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//
#pragma once
#include "tcob/tcob_config.hpp"

#include <span>
#include <unordered_map>
#include <variant>
#include <vector>

#include "tcob/core/AngleUnits.hpp"
#include "tcob/core/Color.hpp"
#include "tcob/core/Interfaces.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Transform.hpp"
#include "tcob/core/assets/Asset.hpp"
#include "tcob/gfx/ColorGradient.hpp"
#include "tcob/gfx/Font.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/RenderTexture.hpp"
#include "tcob/gfx/TextFormatter.hpp"
#include "tcob/gfx/Texture.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

enum class solidity : u8 {
    Solid = 1,
    Hole  = 2,
};

enum class line_cap : u8 {
    Butt   = 0,
    Round  = 1,
    Square = 2
};

enum class line_join : u8 {
    Round = 1,
    Bevel = 3,
    Miter = 4,
};

enum class composite_operation : u8 {
    SourceOver,
    SourceIn,
    SourceOut,
    Atop,
    DestinationOver,
    DestinationIn,
    DestinationOut,
    DestinationAtop,
    Lighter,
    Copy,
    Xor,
};

////////////////////////////////////////////////////////////
using paint_gradient = std::pair<f32, color_gradient>;
using paint_color    = std::variant<color, paint_gradient>;

struct canvas_paint {
    transform   XForm {transform::Identity};
    vec2        Extent {0, 0};
    f32         Radius {0};
    f32         Feather {0};
    paint_color Color {colors::White};
    texture*    Image {nullptr};
};

struct canvas_scissor {
    transform XForm {transform::Identity};
    vec2      Extent {};
};

struct canvas_path {
    i32     First {0};
    usize   Count {0};
    usize   BevelCount {0};
    vertex* Fill {nullptr};
    usize   FillCount {0};
    vertex* Stroke {nullptr};
    usize   StrokeCount {0};
    winding Winding {winding::CCW};
    bool    Convex {false};
    bool    Closed {false};
};

namespace detail {
    struct canvas_point {
        f32   X {0}, Y {0};
        f32   DX {0}, DY {0};
        f32   Length {0};
        f32   DMX {0}, DMY {0};
        ubyte Flags {0};
    };
}

////////////////////////////////////////////////////////////

namespace render_backend {
    class canvas_base;
}

////////////////////////////////////////////////////////////

class TCOB_API canvas final : public non_copyable {
public:
    ////////////////////////////////////////////////////////////

    class TCOB_API state_guard final : public non_copyable {
    public:
        explicit state_guard(canvas* c);
        ~state_guard();

    private:
        canvas* _canvas;
    };

    ////////////////////////////////////////////////////////////

    class TCOB_API path2d {
    public:
        auto static Parse(string_view path) -> std::optional<path2d>;

        std::vector<std::function<void(canvas&)>> Commands;

    private:
        auto static GetCommands(string_view path) -> std::optional<std::vector<std::variant<char, f32>>>;
    };

    ////////////////////////////////////////////////////////////

    canvas();
    ~canvas() = default;

    auto get_texture(i32 level = 0) -> assets::asset_ptr<texture>;
    auto get_size() const -> size_i;

    void begin_frame(size_i windowSize, f32 devicePixelRatio, i32 rtt = 0);
    void end_frame();
    void cancel_frame();

    void set_global_composite_operation(composite_operation op);
    void set_global_composite_blendfunc(blend_func sfactor, blend_func dfactor);
    void set_global_composite_blendfunc_separate(blend_func srcRGB, blend_func dstRGB, blend_func srcAlpha, blend_func dstAlpha);
    void set_global_enforce_path_winding(bool force);

    // State handling
    void save();
    void restore();
    void reset();
    auto create_guard() -> state_guard;

    // Render styles
    void set_fill_style(color c);
    void set_fill_style(canvas_paint const& paint);
    void set_stroke_style(color c);
    void set_stroke_style(canvas_paint const& paint);
    void set_stroke_width(f32 size);
    void set_edge_antialias(bool enabled);
    void set_shape_antialias(bool enabled);
    void set_miter_limit(f32 limit);
    void set_line_cap(line_cap cap);
    void set_line_join(line_join join);
    void set_global_alpha(f32 alpha);

    // Paths
    void begin_path();
    void close_path();
    void set_path_winding(winding dir);
    void set_path_winding(solidity s);
    void move_to(point_f pos);
    void line_to(point_f pos);
    void cubic_bezier_to(point_f cp0, point_f cp1, point_f end);
    void quad_bezier_to(point_f cp, point_f end);
    void arc_to(point_f pos1, point_f pos2, f32 radius);
    void arc(point_f c, f32 r, radian_f startAngle, radian_f endAngle, winding dir);
    void rect(rect_f const& rect);
    void rounded_rect(rect_f const& rect, f32 r);
    void rounded_rect_varying(rect_f const& rect, f32 radTL, f32 radTR, f32 radBR, f32 radBL);
    void ellipse(point_f c, f32 rx, f32 ry);
    void circle(point_f c, f32 r);

    void dotted_cubic_bezier(point_f begin, point_f cp0, point_f cp1, point_f end, f32 r, i32 numDots);
    void dotted_quad_bezier(point_f begin, point_f cp, point_f end, f32 r, i32 numDots);
    void dotted_line(point_f from, point_f to, f32 r, i32 numDots);
    void dotted_circle(point_f center, f32 rcircle, f32 rdots, i32 numDots);
    void wavy_line(point_f from, point_f to, f32 amp, f32 freq, f32 phase = 0.f);
    void regular_polygon(point_f pos, size_f size, i32 n);
    void star(point_f pos, f32 outerR, f32 innerR, i32 n);
    void triangle(point_f a, point_f b, point_f c);

    void path_2d(path2d const& path);

    void fill();
    void stroke();

    // Paints
    auto create_linear_gradient [[nodiscard]] (point_f s, point_f e, color_gradient const& gradient) -> canvas_paint;
    auto create_box_gradient [[nodiscard]] (rect_f const& rect, f32 r, f32 f, color_gradient const& gradient) -> canvas_paint;
    auto create_radial_gradient [[nodiscard]] (point_f c, f32 inr, f32 outr, color_gradient const& gradient) -> canvas_paint;
    auto create_radial_gradient [[nodiscard]] (point_f c, f32 inr, f32 outr, size_f scale, color_gradient const& gradient) -> canvas_paint;
    auto create_image_pattern [[nodiscard]] (point_f c, size_f e, degree_f angle, texture* image, f32 alpha) -> canvas_paint;

    // Image
    void draw_image(texture* image, string const& region, rect_f const& rect);
    void draw_nine_patch(texture* image, string const& region, rect_f const& rect, point_f offsetCenterLT, point_f offsetCenterRB, rect_f const& localCenterUV);
    void draw_nine_patch(texture* image, string const& region, rect_f const& rect, rect_f const& center, rect_f const& localCenterUV);

    // Transforms
    void translate(point_f c);
    void rotate(degree_f angle);
    void rotate_at(degree_f angle, point_f p);
    void scale(size_f scale);
    void scale_at(size_f scale, point_f p);
    void skew(degree_f angleX, degree_f angleY);
    void skew_at(degree_f angleX, degree_f angleY, point_f p);
    void set_transform(transform xform);
    void reset_transform();

    // Scissoring
    void set_scissor(rect_f const& rect, bool transform = true);
    void reset_scissor();

    // Font
    void set_font(font* font);
    void set_text_halign(horizontal_alignment align);
    void set_text_valign(vertical_alignment align);

    void draw_textbox(rect_f const& rect, utf8_string_view text);
    void draw_textbox(point_f offset, text_formatter::result const& formatResult);
    auto format_text(size_f const& size, utf8_string_view text, f32 scale = 1.0f) -> text_formatter::result;
    auto measure_text(f32 height, utf8_string_view text) -> size_f;

    // fill
    void fill_lines(std::span<point_f const> points);

    // stroke
    void stroke_line(point_f from, point_f to);
    void stroke_lines(std::span<point_f const> points);

    void stroke_dashed_cubic_bezier(point_f start, point_f cp0, point_f cp1, point_f end, i32 numDashes);
    void stroke_dashed_quad_bezier(point_f start, point_f cp, point_f end, i32 numDashes);
    void stroke_dashed_line(point_f from, point_f to, i32 numDashes);
    void stroke_dashed_circle(point_f center, f32 r, i32 numDashes);

    auto get_impl() const -> render_backend::canvas_base*;

private:
    struct state {
        blend_funcs    CompositeOperation {};
        bool           ShapeAntiAlias {true};
        canvas_paint   Fill {};
        canvas_paint   Stroke {};
        f32            StrokeWidth {1};
        f32            MiterLimit {10.0f};
        line_join      LineJoin {line_join::Miter};
        line_cap       LineCap {line_cap::Butt};
        f32            Alpha {1};
        transform      XForm {transform::Identity};
        canvas_scissor Scissor {};
        alignments     TextAlign {};
        font*          Font {nullptr};
    };

    struct path_cache {
        std::vector<detail::canvas_point> points;
        std::vector<canvas_path>          paths;
        std::vector<vertex>               verts;

        vec4 bounds;
    };

    void set_device_pixel_ratio(f32 ratio);
    auto get_state() -> state&;
    void set_paint_color(canvas_paint& p, color c);
    auto get_font_scale() -> f32;
    void append_commands(std::span<f32 const> vals);
    void clear_path_cache();
    auto get_last_path() -> canvas_path&;
    void add_path();
    auto get_last_point() -> detail::canvas_point&;
    void add_point(f32 x, f32 y, i32 flags);
    void close_last_path();
    auto alloc_temp_verts(usize nverts) -> vertex*;
    void tesselate_bezier(f32 x1, f32 y1, f32 x2, f32 y2, f32 x3, f32 y3, f32 x4, f32 y4, i32 level, i32 type);
    void flatten_paths();
    void calculate_joins(f32 w, line_join lineJoin, f32 miterLimit);
    void expand_stroke(f32 w, f32 fringe, line_cap lineCap, line_join lineJoin, f32 miterLimit);
    void expand_fill(f32 w, line_join lineJoin, f32 miterLimit);
    void render_text(font* font, std::span<vertex const> verts);
    void path_arc_to(f32 x1, f32 y1, std::vector<f32> const& args, bool rel);

    std::unique_ptr<render_backend::canvas_base> _impl {};
    std::vector<f32>                             _commands {};
    point_f                                      _commandPoint {point_f::Zero};
    std::stack<state>                            _states {};
    path_cache                                   _cache {};

    f32    _tessTol {0};
    f32    _distTol {0};
    f32    _fringeWidth {0};
    f32    _devicePxRatio {0};
    size_i _windowSize;

    bool _edgeAntiAlias {true};
    bool _enforceWinding {true};

    std::unordered_map<i32, assets::manual_asset_ptr<render_texture>> _rtt {};
    i32                                                               _activeRtt {0};
};
}
