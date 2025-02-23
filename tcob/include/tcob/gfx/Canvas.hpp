// Copyright (c) 2025 Tobias Bohnen
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
#include "tcob/core/assets/Asset.hpp"
#include "tcob/gfx/ColorGradient.hpp"
#include "tcob/gfx/Font.hpp"
#include "tcob/gfx/Geometry.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/Path2d.hpp"
#include "tcob/gfx/RenderTexture.hpp"
#include "tcob/gfx/TextFormatter.hpp"
#include "tcob/gfx/Texture.hpp"
#include "tcob/gfx/Transform.hpp"

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
using paint_gradient = std::pair<f32, i32>;
using paint_color    = std::variant<color, paint_gradient>;

////////////////////////////////////////////////////////////

class path_cache;
class states;

////////////////////////////////////////////////////////////

class TCOB_API canvas final : public non_copyable {
public:
    ////////////////////////////////////////////////////////////

    struct paint {
        transform   XForm {transform::Identity};
        size_f      Extent {size_f::Zero};
        f32         Radius {0};
        f32         Feather {0};
        paint_color Color {colors::White};
        texture*    Image {nullptr};
    };

    struct scissor {
        transform XForm {transform::Identity};
        size_f    Extent {size_f::Zero};
    };

    struct path {
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

    ////////////////////////////////////////////////////////////

    class TCOB_API state_guard final : public non_copyable {
    public:
        explicit state_guard(canvas* c);
        ~state_guard();

    private:
        canvas* _canvas;
    };

    ////////////////////////////////////////////////////////////

    canvas();
    ~canvas();

    auto get_texture(i32 level = 0) -> assets::asset_ptr<texture>;

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
    void set_fill_style(paint const& paint);
    void set_stroke_style(color c);
    void set_stroke_style(paint const& paint);
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
    void set_line_dash(std::span<f32 const> dashPattern);
    void set_dash_offset(f32 offset);
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

    void fill();
    void stroke();

    // Extras
    void wavy_line_to(point_f to, f32 amp, f32 freq, f32 phase = 0.f);
    void regular_polygon(point_f pos, size_f size, i32 n);
    void star(point_f pos, f32 outerR, f32 innerR, i32 n);
    void triangle(point_f a, point_f b, point_f c);

    void path_2d(path2d const& path);

    void fill_lines(std::span<point_f const> points);
    void stroke_line(point_f from, point_f to);
    void stroke_lines(std::span<point_f const> points);

    // Paints
    auto create_linear_gradient [[nodiscard]] (point_f s, point_f e, color_gradient const& gradient) -> paint;
    auto create_box_gradient [[nodiscard]] (rect_f const& rect, f32 r, f32 f, color_gradient const& gradient) -> paint;
    auto create_radial_gradient [[nodiscard]] (point_f c, f32 inr, f32 outr, color_gradient const& gradient) -> paint;
    auto create_radial_gradient [[nodiscard]] (point_f c, f32 inr, f32 outr, size_f scale, color_gradient const& gradient) -> paint;
    auto create_image_pattern [[nodiscard]] (point_f c, size_f e, degree_f angle, texture* image, f32 alpha) -> paint;

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
    void set_transform(transform const& xform);
    void reset_transform();

    // Scissoring
    void set_scissor(rect_f const& rect, bool transform = true);
    void reset_scissor();

    // Font
    void set_font(font* font);
    void set_text_halign(horizontal_alignment align);
    void set_text_valign(vertical_alignment align);

    void draw_text(rect_f const& rect, utf8_string_view text);
    void draw_text(point_f offset, text_formatter::result const& formatResult);

    void fill_text(utf8_string_view text, point_f offset);
    void stroke_text(utf8_string_view text, point_f offset);

    auto format_text(size_f const& size, utf8_string_view text, f32 scale = 1.0f) -> text_formatter::result;
    auto measure_text(f32 height, utf8_string_view text) -> size_f;

    auto get_impl() const -> render_backend::canvas_base*;

private:
    void set_device_pixel_ratio(f32 ratio);

    void set_paint_color(paint& p, color c);
    auto get_font_scale() -> f32;
    void render_text(font* font, std::span<vertex const> verts);
    void decompose_text(utf8_string_view text, point_f offset);
    auto create_gradient(color_gradient const& gradient) -> paint_color;

    std::unique_ptr<render_backend::canvas_base> _impl {};

    std::unique_ptr<states> _states {};

    std::unique_ptr<path_cache> _cache {};
    std::vector<color_gradient> _gradients;

    f32 _fringeWidth {0};

    f32    _devicePxRatio {0};
    size_i _windowSize;

    bool _edgeAntiAlias {true};
    bool _enforceWinding {true};

    std::unordered_map<i32, assets::owning_asset_ptr<render_texture>> _rtt {};
    i32                                                               _activeRtt {0};
};
}
