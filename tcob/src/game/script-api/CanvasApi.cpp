// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT
#include "CanvasApi.hpp"

#include <tcob/assets/ResourceLibrary.hpp>
#include <tcob/script/LuaConversions.hpp>

namespace tcob {

namespace lua {
    template <>
    struct Converter<ColorStop> {
        static constexpr i32 StackSlots { 1 };

        static auto IsType(const lua::State& ls, i32 idx) -> bool
        {
            if (ls.is_table(idx)) {
                lua::Table lt { ls, idx };
                return lt.raw_length() == 2 && lt.is<f32>(1) && lt.is<Color>(2);
            }
            return false;
        }

        static auto FromLua(const lua::State& ls, i32&& idx, ColorStop& value) -> bool
        {
            if (ls.is_table(idx)) {
                lua::Table lt { ls, idx++ };
                value.Position = lt[1];
                value.Value = lt[2];
            }
            return true;
        }
    };

    template <typename T>
    struct EnumConverter {
        static std::unordered_map<std::string, T> Map;

        static auto IsType(const lua::State& ls, i32 idx) -> bool
        {
            return ls.is_string(idx) && Map.contains(ls.to_string(idx));
        }

        static auto FromLua(const lua::State& ls, i32&& idx, T& value) -> bool
        {
            if (ls.is_string(idx)) {
                std::string val { ls.to_string(idx++) };

                if (Map.contains(val)) {
                    value = Map[val];
                    return true;
                }
            }

            idx++;
            return false;
        }
    };

    template <>
    struct Converter<Winding> {
        static constexpr i32 StackSlots { 1 };

        static auto IsType(const lua::State& ls, i32 idx) -> bool
        {
            return EnumConverter<Winding>::IsType(ls, idx);
        }

        static auto FromLua(const lua::State& ls, i32&& idx, Winding& value) -> bool
        {
            return EnumConverter<Winding>::FromLua(ls, std::forward<i32>(idx), value);
        }
    };
    template <>
    std::unordered_map<std::string, Winding> EnumConverter<Winding>::Map {
        { "CW", Winding::CW },
        { "CCW", Winding::CCW },
    };

    template <>
    struct Converter<Solidity> {
        static constexpr i32 StackSlots { 1 };

        static auto IsType(const lua::State& ls, i32 idx) -> bool
        {
            return EnumConverter<Solidity>::IsType(ls, idx);
        }

        static auto FromLua(const lua::State& ls, i32&& idx, Solidity& value) -> bool
        {
            return EnumConverter<Solidity>::FromLua(ls, std::forward<i32>(idx), value);
        }
    };
    template <>
    std::unordered_map<std::string, Solidity> EnumConverter<Solidity>::Map {
        { "Solid", Solidity::Solid },
        { "Hole", Solidity::Hole },
    };

    template <>
    struct Converter<LineCap> {
        static constexpr i32 StackSlots { 1 };

        static auto IsType(const lua::State& ls, i32 idx) -> bool
        {
            return EnumConverter<LineCap>::IsType(ls, idx);
        }

        static auto FromLua(const lua::State& ls, i32&& idx, LineCap& value) -> bool
        {
            return EnumConverter<LineCap>::FromLua(ls, std::forward<i32>(idx), value);
        }
    };
    template <>
    std::unordered_map<std::string, LineCap> EnumConverter<LineCap>::Map {
        { "Butt", LineCap::Butt },
        { "Round", LineCap::Round },
        { "Square", LineCap::Square },
    };

    template <>
    struct Converter<LineJoin> {
        static constexpr i32 StackSlots { 1 };

        static auto IsType(const lua::State& ls, i32 idx) -> bool
        {
            return EnumConverter<LineJoin>::IsType(ls, idx);
        }

        static auto FromLua(const lua::State& ls, i32&& idx, LineJoin& value) -> bool
        {
            return EnumConverter<LineJoin>::FromLua(ls, std::forward<i32>(idx), value);
        }
    };
    template <>
    std::unordered_map<std::string, LineJoin> EnumConverter<LineJoin>::Map {
        { "Round", LineJoin::Round },
        { "Bevel", LineJoin::Bevel },
        { "Miter", LineJoin::Miter },
    };

