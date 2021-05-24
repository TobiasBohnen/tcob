// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <algorithm>
#include <sstream>
#include <unordered_map>

namespace tcob {

#pragma pack(push, 1)
struct Color final {
    constexpr Color() = default;

    constexpr Color(u8 r, u8 g, u8 b, u8 a)
        : R { r }
        , G { g }
        , B { b }
        , A { a }
    {
    }

    explicit constexpr Color(u32 color)
        : Color {
            static_cast<u8>((color & 0xff000000) >> 24),
            static_cast<u8>((color & 0x00ff0000) >> 16),
            static_cast<u8>((color & 0x0000ff00) >> 8),
            static_cast<u8>((color & 0x000000ff) >> 0)
        }
    {
    }

    auto interpolate(const Color& other, f64 step) const -> Color;

    auto premultiply_alpha() const -> Color;

    u8 R { 0 };
    u8 G { 0 };
    u8 B { 0 };
    u8 A { 0 };
};
#pragma pack(pop)

inline constexpr auto operator==(const Color& left, const Color& right) -> bool
{
    return (left.R == right.R) && (left.G == right.G) && (left.B == right.B) && (left.A == right.A);
}
inline constexpr auto operator!=(const Color& left, const Color& right) -> bool
{
    return (left.R != right.R) || (left.G != right.G) || (left.B != right.B) || (left.A != right.A);
}

inline auto operator<<(std::ostream& os, const Color& m) -> std::ostream&
{
    return os << "r:" << static_cast<u32>(m.R) << "|g:" << static_cast<u32>(m.G) << "|b:" << static_cast<u32>(m.B) << "|a:" << static_cast<u32>(m.A);
}

////////////////////////////////////////////////////////////

namespace Colors {
    inline constexpr Color AliceBlue(0xF0F8FFFF);
    inline constexpr Color AntiqueWhite(0xFAEBD7FF);
    inline constexpr Color Aqua(0x00FFFFFF);
    inline constexpr Color Aquamarine(0x7FFFD4FF);
    inline constexpr Color Azure(0xF0FFFFFF);
    inline constexpr Color Beige(0xF5F5DCFF);
    inline constexpr Color Bisque(0xFFE4C4FF);
    inline constexpr Color Black(0x000000FF);
    inline constexpr Color BlanchedAlmond(0xFFEBCDFF);
    inline constexpr Color Blue(0x0000FFFF);
    inline constexpr Color BlueViolet(0x8A2BE2FF);
    inline constexpr Color Brown(0xA52A2AFF);
    inline constexpr Color BurlyWood(0xDEB887FF);
    inline constexpr Color CadetBlue(0x5F9EA0FF);
    inline constexpr Color Chartreuse(0x7FFF00FF);
    inline constexpr Color Chocolate(0xD2691EFF);
    inline constexpr Color Coral(0xFF7F50FF);
    inline constexpr Color CornflowerBlue(0x6495EDFF);
    inline constexpr Color Cornsilk(0xFFF8DCFF);
    inline constexpr Color Crimson(0xDC143CFF);
    inline constexpr Color Cyan(0x00FFFFFF);
    inline constexpr Color DarkBlue(0x00008BFF);
    inline constexpr Color DarkCyan(0x008B8BFF);
    inline constexpr Color DarkGoldenRod(0xB8860BFF);
    inline constexpr Color DarkGray(0xA9A9A9FF);
    inline constexpr Color DarkGreen(0x006400FF);
    inline constexpr Color DarkKhaki(0xBDB76BFF);
    inline constexpr Color DarkMagenta(0x8B008BFF);
    inline constexpr Color DarkOliveGreen(0x556B2FFF);
    inline constexpr Color DarkOrange(0xFF8C00FF);
    inline constexpr Color DarkOrchid(0x9932CCFF);
    inline constexpr Color DarkRed(0x8B0000FF);
    inline constexpr Color DarkSalmon(0xE9967AFF);
    inline constexpr Color DarkSeaGreen(0x8FBC8FFF);
    inline constexpr Color DarkSlateBlue(0x483D8BFF);
    inline constexpr Color DarkSlateGray(0x2F4F4FFF);
    inline constexpr Color DarkTurquoise(0x00CED1FF);
    inline constexpr Color DarkViolet(0x9400D3FF);
    inline constexpr Color DeepPink(0xFF1493FF);
    inline constexpr Color DeepSkyBlue(0x00BFFFFF);
    inline constexpr Color DimGray(0x696969FF);
    inline constexpr Color DodgerBlue(0x1E90FFFF);
    inline constexpr Color FireBrick(0xB22222FF);
    inline constexpr Color FloralWhite(0xFFFAF0FF);
    inline constexpr Color ForestGreen(0x228B22FF);
    inline constexpr Color Fuchsia(0xFF00FFFF);
    inline constexpr Color Gainsboro(0xDCDCDCFF);
    inline constexpr Color GhostWhite(0xF8F8FFFF);
    inline constexpr Color Gold(0xFFD700FF);
    inline constexpr Color GoldenRod(0xDAA520FF);
    inline constexpr Color Gray(0x808080FF);
    inline constexpr Color Green(0x008000FF);
    inline constexpr Color GreenYellow(0xADFF2FFF);
    inline constexpr Color HoneyDew(0xF0FFF0FF);
    inline constexpr Color HotPink(0xFF69B4FF);
    inline constexpr Color IndianRed(0xCD5C5CFF);
    inline constexpr Color Indigo(0x4B0082FF);
    inline constexpr Color Ivory(0xFFFFF0FF);
    inline constexpr Color Khaki(0xF0E68CFF);
    inline constexpr Color Lavender(0xE6E6FAFF);
    inline constexpr Color LavenderBlush(0xFFF0F5FF);
    inline constexpr Color LawnGreen(0x7CFC00FF);
    inline constexpr Color LemonChiffon(0xFFFACDFF);
    inline constexpr Color LightBlue(0xADD8E6FF);
    inline constexpr Color LightCoral(0xF08080FF);
    inline constexpr Color LightCyan(0xE0FFFFFF);
    inline constexpr Color LightGoldenRodYellow(0xFAFAD2FF);
    inline constexpr Color LightGray(0xD3D3D3FF);
    inline constexpr Color LightGreen(0x90EE90FF);
    inline constexpr Color LightPink(0xFFB6C1FF);
    inline constexpr Color LightSalmon(0xFFA07AFF);
    inline constexpr Color LightSeaGreen(0x20B2AAFF);
    inline constexpr Color LightSkyBlue(0x87CEFAFF);
    inline constexpr Color LightSlateGray(0x778899FF);
    inline constexpr Color LightSteelBlue(0xB0C4DEFF);
    inline constexpr Color LightYellow(0xFFFFE0FF);
    inline constexpr Color Lime(0x00FF00FF);
    inline constexpr Color LimeGreen(0x32CD32FF);
    inline constexpr Color Linen(0xFAF0E6FF);
    inline constexpr Color Magenta(0xFF00FFFF);
    inline constexpr Color Maroon(0x800000FF);
    inline constexpr Color MediumAquaMarine(0x66CDAAFF);
    inline constexpr Color MediumBlue(0x0000CDFF);
    inline constexpr Color MediumOrchid(0xBA55D3FF);
    inline constexpr Color MediumPurple(0x9370DBFF);
    inline constexpr Color MediumSeaGreen(0x3CB371FF);
    inline constexpr Color MediumSlateBlue(0x7B68EEFF);
    inline constexpr Color MediumSpringGreen(0x00FA9AFF);
    inline constexpr Color MediumTurquoise(0x48D1CCFF);
    inline constexpr Color MediumVioletRed(0xC71585FF);
    inline constexpr Color MidnightBlue(0x191970FF);
    inline constexpr Color MintCream(0xF5FFFAFF);
    inline constexpr Color MistyRose(0xFFE4E1FF);
    inline constexpr Color Moccasin(0xFFE4B5FF);
    inline constexpr Color NavajoWhite(0xFFDEADFF);
    inline constexpr Color Navy(0x000080FF);
    inline constexpr Color OldLace(0xFDF5E6FF);
    inline constexpr Color Olive(0x808000FF);
    inline constexpr Color OliveDrab(0x6B8E23FF);
    inline constexpr Color Orange(0xFFA500FF);
    inline constexpr Color OrangeRed(0xFF4500FF);
    inline constexpr Color Orchid(0xDA70D6FF);
    inline constexpr Color PaleGoldenRod(0xEEE8AAFF);
    inline constexpr Color PaleGreen(0x98FB98FF);
    inline constexpr Color PaleTurquoise(0xAFEEEEFF);
    inline constexpr Color PaleVioletRed(0xDB7093FF);
    inline constexpr Color PapayaWhip(0xFFEFD5FF);
    inline constexpr Color PeachPuff(0xFFDAB9FF);
    inline constexpr Color Peru(0xCD853FFF);
    inline constexpr Color Pink(0xFFC0CBFF);
    inline constexpr Color Plum(0xDDA0DDFF);
    inline constexpr Color PowderBlue(0xB0E0E6FF);
    inline constexpr Color Purple(0x800080FF);
    inline constexpr Color RebeccaPurple(0x663399FF);
    inline constexpr Color Red(0xFF0000FF);
    inline constexpr Color RosyBrown(0xBC8F8FFF);
    inline constexpr Color RoyalBlue(0x4169E1FF);
    inline constexpr Color SaddleBrown(0x8B4513FF);
    inline constexpr Color Salmon(0xFA8072FF);
    inline constexpr Color SandyBrown(0xF4A460FF);
    inline constexpr Color SeaGreen(0x2E8B57FF);
    inline constexpr Color SeaShell(0xFFF5EEFF);
    inline constexpr Color Sienna(0xA0522DFF);
    inline constexpr Color Silver(0xC0C0C0FF);
    inline constexpr Color SkyBlue(0x87CEEBFF);
    inline constexpr Color SlateBlue(0x6A5ACDFF);
    inline constexpr Color SlateGray(0x708090FF);
    inline constexpr Color Snow(0xFFFAFAFF);
    inline constexpr Color SpringGreen(0x00FF7FFF);
    inline constexpr Color SteelBlue(0x4682B4FF);
    inline constexpr Color Tan(0xD2B48CFF);
    inline constexpr Color Teal(0x008080FF);
    inline constexpr Color Thistle(0xD8BFD8FF);
    inline constexpr Color Tomato(0xFF6347FF);
    inline constexpr Color Turquoise(0x40E0D0FF);
    inline constexpr Color Violet(0xEE82EEFF);
    inline constexpr Color Wheat(0xF5DEB3FF);
    inline constexpr Color White(0xFFFFFFFF);
    inline constexpr Color WhiteSmoke(0xF5F5F5FF);
    inline constexpr Color Yellow(0xFFFF00FF);
    inline constexpr Color YellowGreen(0x9ACD32FF);

