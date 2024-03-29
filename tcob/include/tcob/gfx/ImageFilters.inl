// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "ImageFilters.hpp"

#include <execution>

namespace tcob::gfx {

template <i32 Width, i32 Height>
inline auto convolution_filter<Width, Height>::operator()(image const& img) const -> image
{
    auto const& info {img.get_info()};
    auto const [imgWidth, imgHeight] {info.Size};
    i32 const bpp {info.bytes_per_pixel()};

    auto const srcBuffer {img.get_data()};
    auto       retValue {image::CreateEmpty(info.Size, info.Format)};

    bool const incAlpha {IncludeAlpha && info.Format == image::format::RGBA};

    f64 const  factor {get_factor()};
    u8 const   offset {get_offset()};
    auto const matrix {get_matrix()};

    std::for_each(std::execution::par_unseq,
                  tcob::detail::counting_iterator<i32> {0},
                  tcob::detail::counting_iterator<i32> {imgWidth * imgHeight},
                  [&](i32 pixIdx) {
                      i32 const x {pixIdx % imgWidth};
                      i32 const y {pixIdx / imgWidth};

                      f64 red {0.0}, green {0.0}, blue {0.0}, alpha {0.0};

                      for (i32 filterX {0}; filterX < Width; ++filterX) {
                          for (i32 filterY {0}; filterY < Height; ++filterY) {
                              i32 const   imgX {(x - Width / 2 + filterX + imgWidth) % imgWidth};
                              i32 const   imgY {(y - Height / 2 + filterY + imgHeight) % imgHeight};
                              usize const idx {static_cast<usize>((imgX + (imgY * info.Size.Width)) * bpp)};
                              i32 const   mat {matrix[static_cast<usize>(filterX + filterY * Width)]};
                              red += srcBuffer[idx] * mat;
                              green += srcBuffer[idx + 1] * mat;
                              blue += srcBuffer[idx + 2] * mat;
                              if (incAlpha) {
                                  alpha += srcBuffer[idx + 3] * mat;
                              }
                          }
                      }

                      u8 const r {static_cast<u8>(std::clamp(factor * red + offset, 0.0, 255.0))};
                      u8 const g {static_cast<u8>(std::clamp(factor * green + offset, 0.0, 255.0))};
                      u8 const b {static_cast<u8>(std::clamp(factor * blue + offset, 0.0, 255.0))};
                      u8 const a {incAlpha ? static_cast<u8>(std::clamp(factor * alpha + offset, 0.0, 255.0)) : img.get_pixel({x, y}).A};
                      retValue.set_pixel({x, y}, {r, g, b, a});
                  });

    return retValue;
}

}
