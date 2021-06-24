// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/gfx/QuadAutomation.hpp>

#include <iostream>

namespace tcob {
using namespace std::chrono_literals;

////////////////////////////////////////////////////////////

auto FadeInEffect::value(f32 progress, isize index, isize length, const Quad& quad) -> Quad
{
    Quad retValue { quad };

    isize fadeidx { static_cast<isize>(progress * length) };
    if (index < fadeidx) {
        retValue.TopRight.Color[3] = 255;
        retValue.BottomRight.Color[3] = 255;
        retValue.TopLeft.Color[3] = 255;
        retValue.BottomLeft.Color[3] = 255;
    } else if (index > fadeidx) {
        retValue.TopRight.Color[3] = 0;
        retValue.BottomRight.Color[3] = 0;
        retValue.TopLeft.Color[3] = 0;
        retValue.BottomLeft.Color[3] = 0;
    } else {
        f32 val { progress * length - fadeidx };
        retValue.TopRight.Color[3] = static_cast<u8>(val * 255);
        retValue.BottomRight.Color[3] = static_cast<u8>(val * 255);
        retValue.TopLeft.Color[3] = static_cast<u8>(val * 255);
        retValue.BottomLeft.Color[3] = static_cast<u8>(val * 255);
    }

    return retValue;
}

////////////////////////////////////////////////////////////

auto FadeOutEffect::value(f32 progress, isize index, isize length, const Quad& quad) -> Quad
{
    Quad retValue { quad };

    isize fadeidx { static_cast<isize>(progress * length) };
    if (fadeidx < index) {
        retValue.TopRight.Color[3] = 255;
        retValue.BottomRight.Color[3] = 255;
        retValue.TopLeft.Color[3] = 255;
        retValue.BottomLeft.Color[3] = 255;
    } else if (fadeidx > index) {
        retValue.TopRight.Color[3] = 0;
        retValue.BottomRight.Color[3] = 0;
        retValue.TopLeft.Color[3] = 0;
        retValue.BottomLeft.Color[3] = 0;
    } else {
        f32 val { 1 - (progress * length - fadeidx) };
        retValue.TopRight.Color[3] = static_cast<u8>(val * 255);
        retValue.BottomRight.Color[3] = static_cast<u8>(val * 255);
        retValue.TopLeft.Color[3] = static_cast<u8>(val * 255);
        retValue.BottomLeft.Color[3] = static_cast<u8>(val * 255);
    }

    return retValue;
}

////////////////////////////////////////////////////////////

BlinkEffect::BlinkEffect(MilliSeconds duration, MilliSeconds interval, Color color0, Color color1)
    : Duration { duration }
    , Interval { interval }
    , Color0 { color0 }
    , Color1 { color1 }
{
}

auto BlinkEffect::value([[maybe_unused]] f32 progress, isize index, isize length, const Quad& quad) -> Quad
{
    Quad retValue { quad };

    retValue.color(_flip ? Color0 : Color1);

    if (index == length - 1)
        _flip = !_flip;

    return retValue;
}

////////////////////////////////////////////////////////////
}