    inline static Color FromString(const std::string& name)
    {
        static std::unordered_map<std::string, Color> colors {
            { "AliceBlue", Colors::AliceBlue },
            { "AntiqueWhite", Colors::AntiqueWhite },
            { "Aqua", Colors::Aqua },
            { "Aquamarine", Colors::Aquamarine },
            { "Azure", Colors::Azure },
            { "Beige", Colors::Beige },
            { "Bisque", Colors::Bisque },
            { "Black", Colors::Black },
            { "BlanchedAlmond", Colors::BlanchedAlmond },
            { "Blue", Colors::Blue },
            { "BlueViolet", Colors::BlueViolet },
            { "Brown", Colors::Brown },
            { "BurlyWood", Colors::BurlyWood },
            { "CadetBlue", Colors::CadetBlue },
            { "Chartreuse", Colors::Chartreuse },
            { "Chocolate", Colors::Chocolate },
            { "Coral", Colors::Coral },
            { "CornflowerBlue", Colors::CornflowerBlue },
            { "Cornsilk", Colors::Cornsilk },
            { "Crimson", Colors::Crimson },
            { "Cyan", Colors::Cyan },
            { "DarkBlue", Colors::DarkBlue },
            { "DarkCyan", Colors::DarkCyan },
            { "DarkGoldenRod", Colors::DarkGoldenRod },
            { "DarkGray", Colors::DarkGray },
            { "DarkGreen", Colors::DarkGreen },
            { "DarkKhaki", Colors::DarkKhaki },
            { "DarkMagenta", Colors::DarkMagenta },
            { "DarkOliveGreen", Colors::DarkOliveGreen },
            { "DarkOrange", Colors::DarkOrange },
            { "DarkOrchid", Colors::DarkOrchid },
            { "DarkRed", Colors::DarkRed },
            { "DarkSalmon", Colors::DarkSalmon },
            { "DarkSeaGreen", Colors::DarkSeaGreen },
            { "DarkSlateBlue", Colors::DarkSlateBlue },
            { "DarkSlateGray", Colors::DarkSlateGray },
            { "DarkTurquoise", Colors::DarkTurquoise },
            { "DarkViolet", Colors::DarkViolet },
            { "DeepPink", Colors::DeepPink },
            { "DeepSkyBlue", Colors::DeepSkyBlue },
            { "DimGray", Colors::DimGray },
            { "DodgerBlue", Colors::DodgerBlue },
            { "FireBrick", Colors::FireBrick },
            { "FloralWhite", Colors::FloralWhite },
            { "ForestGreen", Colors::ForestGreen },
            { "Fuchsia", Colors::Fuchsia },
            { "Gainsboro", Colors::Gainsboro },
            { "GhostWhite", Colors::GhostWhite },
            { "Gold", Colors::Gold },
            { "GoldenRod", Colors::GoldenRod },
            { "Gray", Colors::Gray },
            { "Green", Colors::Green },
            { "GreenYellow", Colors::GreenYellow },
            { "HoneyDew", Colors::HoneyDew },
            { "HotPink", Colors::HotPink },
            { "IndianRed", Colors::IndianRed },
            { "Indigo", Colors::Indigo },
            { "Ivory", Colors::Ivory },
            { "Khaki", Colors::Khaki },
            { "Lavender", Colors::Lavender },
            { "LavenderBlush", Colors::LavenderBlush },
            { "LawnGreen", Colors::LawnGreen },
            { "LemonChiffon", Colors::LemonChiffon },
            { "LightBlue", Colors::LightBlue },
            { "LightCoral", Colors::LightCoral },
            { "LightCyan", Colors::LightCyan },
            { "LightGoldenRodYellow", Colors::LightGoldenRodYellow },
            { "LightGray", Colors::LightGray },
            { "LightGreen", Colors::LightGreen },
            { "LightPink", Colors::LightPink },
            { "LightSalmon", Colors::LightSalmon },
            { "LightSeaGreen", Colors::LightSeaGreen },
            { "LightSkyBlue", Colors::LightSkyBlue },
            { "LightSlateGray", Colors::LightSlateGray },
            { "LightSteelBlue", Colors::LightSteelBlue },
            { "LightYellow", Colors::LightYellow },
            { "Lime", Colors::Lime },
            { "LimeGreen", Colors::LimeGreen },
            { "Linen", Colors::Linen },
            { "Magenta", Colors::Magenta },
            { "Maroon", Colors::Maroon },
            { "MediumAquaMarine", Colors::MediumAquaMarine },
            { "MediumBlue", Colors::MediumBlue },
            { "MediumOrchid", Colors::MediumOrchid },
            { "MediumPurple", Colors::MediumPurple },
            { "MediumSeaGreen", Colors::MediumSeaGreen },
            { "MediumSlateBlue", Colors::MediumSlateBlue },
            { "MediumSpringGreen", Colors::MediumSpringGreen },
            { "MediumTurquoise", Colors::MediumTurquoise },
            { "MediumVioletRed", Colors::MediumVioletRed },
            { "MidnightBlue", Colors::MidnightBlue },
            { "MintCream", Colors::MintCream },
            { "MistyRose", Colors::MistyRose },
            { "Moccasin", Colors::Moccasin },
            { "NavajoWhite", Colors::NavajoWhite },
            { "Navy", Colors::Navy },
            { "OldLace", Colors::OldLace },
            { "Olive", Colors::Olive },
            { "OliveDrab", Colors::OliveDrab },
            { "Orange", Colors::Orange },
            { "OrangeRed", Colors::OrangeRed },
            { "Orchid", Colors::Orchid },
            { "PaleGoldenRod", Colors::PaleGoldenRod },
            { "PaleGreen", Colors::PaleGreen },
            { "PaleTurquoise", Colors::PaleTurquoise },
            { "PaleVioletRed", Colors::PaleVioletRed },
            { "PapayaWhip", Colors::PapayaWhip },
            { "PeachPuff", Colors::PeachPuff },
            { "Peru", Colors::Peru },
            { "Pink", Colors::Pink },
            { "Plum", Colors::Plum },
            { "PowderBlue", Colors::PowderBlue },
            { "Purple", Colors::Purple },
            { "RebeccaPurple", Colors::RebeccaPurple },
            { "Red", Colors::Red },
            { "RosyBrown", Colors::RosyBrown },
            { "RoyalBlue", Colors::RoyalBlue },
            { "SaddleBrown", Colors::SaddleBrown },
            { "Salmon", Colors::Salmon },
            { "SandyBrown", Colors::SandyBrown },
            { "SeaGreen", Colors::SeaGreen },
            { "SeaShell", Colors::SeaShell },
            { "Sienna", Colors::Sienna },
            { "Silver", Colors::Silver },
            { "SkyBlue", Colors::SkyBlue },
            { "SlateBlue", Colors::SlateBlue },
            { "SlateGray", Colors::SlateGray },
            { "Snow", Colors::Snow },
            { "SpringGreen", Colors::SpringGreen },
            { "SteelBlue", Colors::SteelBlue },
            { "Tan", Colors::Tan },
            { "Teal", Colors::Teal },
            { "Thistle", Colors::Thistle },
            { "Tomato", Colors::Tomato },
            { "Turquoise", Colors::Turquoise },
            { "Violet", Colors::Violet },
            { "Wheat", Colors::Wheat },
            { "White", Colors::White },
            { "WhiteSmoke", Colors::WhiteSmoke },
            { "Yellow", Colors::Yellow },
            { "YellowGreen", Colors::YellowGreen },
        };

        if (colors.contains(name)) {
            return colors.at(name);
        } else if (name[0] == '#') {
            u32 x { 0 };
            std::stringstream ss {};
            if (name.size() == 7) {
                ss << std::hex << (name.substr(1) + "ff");
            } else if (name.size() == 9) {
                ss << std::hex << name.substr(1);
            }
            ss >> x;
            return Color { x };
        }

        return { 0, 0, 0, 0 };
    }
}
}