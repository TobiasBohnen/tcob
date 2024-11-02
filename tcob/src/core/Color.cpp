// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/core/Color.hpp"

#include <unordered_map>

#include "tcob/core/StringUtils.hpp"

namespace tcob {

auto color::FromString(string_view name) -> color
{
    static std::unordered_map<string, color> const colorMap {
        {"transparent", colors::Transparent},
        {"aliceblue", colors::AliceBlue},
        {"antiquewhite", colors::AntiqueWhite},
        {"aqua", colors::Aqua},
        {"aquamarine", colors::Aquamarine},
        {"azure", colors::Azure},
        {"beige", colors::Beige},
        {"bisque", colors::Bisque},
        {"black", colors::Black},
        {"blanchedalmond", colors::BlanchedAlmond},
        {"blue", colors::Blue},
        {"blueviolet", colors::BlueViolet},
        {"brown", colors::Brown},
        {"burlywood", colors::BurlyWood},
        {"cadetblue", colors::CadetBlue},
        {"chartreuse", colors::Chartreuse},
        {"chocolate", colors::Chocolate},
        {"coral", colors::Coral},
        {"cornflowerblue", colors::CornflowerBlue},
        {"cornsilk", colors::Cornsilk},
        {"crimson", colors::Crimson},
        {"cyan", colors::Cyan},
        {"darkblue", colors::DarkBlue},
        {"darkcyan", colors::DarkCyan},
        {"darkgoldenrod", colors::DarkGoldenRod},
        {"darkgray", colors::DarkGray},
        {"darkgreen", colors::DarkGreen},
        {"darkkhaki", colors::DarkKhaki},
        {"darkmagenta", colors::DarkMagenta},
        {"darkolivegreen", colors::DarkOliveGreen},
        {"darkorange", colors::DarkOrange},
        {"darkorchid", colors::DarkOrchid},
        {"darkred", colors::DarkRed},
        {"darksalmon", colors::DarkSalmon},
        {"darkseagreen", colors::DarkSeaGreen},
        {"darkslateblue", colors::DarkSlateBlue},
        {"darkslategray", colors::DarkSlateGray},
        {"darkturquoise", colors::DarkTurquoise},
        {"darkviolet", colors::DarkViolet},
        {"deeppink", colors::DeepPink},
        {"deepskyblue", colors::DeepSkyBlue},
        {"dimgray", colors::DimGray},
        {"dodgerblue", colors::DodgerBlue},
        {"firebrick", colors::FireBrick},
        {"floralwhite", colors::FloralWhite},
        {"forestgreen", colors::ForestGreen},
        {"fuchsia", colors::Fuchsia},
        {"gainsboro", colors::Gainsboro},
        {"ghostwhite", colors::GhostWhite},
        {"gold", colors::Gold},
        {"goldenrod", colors::GoldenRod},
        {"gray", colors::Gray},
        {"green", colors::Green},
        {"greenyellow", colors::GreenYellow},
        {"honeydew", colors::HoneyDew},
        {"hotpink", colors::HotPink},
        {"indianred", colors::IndianRed},
        {"indigo", colors::Indigo},
        {"ivory", colors::Ivory},
        {"khaki", colors::Khaki},
        {"lavender", colors::Lavender},
        {"lavenderblush", colors::LavenderBlush},
        {"lawngreen", colors::LawnGreen},
        {"lemonchiffon", colors::LemonChiffon},
        {"lightblue", colors::LightBlue},
        {"lightcoral", colors::LightCoral},
        {"lightcyan", colors::LightCyan},
        {"lightgoldenrodyellow", colors::LightGoldenRodYellow},
        {"lightgray", colors::LightGray},
        {"lightgreen", colors::LightGreen},
        {"lightpink", colors::LightPink},
        {"lightsalmon", colors::LightSalmon},
        {"lightseagreen", colors::LightSeaGreen},
        {"lightskyblue", colors::LightSkyBlue},
        {"lightslategray", colors::LightSlateGray},
        {"lightsteelblue", colors::LightSteelBlue},
        {"lightyellow", colors::LightYellow},
        {"lime", colors::Lime},
        {"limegreen", colors::LimeGreen},
        {"linen", colors::Linen},
        {"magenta", colors::Magenta},
        {"maroon", colors::Maroon},
        {"mediumaquamarine", colors::MediumAquaMarine},
        {"mediumblue", colors::MediumBlue},
        {"mediumorchid", colors::MediumOrchid},
        {"mediumpurple", colors::MediumPurple},
        {"mediumseagreen", colors::MediumSeaGreen},
        {"mediumslateblue", colors::MediumSlateBlue},
        {"mediumspringgreen", colors::MediumSpringGreen},
        {"mediumturquoise", colors::MediumTurquoise},
        {"mediumvioletred", colors::MediumVioletRed},
        {"midnightblue", colors::MidnightBlue},
        {"mintcream", colors::MintCream},
        {"mistyrose", colors::MistyRose},
        {"moccasin", colors::Moccasin},
        {"navajowhite", colors::NavajoWhite},
        {"navy", colors::Navy},
        {"oldlace", colors::OldLace},
        {"olive", colors::Olive},
        {"olivedrab", colors::OliveDrab},
        {"orange", colors::Orange},
        {"orangered", colors::OrangeRed},
        {"orchid", colors::Orchid},
        {"palegoldenrod", colors::PaleGoldenRod},
        {"palegreen", colors::PaleGreen},
        {"paleturquoise", colors::PaleTurquoise},
        {"palevioletred", colors::PaleVioletRed},
        {"papayawhip", colors::PapayaWhip},
        {"peachpuff", colors::PeachPuff},
        {"peru", colors::Peru},
        {"pink", colors::Pink},
        {"plum", colors::Plum},
        {"powderblue", colors::PowderBlue},
        {"purple", colors::Purple},
        {"rebeccapurple", colors::RebeccaPurple},
        {"red", colors::Red},
        {"rosybrown", colors::RosyBrown},
        {"royalblue", colors::RoyalBlue},
        {"saddlebrown", colors::SaddleBrown},
        {"salmon", colors::Salmon},
        {"sandybrown", colors::SandyBrown},
        {"seagreen", colors::SeaGreen},
        {"seashell", colors::SeaShell},
        {"sienna", colors::Sienna},
        {"silver", colors::Silver},
        {"skyblue", colors::SkyBlue},
        {"slateblue", colors::SlateBlue},
        {"slategray", colors::SlateGray},
        {"snow", colors::Snow},
        {"springgreen", colors::SpringGreen},
        {"steelblue", colors::SteelBlue},
        {"tan", colors::Tan},
        {"teal", colors::Teal},
        {"thistle", colors::Thistle},
        {"tomato", colors::Tomato},
        {"turquoise", colors::Turquoise},
        {"violet", colors::Violet},
        {"wheat", colors::Wheat},
        {"white", colors::White},
        {"whitesmoke", colors::WhiteSmoke},
        {"yellow", colors::Yellow},
        {"yellowgreen", colors::YellowGreen},
    };

    if (name.empty()) { return colors::Transparent; }

    string const test {helper::to_lower(name)};
    if (colorMap.contains(test)) {
        return colorMap.at(test);
    }

    if (name[0] == '#') {
        std::stringstream ss {};

        ss << std::hex;
        string_view const color {name.substr(1)};
        switch (color.size()) {
        case 3:
            for (char ch : color) { ss << std::string(2, ch); }
            ss << "ff";
            break;
        case 4:
            for (char ch : color) { ss << std::string(2, ch); }
            break;
        case 6:
            ss << color << "ff";
            break;
        case 8: ss << color; break;
        }

        u32 x {0};
        ss >> x;
        return color::FromRGBA(x);
    }

    return {0, 0, 0, 0};
}
}
