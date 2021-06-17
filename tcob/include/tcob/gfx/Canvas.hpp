// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

// based on NanoVG
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
#include <tcob/tcob_config.hpp>

#include <map>

#include <tcob/assets/Resource.hpp>
#include <tcob/core/data/Color.hpp>
#include <tcob/core/data/Point.hpp>
#include <tcob/core/data/Rect.hpp>
#include <tcob/core/data/Transform.hpp>
#include <tcob/gfx/drawables/Text.hpp>
#include <tcob/gfx/gl/GLEnum.hpp>

namespace tcob {
namespace detail {
    class GLNVGcontext;
}

enum class Winding : char {
    CCW = 1, // Winding for solid shapes
    CW = 2, // Winding for holes
};

enum class LineCap : char {
    Butt = 0,
    Round = 1,
    Square = 2
};

enum class LineJoin : char {
    Round = 1,
    Bevel = 3,
    Miter = 4,
};

enum class Solidity : char {
    Solid = 1,
    Hole = 2,
};

enum class CompositeOperation {
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

struct ColorStop {
    f32 Position;
    Color Value;
};

////////////////////////////////////////////////////////////

template <u32 Size>
class ColorGradient final {
public:
    ColorGradient(const Color& color, bool preMulAlpha = true)
        : ColorGradient { color, color, preMulAlpha }
    {
    }

    ColorGradient(const Color& startColor, const Color& endColor, bool preMulAlpha = true)
        : _premulAlpha { preMulAlpha }
    {
        static_assert(Size > 0);

        _colorStops[0] = startColor;
        if constexpr (Size > 1) {
            _colorStops[1] = endColor;
        }

        calculate();
    }

    ColorGradient(const std::vector<ColorStop>& colorStops, bool preMulAlpha = true)
        : _premulAlpha { preMulAlpha }
    {
        static_assert(Size > 0);

        for (const auto& cs : colorStops) {
            auto pos { cs.Position };
            pos = std::clamp(pos, 0.0f, 1.0f);
            _colorStops[pos] = cs.Value;
        }

        calculate();
    }

    auto colors() const -> std::array<vec4, Size>
    {
        return _colors;
    }

    void multiply_alpha(f32 alpha)
    {
        for (vec4& color : _colors) {
            color[3] *= alpha;
        }
    }

    auto is_single_color() const -> bool
    {
        return _colorStops.size() == 2 && _colorStops.at(0) == _colorStops.at(1);
    }

private:
    void calculate()
    {
        if (is_single_color()) {
            Color c { _colorStops.at(0) };
            if (_premulAlpha) {
                c = c.premultiply_alpha();
            }
            _colors.fill({ c.R / 255.f, c.G / 255.f, c.B / 255.f, c.A / 255.f });
        } else if (_colorStops.size() >= 2) {
            auto it2 { _colorStops.cbegin() };
            auto it1 { it2++ };
            while (it2 != _colorStops.cend()) {
                const auto [k1, col1] = *it1;
                const auto [k2, col2] = *it2;

                const i32 start { static_cast<i32>(k1 * (Size - 1)) };
                const i32 size { static_cast<i32>(k2 * (Size - 1)) - start };

                for (i32 i { 0 }; i <= size; ++i) {
                    Color c { col1.interpolate(col2, static_cast<f32>(i) / size) };
                    if (_premulAlpha) {
                        c = c.premultiply_alpha();
                    }
                    _colors[i + start] = { c.R / 255.f, c.G / 255.f, c.B / 255.f, c.A / 255.f };
                }

                it1 = it2;
                ++it2;
            }
        }
    }

    std::map<f32, Color> _colorStops;
    std::array<vec4, Size> _colors;
    bool _premulAlpha;
};

////////////////////////////////////////////////////////////

struct CanvasPaint {
    Transform xform;
    vec2 extent;
    f32 radius { 0 };
    f32 feather { 0 };
    ColorGradient<256> gradient { Colors::White, Colors::White };
    gl::Texture2D* image { nullptr };
    Color text_outline_color {};
    float text_outline_thickness { 0 };
};

namespace detail {
    struct NVGscissor {
        Transform xform;
        vec2 extent;
    };

