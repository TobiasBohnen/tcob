// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ImageFilters.hpp"

namespace tcob::gfx {

using iterator = tcob::detail::counting_iterator<i32>;

////////////////////////////////////////////////////////////

auto blur_filter::get_factor() const -> f64
{
    return 1.0 / 9;
}

auto blur_filter::get_offset() const -> u8
{
    return 0;
}

auto blur_filter::get_matrix() const -> std::array<i32, 25>
{
    return {0, 0, 0, 0, 0,
            0, 1, 1, 1, 0,
            0, 1, 1, 1, 0,
            0, 1, 1, 1, 0,
            0, 0, 0, 0, 0};
}

////////////////////////////////////////////////////////////

auto edge_detect_filter::get_factor() const -> f64
{
    return 1.5;
}

auto edge_detect_filter::get_offset() const -> u8
{
    return 0;
}

auto edge_detect_filter::get_matrix() const -> std::array<i32, 9>
{
    return {0, 1, 0,
            1, -4, 1,
            0, 1, 0};
}

////////////////////////////////////////////////////////////

auto emboss_filter::get_factor() const -> f64
{
    return 1.0;
}

auto emboss_filter::get_offset() const -> u8
{
    return 128;
}

auto emboss_filter::get_matrix() const -> std::array<i32, 9>
{
    return {-1, -1, 0,
            -1, 0, 1,
            0, 1, 1};
}

////////////////////////////////////////////////////////////

auto edge_enhance_filter::get_factor() const -> f64
{
    return 2.0;
}

auto edge_enhance_filter::get_offset() const -> u8
{
    return 0;
}

auto edge_enhance_filter::get_matrix() const -> std::array<i32, 9>
{
    return {0, 0, 0,
            -1, 1, 0,
            0, 0, 0};
}

////////////////////////////////////////////////////////////

auto motion_blur_filter::get_factor() const -> f64
{
    return 1.0 / 9.0;
}

auto motion_blur_filter::get_offset() const -> u8
{
    return 0;
}

auto motion_blur_filter::get_matrix() const -> std::array<i32, 81>
{
    return {1, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 1, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 1, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 1, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 1, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 1, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 1, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 1, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 1};
}

////////////////////////////////////////////////////////////

auto sharpen_filter::get_factor() const -> f64
{
    return 1.0;
}

auto sharpen_filter::get_offset() const -> u8
{
    return 0;
}

auto sharpen_filter::get_matrix() const -> std::array<i32, 25>
{
    return {0, 0, 0, 0, 0,
            0, 0, -1, 0, 0,
            0, -1, 5, -1, 0,
            0, 0, -1, 0, 0,
            0, 0, 0, 0, 0};
}

////////////////////////////////////////////////////////////

auto grayscale_filter::operator()(image const& img) const -> image
{
    auto const& info {img.get_info()};
    auto        retValue {image::CreateEmpty(info.Size, info.Format)};
    i32 const   bpp {info.bytes_per_pixel()};

    auto const srcBuffer {img.get_data()};
    auto       dstBuffer {retValue.get_data()};

    auto const [imgWidth, imgHeight] {info.Size};

    std::for_each(std::execution::par_unseq,
                  iterator {0},
                  iterator {imgWidth * imgHeight},
                  [&](i32 pixIdx) {
                      i32 const idx {pixIdx * bpp};
                      u8 const  value {static_cast<u8>(srcBuffer[idx] * RedFactor + srcBuffer[idx + 1] * GreenFactor + srcBuffer[idx + 2] * BlueFactor)};
                      dstBuffer[idx] = dstBuffer[idx + 1] = dstBuffer[idx + 2] = value;
                      if (info.Format == image::format::RGBA) {
                          dstBuffer[idx + 3] = srcBuffer[idx + 3];
                      }
                  });

    return retValue;
}

////////////////////////////////////////////////////////////

auto resize_nearest_neighbor::operator()(image const& img) const -> image
{
    auto const [newWidth, newHeight] {NewSize};

    auto const& info {img.get_info()};
    auto const [imgWidth, imgHeight] {info.Size};

    f64 const xFactor {static_cast<f64>(imgWidth) / newWidth};
    f64 const yFactor {static_cast<f64>(imgHeight) / newHeight};

    auto retValue {image::CreateEmpty(NewSize, info.Format)};

    std::for_each(std::execution::par_unseq,
                  iterator {0},
                  iterator {newHeight * newWidth},
                  [&](i32 pixIdx) {
                      i32 const x {pixIdx % newWidth};
                      i32 const y {pixIdx / newWidth};

                      i32 const srcY {static_cast<i32>(y * yFactor)};
                      i32 const srcX {static_cast<i32>(x * xFactor)};

                      retValue.set_pixel({x, y}, img.get_pixel({srcX, srcY}));
                  });

    return retValue;
}

////////////////////////////////////////////////////////////

auto resize_bilinear::operator()(image const& img) const -> image
{
    auto const [newWidth, newHeight] {NewSize};

    auto const& info {img.get_info()};
    auto const [width, height] {info.Size};

    f64 const xFactor {static_cast<f64>(width) / newWidth};
    f64 const yFactor {static_cast<f64>(height) / newHeight};
    i32 const ymax {height - 1};
    i32 const xmax {width - 1};

    auto retValue {image::CreateEmpty(NewSize, info.Format)};

    std::for_each(std::execution::par_unseq,
                  iterator {0},
                  iterator {newHeight * newWidth},
                  [&](i32 pixIdx) {
                      i32 const x {pixIdx % newWidth};
                      i32 const y {pixIdx / newWidth};

                      f64 const oy {y * yFactor};
                      i32 const y1 {static_cast<i32>(oy)};
                      i32 const y2 {(y1 == ymax) ? y1 : y1 + 1};

                      f64 const ox {x * xFactor};
                      i32 const x1 {static_cast<i32>(ox)};
                      i32 const x2 {(x1 == xmax) ? x1 : x1 + 1};

                      auto const c1 {img.get_pixel({x1, y1})};
                      auto const c2 {img.get_pixel({x2, y1})};
                      auto const c3 {img.get_pixel({x1, y2})};
                      auto const c4 {img.get_pixel({x2, y2})};

                      f64 const dy1 {oy - y1};
                      f64 const dy2 {1.0 - dy1};
                      f64 const dx1 {ox - x1};
                      f64 const dx2 {1.0 - dx1};

                      u8 const r {static_cast<u8>(dy2 * (dx2 * c1.R + dx1 * c2.R) + dy1 * (dx2 * c3.R + dx1 * c4.R))};
                      u8 const g {static_cast<u8>(dy2 * (dx2 * c1.G + dx1 * c2.G) + dy1 * (dx2 * c3.G + dx1 * c4.G))};
                      u8 const b {static_cast<u8>(dy2 * (dx2 * c1.B + dx1 * c2.B) + dy1 * (dx2 * c3.B + dx1 * c4.B))};
                      u8 const a {static_cast<u8>(dy2 * (dx2 * c1.A + dx1 * c2.A) + dy1 * (dx2 * c3.A + dx1 * c4.A))};

                      retValue.set_pixel({x, y}, {r, g, b, a});
                  });

    return retValue;
}

////////////////////////////////////////////////////////////

}
