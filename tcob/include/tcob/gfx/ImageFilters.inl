// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "ImageFilters.hpp"

#include <algorithm>

#include "tcob/core/Point.hpp"
#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/TaskManager.hpp"
#include "tcob/gfx/Image.hpp"

namespace tcob::gfx {

template <i32 Width, i32 Height>
inline auto convolution_filter<Width, Height>::operator()(image const& img) const -> image
{
    auto const& info {img.info()};
    auto const [imgWidth, imgHeight] {info.Size};
    i32 const bpp {info.bytes_per_pixel()};

    auto const srcBuffer {img.data()};
    auto       retValue {image::CreateEmpty(info.Size, info.Format)};

    bool const incAlpha {IncludeAlpha && info.Format == image::format::RGBA};

    f64 const  f {factor()};
    u8 const   o {offset()};
    auto const m {matrix()};

    locate_service<task_manager>().run_parallel(
        [&](par_task const& ctx) {
            for (isize pixIdx {ctx.Start}; pixIdx < ctx.End; ++pixIdx) {
                isize const x {pixIdx % imgWidth};
                isize const y {pixIdx / imgWidth};

                f64 red {0.0}, green {0.0}, blue {0.0}, alpha {0.0};

                for (i32 filterX {0}; filterX < Width; ++filterX) {
                    for (i32 filterY {0}; filterY < Height; ++filterY) {
                        isize const imgX {(x - Width / 2 + filterX + imgWidth) % imgWidth};
                        isize const imgY {(y - Height / 2 + filterY + imgHeight) % imgHeight};
                        usize const idx {static_cast<usize>((imgX + (imgY * info.Size.Width)) * bpp)};
                        i32 const   mat {m[static_cast<usize>(filterX + (filterY * Width))]};
                        red += srcBuffer[idx] * mat;
                        green += srcBuffer[idx + 1] * mat;
                        blue += srcBuffer[idx + 2] * mat;
                        if (incAlpha) {
                            alpha += srcBuffer[idx + 3] * mat;
                        }
                    }
                }

                u8 const      r {static_cast<u8>(std::clamp(f * red + o, 0.0, 255.0))};
                u8 const      g {static_cast<u8>(std::clamp(f * green + o, 0.0, 255.0))};
                u8 const      b {static_cast<u8>(std::clamp(f * blue + o, 0.0, 255.0))};
                point_i const pos {static_cast<i32>(x), static_cast<i32>(y)};
                u8 const      a {incAlpha ? static_cast<u8>(std::clamp(f * alpha + o, 0.0, 255.0)) : img.get_pixel(pos).A};
                retValue.set_pixel(pos, {r, g, b, a});
            }
        },
        imgWidth * imgHeight);

    return retValue;
}

}