    struct NVGstate {
        gl::BlendFuncs compositeOperation;
        bool shapeAntiAlias { false };
        CanvasPaint fill;
        CanvasPaint stroke;
        f32 strokeWidth { 0 };
        f32 miterLimit { 0 };
        LineJoin lineJoin { LineJoin::Round };
        LineCap lineCap { LineCap::Butt };
        f32 alpha { 0 };
        Transform xform {};
        NVGscissor scissor {};
        TextAlignment textAlign { TextAlignment::Left };
        isize fontId { 0 };
    };

    struct NVGpoint {
        f32 x { 0 }, y { 0 };
        f32 dx { 0 }, dy { 0 };
        f32 len { 0 };
        f32 dmx { 0 }, dmy { 0 };
        ubyte flags { 0 };
    };

    struct NVGpath {
        i32 first { 0 };
        isize count { 0 };
        bool closed { false };
        isize nbevel { 0 };
        Vertex* fill { nullptr };
        isize nfill { 0 };
        Vertex* stroke { nullptr };
        isize nstroke { 0 };
        Winding winding { Winding::CCW };
        bool convex { false };
    };

    struct NVGpathCache {
        std::vector<NVGpoint> points;
        std::vector<NVGpath> paths;
        std::vector<Vertex> verts;

        vec4 bounds;
    };
}

class Canvas final {
public:
    Canvas();
    ~Canvas();

    Canvas(const Canvas&) = delete;
    auto operator=(const Canvas& other) -> Canvas& = delete;

    void begin_frame(const SizeU& windowSize, f32 devicePixelRatio);
    void end_frame();
    void cancel_frame();

    void global_composite_operation(CompositeOperation op);
    void global_composite_blendfunc(gl::BlendFunc sfactor, gl::BlendFunc dfactor);
    void global_composite_blendfunc_separate(gl::BlendFunc srcRGB, gl::BlendFunc dstRGB, gl::BlendFunc srcAlpha, gl::BlendFunc dstAlpha);

    // State handling
    void save();
    void restore();
    void reset();

    // Render styles
    void fill_color(const Color& color);
    void fill_paint(const CanvasPaint& paint);
    void stroke_color(const Color& color);
    void stroke_paint(const CanvasPaint& paint);
    void stroke_width(f32 size);
    void shape_antialias(bool enabled);
    void miter_limit(f32 limit);
    void line_cap(LineCap cap);
    void line_join(LineJoin join);
    void global_alpha(f32 alpha);

    // Paths
    void begin_path();
    void close_path();
    void path_winding(Winding dir);
    void move_to(const PointF& pos);
    void line_to(const PointF& pos);
    void cubic_bezier_to(const PointF& c1, const PointF& c2, const PointF& pos);
    void quad_bezier_to(const PointF& c, const PointF& pos);
    void arc_to(const PointF& pos1, const PointF& pos2, f32 radius);
    void arc(const PointF& c, f32 r, f32 a0, f32 a1, Winding dir);
    void rect(const RectF& rect);
    void rounded_rect(const RectF& rect, f32 r);
    void rounded_rect_varying(const RectF& rect, f32 radTopLeft, f32 radTopRight, f32 radBottomRight, f32 radBottomLeft);
    void ellipse(const PointF& c, f32 hr, f32 vr);
    void circle(const PointF& c, f32 r);
    void fill();
    void stroke();

    // Paints
    auto create_linear_gradient(const PointF& s, const PointF& e, const ColorGradient<256>& gradient) -> CanvasPaint;
    auto create_box_gradient(const RectF& rect, f32 r, f32 f, const ColorGradient<256>& gradient) -> CanvasPaint;
    auto create_radial_gradient(const PointF& c, f32 inr, f32 outr, const ColorGradient<256>& gradient) -> CanvasPaint;
    auto create_image_pattern(const PointF& c, const SizeF& e, f32 angle, i32 image, f32 alpha) -> CanvasPaint;

    // Image
    auto add_image(const std::string& imageName) -> i32;

