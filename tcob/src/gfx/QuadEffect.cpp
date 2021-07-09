// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/gfx/QuadEffect.hpp>

#include <iostream>

namespace tcob {
using namespace std::chrono_literals;

////////////////////////////////////////////////////////////

void TypingEffect::value(f32 progress, isize index, isize length, Quad& dest, [[maybe_unused]] const Quad& src) const
{
    isize fadeidx { static_cast<isize>(progress * length) };
    if (index <= fadeidx) {
        dest.TopRight.Color[3] = 255;
        dest.BottomRight.Color[3] = 255;
        dest.TopLeft.Color[3] = 255;
        dest.BottomLeft.Color[3] = 255;
    } else if (index > fadeidx) {
        dest.TopRight.Color[3] = 0;
        dest.BottomRight.Color[3] = 0;
        dest.TopLeft.Color[3] = 0;
        dest.BottomLeft.Color[3] = 0;
    }
}

////////////////////////////////////////////////////////////

void FadeInEffect::value(f32 progress, isize index, isize length, Quad& dest, [[maybe_unused]] const Quad& src) const
{
    isize fadeidx { static_cast<isize>(progress * length) };
    if (index < fadeidx) {
        dest.TopRight.Color[3] = 255;
        dest.BottomRight.Color[3] = 255;
        dest.TopLeft.Color[3] = 255;
        dest.BottomLeft.Color[3] = 255;
    } else if (index > fadeidx) {
        dest.TopRight.Color[3] = 0;
        dest.BottomRight.Color[3] = 0;
        dest.TopLeft.Color[3] = 0;
        dest.BottomLeft.Color[3] = 0;
    } else {
        f32 val { progress * length - fadeidx };
        dest.TopRight.Color[3] = static_cast<u8>(val * 255);
        dest.BottomRight.Color[3] = static_cast<u8>(val * 255);
        dest.TopLeft.Color[3] = static_cast<u8>(val * 255);
        dest.BottomLeft.Color[3] = static_cast<u8>(val * 255);
    }
}

////////////////////////////////////////////////////////////

void FadeOutEffect::value(f32 progress, isize index, isize length, Quad& dest, [[maybe_unused]] const Quad& src) const
{
    isize fadeidx { static_cast<isize>(progress * length) };
    if (fadeidx < index) {
        dest.TopRight.Color[3] = 255;
        dest.BottomRight.Color[3] = 255;
        dest.TopLeft.Color[3] = 255;
        dest.BottomLeft.Color[3] = 255;
    } else if (fadeidx > index) {
        dest.TopRight.Color[3] = 0;
        dest.BottomRight.Color[3] = 0;
        dest.TopLeft.Color[3] = 0;
        dest.BottomLeft.Color[3] = 0;
    } else {
        f32 val { 1 - (progress * length - fadeidx) };
        dest.TopRight.Color[3] = static_cast<u8>(val * 255);
        dest.BottomRight.Color[3] = static_cast<u8>(val * 255);
        dest.TopLeft.Color[3] = static_cast<u8>(val * 255);
        dest.BottomLeft.Color[3] = static_cast<u8>(val * 255);
    }
}

////////////////////////////////////////////////////////////

BlinkEffect::BlinkEffect(Color color0, Color color1)
    : Color0 { color0 }
    , Color1 { color1 }
{
}

void BlinkEffect::value([[maybe_unused]] f32 progress, isize index, isize length, Quad& dest, [[maybe_unused]] const Quad& src)
{
    dest.color(_flip ? Color0 : Color1);

    if (index == length - 1)
        _flip = !_flip;
}

////////////////////////////////////////////////////////////

void ShakeEffect::value([[maybe_unused]] f32 progress, [[maybe_unused]] isize index, [[maybe_unused]] isize length, Quad& dest, const Quad& src)
{
    f32 r { RNG(-Intensity, Intensity) };
    switch (RNG(0, 1)) {
    case 0:
        dest.TopRight.Position[0] = src.TopRight.Position[0] + r;
        dest.TopRight.Position[1] = src.TopRight.Position[1] + r;
        dest.BottomRight.Position[0] = src.BottomRight.Position[0] + r;
        dest.BottomRight.Position[1] = src.BottomRight.Position[1] + r;
        dest.TopLeft.Position[0] = src.TopLeft.Position[0] + r;
        dest.TopLeft.Position[1] = src.TopLeft.Position[1] + r;
        dest.BottomLeft.Position[0] = src.BottomLeft.Position[0] + r;
        dest.BottomLeft.Position[1] = src.BottomLeft.Position[1] + r;
        break;
    case 1:
        dest.TopRight.Position[0] = src.TopRight.Position[0] + r;
        dest.TopRight.Position[1] = src.TopRight.Position[1] - r;
        dest.BottomRight.Position[0] = src.BottomRight.Position[0] + r;
        dest.BottomRight.Position[1] = src.BottomRight.Position[1] - r;
        dest.TopLeft.Position[0] = src.TopLeft.Position[0] + r;
        dest.TopLeft.Position[1] = src.TopLeft.Position[1] - r;
        dest.BottomLeft.Position[0] = src.BottomLeft.Position[0] + r;
        dest.BottomLeft.Position[1] = src.BottomLeft.Position[1] - r;
        break;
    default:
        break;
    }
}

////////////////////////////////////////////////////////////

void WaveEffect::value(f32 progress, isize index, isize length, Quad& dest, const Quad& src)
{
    const f64 phase { static_cast<f64>(index) / length };
    const f64 factor { (std::sin((TAU * progress) + (0.75 * TAU) + phase * Amplitude) + 1) / 2 };

    const f64 val { factor * Height };

    dest.TopRight.Position[1] = static_cast<f32>(src.TopRight.Position[1] + val);
    dest.BottomRight.Position[1] = static_cast<f32>(src.BottomRight.Position[1] + val);
    dest.TopLeft.Position[1] = static_cast<f32>(src.TopLeft.Position[1] + val);
    dest.BottomLeft.Position[1] = static_cast<f32>(src.BottomLeft.Position[1] + val);
}

}