    template <>
    struct Converter<TextAlignment> {
        static constexpr i32 StackSlots { 1 };

        static auto IsType(const lua::State& ls, i32 idx) -> bool
        {
            return EnumConverter<TextAlignment>::IsType(ls, idx);
        }

        static auto FromLua(const lua::State& ls, i32&& idx, TextAlignment& value) -> bool
        {
            return EnumConverter<TextAlignment>::FromLua(ls, std::forward<i32>(idx), value);
        }
    };
    template <>
    std::unordered_map<std::string, TextAlignment> EnumConverter<TextAlignment>::Map {
        { "Left", TextAlignment::Left },
        { "Center", TextAlignment::Centered },
        { "Right", TextAlignment::Right },
    };

}

}

namespace tcob::detail {

void create_canvas_wrapper(lua::Script* script, const ResourceLibrary& library)
{
    // ---Canvas wrapper
    auto& canvasWrap { script->create_wrapper<Canvas>("Canvas") };

    canvasWrap.function("begin_frame", &Canvas::begin_frame);
    canvasWrap.function("end_frame", &Canvas::end_frame);

    // Transforms
    canvasWrap.function("translate", &Canvas::translate);
    canvasWrap.function("rotate", &Canvas::rotate, &Canvas::rotate_at);
    canvasWrap.function("scale", &Canvas::scale, &Canvas::scale_at);
    canvasWrap.function("skew_x", &Canvas::skew_x, &Canvas::skew_x_at);
    canvasWrap.function("skew_y", &Canvas::skew_y, &Canvas::skew_y_at);
    canvasWrap.function("reset_transform", &Canvas::reset_transform);

    // State handling
    canvasWrap.function("save", &Canvas::save);
    canvasWrap.function("restore", &Canvas::restore);

    // Paths
    canvasWrap.function("path_winding", &Canvas::path_winding);
    canvasWrap.function("move_to", &Canvas::move_to);
    canvasWrap.function("line_to", &Canvas::line_to);
    canvasWrap.function("arc_to", &Canvas::arc_to);
    canvasWrap.function("quad_bezier_to", &Canvas::quad_bezier_to);
    canvasWrap.function("cubic_bezier_to", &Canvas::cubic_bezier_to);
    canvasWrap.function("begin_path", &Canvas::begin_path);
    canvasWrap.function("close_path", &Canvas::close_path);
    canvasWrap.function("fill", &Canvas::fill);
    canvasWrap.function("stroke", &Canvas::stroke);

    // Shapes
    canvasWrap.function("fill_rect", &Canvas::fill_rect);
    canvasWrap.function("stroke_rect", &Canvas::stroke_rect);
    canvasWrap.function("fill_rounded_rect", &Canvas::fill_rounded_rect);
    canvasWrap.function("stroke_rounded_rect", &Canvas::stroke_rounded_rect);
    canvasWrap.function("fill_rounded_rect_varying", &Canvas::fill_rounded_rect_varying);
    canvasWrap.function("stroke_rounded_rect_varying", &Canvas::stroke_rounded_rect_varying);
    canvasWrap.function("fill_circle", &Canvas::fill_circle);
    canvasWrap.function("stroke_circle", &Canvas::stroke_circle);
    canvasWrap.function("fill_ellipse", &Canvas::fill_ellipse);
    canvasWrap.function("stroke_ellipse", &Canvas::stroke_ellipse);
    canvasWrap.function("fill_arc", &Canvas::fill_arc);
    canvasWrap.function("stroke_arc", &Canvas::stroke_arc);
    canvasWrap.function("fill_lines",
        [](Canvas* canvas, std::vector<PointF>& points) {
            canvas->fill_lines(points);
        });
    canvasWrap.function("stroke_lines",
        [](Canvas* canvas, std::vector<PointF>& points) {
            canvas->stroke_lines(points);
        });

    // Render styles
    canvasWrap.function("fill_style", &Canvas::fill_color,
        [](Canvas* canvas, CanvasPaint* paint) {
            canvas->fill_paint(*paint);
        });
    canvasWrap.function("stroke_style", &Canvas::stroke_color,
        [](Canvas* canvas, CanvasPaint* paint) {
            canvas->stroke_paint(*paint);
        });
    canvasWrap.function("stroke_width", &Canvas::stroke_width);
    canvasWrap.function("global_alpha", &Canvas::global_alpha);
    canvasWrap.function("shape_antialias", &Canvas::shape_antialias);
    canvasWrap.function("miter_limit", &Canvas::miter_limit);
    canvasWrap.function("line_cap", &Canvas::line_cap);
    canvasWrap.function("line_join", &Canvas::line_join);

    // Scissoring
    canvasWrap.function("scissor", &Canvas::scissor);
    canvasWrap.function("reset_scissor", &Canvas::reset_scissor);

    // Gradients
    canvasWrap.function("create_linear_gradient", [](Canvas* canvas, const PointF& s, const PointF& e, std::vector<ColorStop>& colors) {
        auto paint { canvas->create_linear_gradient(s, e, ColorGradient<256> { colors }) };
        return lua::LuaOwnedPtr { new CanvasPaint { paint } };
    });
    canvasWrap.function("create_box_gradient", [](Canvas* canvas, const RectF& rect, f32 r, f32 f, std::vector<ColorStop>& colors) {
        auto paint { canvas->create_box_gradient(rect, r, f, ColorGradient<256> { colors }) };
        return lua::LuaOwnedPtr { new CanvasPaint { paint } };
    });
    canvasWrap.function("create_radial_gradient", [](Canvas* canvas, const PointF& c, f32 inr, f32 outr, std::vector<ColorStop>& colors) {
        auto paint { canvas->create_radial_gradient(c, inr, outr, ColorGradient<256> { colors }) };
        return lua::LuaOwnedPtr { new CanvasPaint { paint } };
    });

    // Image
    canvasWrap.function("add_image", &Canvas::add_image);
    canvasWrap.function("draw_image", &Canvas::draw_image, &Canvas::draw_image_clipped);
    canvasWrap.function("create_image_pattern", [](Canvas* canvas, const PointF& c, const SizeF& e, f32 angle, i32 image, f32 alpha) {
        auto paint { canvas->create_image_pattern(c, e, angle, image, alpha) };
        return lua::LuaOwnedPtr { new CanvasPaint { paint } };
    });

    // Font
    canvasWrap.function("add_font", [&library](Canvas* canvas, const std::string& group, const std::string& font) {
        return canvas->add_font(library.get<Font>(group, font));
    });
    canvasWrap.function("font_face", &Canvas::font_face_ID);
    canvasWrap.function("text_align", &Canvas::text_align);
    canvasWrap.function("draw_textbox", &Canvas::draw_textbox);
    canvasWrap.function("text_outline_color", &Canvas::text_outline_color);
    canvasWrap.function("text_outline_thickness", &Canvas::text_outline_thickness);

    // RenderTarget
    canvasWrap.function("window_size", &Canvas::window_size);
}

void fill_colors_table(const lua::Table& tab)
{
    tab["AliceBlue"] = Colors::AliceBlue;
    tab["AntiqueWhite"] = Colors::AntiqueWhite;
    tab["Aqua"] = Colors::Aqua;
    tab["Aquamarine"] = Colors::Aquamarine;
    tab["Azure"] = Colors::Azure;
    tab["Beige"] = Colors::Beige;
    tab["Bisque"] = Colors::Bisque;
    tab["Black"] = Colors::Black;
    tab["BlanchedAlmond"] = Colors::BlanchedAlmond;
    tab["Blue"] = Colors::Blue;
    tab["BlueViolet"] = Colors::BlueViolet;
    tab["Brown"] = Colors::Brown;
    tab["BurlyWood"] = Colors::BurlyWood;
    tab["CadetBlue"] = Colors::CadetBlue;
    tab["Chartreuse"] = Colors::Chartreuse;
    tab["Chocolate"] = Colors::Chocolate;
    tab["Coral"] = Colors::Coral;
    tab["CornflowerBlue"] = Colors::CornflowerBlue;
    tab["Cornsilk"] = Colors::Cornsilk;
    tab["Crimson"] = Colors::Crimson;
    tab["Cyan"] = Colors::Cyan;
    tab["DarkBlue"] = Colors::DarkBlue;
    tab["DarkCyan"] = Colors::DarkCyan;
    tab["DarkGoldenRod"] = Colors::DarkGoldenRod;
    tab["DarkGray"] = Colors::DarkGray;
    tab["DarkGreen"] = Colors::DarkGreen;
    tab["DarkKhaki"] = Colors::DarkKhaki;
    tab["DarkMagenta"] = Colors::DarkMagenta;
    tab["DarkOliveGreen"] = Colors::DarkOliveGreen;
    tab["DarkOrange"] = Colors::DarkOrange;
    tab["DarkOrchid"] = Colors::DarkOrchid;
    tab["DarkRed"] = Colors::DarkRed;
    tab["DarkSalmon"] = Colors::DarkSalmon;
    tab["DarkSeaGreen"] = Colors::DarkSeaGreen;
    tab["DarkSlateBlue"] = Colors::DarkSlateBlue;
    tab["DarkSlateGray"] = Colors::DarkSlateGray;
    tab["DarkTurquoise"] = Colors::DarkTurquoise;
    tab["DarkViolet"] = Colors::DarkViolet;
    tab["DeepPink"] = Colors::DeepPink;
    tab["DeepSkyBlue"] = Colors::DeepSkyBlue;
    tab["DimGray"] = Colors::DimGray;
    tab["DodgerBlue"] = Colors::DodgerBlue;
    tab["FireBrick"] = Colors::FireBrick;
    tab["FloralWhite"] = Colors::FloralWhite;
    tab["ForestGreen"] = Colors::ForestGreen;
    tab["Fuchsia"] = Colors::Fuchsia;
    tab["Gainsboro"] = Colors::Gainsboro;
    tab["GhostWhite"] = Colors::GhostWhite;
    tab["Gold"] = Colors::Gold;
    tab["GoldenRod"] = Colors::GoldenRod;
    tab["Gray"] = Colors::Gray;
    tab["Green"] = Colors::Green;
    tab["GreenYellow"] = Colors::GreenYellow;
    tab["HoneyDew"] = Colors::HoneyDew;
    tab["HotPink"] = Colors::HotPink;
    tab["IndianRed"] = Colors::IndianRed;
    tab["Indigo"] = Colors::Indigo;
    tab["Ivory"] = Colors::Ivory;
    tab["Khaki"] = Colors::Khaki;
    tab["Lavender"] = Colors::Lavender;
    tab["LavenderBlush"] = Colors::LavenderBlush;
    tab["LawnGreen"] = Colors::LawnGreen;
    tab["LemonChiffon"] = Colors::LemonChiffon;
    tab["LightBlue"] = Colors::LightBlue;
    tab["LightCoral"] = Colors::LightCoral;
    tab["LightCyan"] = Colors::LightCyan;
    tab["LightGoldenRodYellow"] = Colors::LightGoldenRodYellow;
    tab["LightGray"] = Colors::LightGray;
    tab["LightGreen"] = Colors::LightGreen;
    tab["LightPink"] = Colors::LightPink;
    tab["LightSalmon"] = Colors::LightSalmon;
    tab["LightSeaGreen"] = Colors::LightSeaGreen;
    tab["LightSkyBlue"] = Colors::LightSkyBlue;
    tab["LightSlateGray"] = Colors::LightSlateGray;
    tab["LightSteelBlue"] = Colors::LightSteelBlue;
    tab["LightYellow"] = Colors::LightYellow;
    tab["Lime"] = Colors::Lime;
    tab["LimeGreen"] = Colors::LimeGreen;
    tab["Linen"] = Colors::Linen;
    tab["Magenta"] = Colors::Magenta;
    tab["Maroon"] = Colors::Maroon;
    tab["MediumAquaMarine"] = Colors::MediumAquaMarine;
    tab["MediumBlue"] = Colors::MediumBlue;
    tab["MediumOrchid"] = Colors::MediumOrchid;
    tab["MediumPurple"] = Colors::MediumPurple;
    tab["MediumSeaGreen"] = Colors::MediumSeaGreen;
    tab["MediumSlateBlue"] = Colors::MediumSlateBlue;
    tab["MediumSpringGreen"] = Colors::MediumSpringGreen;
    tab["MediumTurquoise"] = Colors::MediumTurquoise;
    tab["MediumVioletRed"] = Colors::MediumVioletRed;
    tab["MidnightBlue"] = Colors::MidnightBlue;
    tab["MintCream"] = Colors::MintCream;
    tab["MistyRose"] = Colors::MistyRose;
    tab["Moccasin"] = Colors::Moccasin;
    tab["NavajoWhite"] = Colors::NavajoWhite;
    tab["Navy"] = Colors::Navy;
    tab["OldLace"] = Colors::OldLace;
    tab["Olive"] = Colors::Olive;
    tab["OliveDrab"] = Colors::OliveDrab;
    tab["Orange"] = Colors::Orange;
    tab["OrangeRed"] = Colors::OrangeRed;
    tab["Orchid"] = Colors::Orchid;
    tab["PaleGoldenRod"] = Colors::PaleGoldenRod;
    tab["PaleGreen"] = Colors::PaleGreen;
    tab["PaleTurquoise"] = Colors::PaleTurquoise;
    tab["PaleVioletRed"] = Colors::PaleVioletRed;
    tab["PapayaWhip"] = Colors::PapayaWhip;
    tab["PeachPuff"] = Colors::PeachPuff;
    tab["Peru"] = Colors::Peru;
    tab["Pink"] = Colors::Pink;
    tab["Plum"] = Colors::Plum;
    tab["PowderBlue"] = Colors::PowderBlue;
    tab["Purple"] = Colors::Purple;
    tab["RebeccaPurple"] = Colors::RebeccaPurple;
    tab["Red"] = Colors::Red;
    tab["RosyBrown"] = Colors::RosyBrown;
    tab["RoyalBlue"] = Colors::RoyalBlue;
    tab["SaddleBrown"] = Colors::SaddleBrown;
    tab["Salmon"] = Colors::Salmon;
    tab["SandyBrown"] = Colors::SandyBrown;
    tab["SeaGreen"] = Colors::SeaGreen;
    tab["SeaShell"] = Colors::SeaShell;
    tab["Sienna"] = Colors::Sienna;
    tab["Silver"] = Colors::Silver;
    tab["SkyBlue"] = Colors::SkyBlue;
    tab["SlateBlue"] = Colors::SlateBlue;
    tab["SlateGray"] = Colors::SlateGray;
    tab["Snow"] = Colors::Snow;
    tab["SpringGreen"] = Colors::SpringGreen;
    tab["SteelBlue"] = Colors::SteelBlue;
    tab["Tan"] = Colors::Tan;
    tab["Teal"] = Colors::Teal;
    tab["Thistle"] = Colors::Thistle;
    tab["Tomato"] = Colors::Tomato;
    tab["Turquoise"] = Colors::Turquoise;
    tab["Violet"] = Colors::Violet;
    tab["Wheat"] = Colors::Wheat;
    tab["White"] = Colors::White;
    tab["WhiteSmoke"] = Colors::WhiteSmoke;
    tab["Yellow"] = Colors::Yellow;
    tab["YellowGreen"] = Colors::YellowGreen;
}
}