    // Transforms
    void translate(const PointF& c);
    void rotate(f32 angle);
    void rotate_at(f32 angle, const PointF& p);
    void scale(const SizeF& scale);
    void scale_at(const SizeF& scale, const PointF& p);
    void skew_x(f32 angle);
    void skew_x_at(f32 angle, const PointF& p);
    void skew_y(f32 angle);
    void skew_y_at(f32 angle, const PointF& p);
    void reset_transform();

    // Scissoring
    void scissor(const RectF& rect);
    void reset_scissor();

    // Font
    auto add_font(ResourcePtr<Font> font) -> isize;
    void font_face_ID(isize id);
    void draw_textbox(const PointF& pos, const SizeF& size, const std::string& text);
    void text_align(TextAlignment align);
    void text_outline_color(const Color& color);
    void text_outline_thickness(f32 thickness);

    // Helper
    void fill_rect(const RectF& rect);
    void stroke_rect(const RectF& rect);

    void fill_rounded_rect(const RectF& r, f32 rad);
    void stroke_rounded_rect(const RectF& r, f32 rad);

    void fill_rounded_rect_varying(const RectF& r, f32 rtl, f32 rtr, f32 rbr, f32 rbl);
    void stroke_rounded_rect_varying(const RectF& r, f32 rtl, f32 rtr, f32 rbr, f32 rbl);

    void fill_circle(const PointF& center, f32 r);
    void stroke_circle(const PointF& center, f32 r);

    void fill_ellipse(const PointF& center, f32 hr, f32 vr);
    void stroke_ellipse(const PointF& center, f32 hr, f32 vr);

    void fill_arc(const PointF& center, f32 r, f32 a0, f32 a1, Winding wind);
    void stroke_arc(const PointF& center, f32 r, f32 a0, f32 a1, Winding wind);

    void fill_lines(std::span<PointF> points);
    void stroke_lines(std::span<PointF> points);

    void draw_image(i32 handle, const RectF& rect);
    void draw_image_clipped(i32 handle, const RectF& srect, const RectF& rect);

    auto window_size() const -> SizeU;

    static auto deg_to_rad(f32 deg) -> f32;

private:
    void device_pixel_ratio(f32 ratio);
    auto state() -> detail::NVGstate&;
    void set_paint_color(CanvasPaint& p, const Color& color);
    void append_commands(std::vector<f32>&& vals);
    void clear_path_cache();
    auto get_last_path() -> detail::NVGpath&;
    void add_path();
    auto get_last_point() -> detail::NVGpoint&;
    void add_point(f32 x, f32 y, i32 flags);
    void close_last_path();
    void set_path_winding(Winding winding);
    auto alloc_temp_verts(isize nverts) -> Vertex*;
    void tesselate_bezier(
        f32 x1, f32 y1, f32 x2, f32 y2,
        f32 x3, f32 y3, f32 x4, f32 y4,
        i32 level, i32 type);
    void flatten_paths();
    void calculate_joins(f32 w, LineJoin lineJoin, f32 miterLimit);
    void expand_stroke(f32 w, f32 fringe, LineCap lineCap, LineJoin lineJoin, f32 miterLimit);
    void expand_fill(f32 w, LineJoin lineJoin, f32 miterLimit);
    void render_text(Vertex* verts, i32 nverts);

    bool _edgeAntiAlias { true };
    std::unique_ptr<detail::GLNVGcontext> _glc;

    std::vector<f32> _commands;
    f32 _commandx { 0 }, _commandy { 0 };

    std::stack<detail::NVGstate> _states;

    detail::NVGpathCache _cache;

    f32 _tessTol { 0 };
    f32 _distTol { 0 };
    f32 _fringeWidth { 0 };
    f32 _devicePxRatio { 0 };

    std::vector<ResourcePtr<Font>> _fonts;
    std::vector<std::unique_ptr<gl::Texture2D>> _images;

    isize _drawCallCount { 0 };
    isize _fillTriCount { 0 };
    isize _strokeTriCount { 0 };
    isize _textTriCount { 0 };

    SizeF _windowSize;
};
}