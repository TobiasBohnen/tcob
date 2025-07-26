// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ImageFilters.hpp"

#include <array>

#include "tcob/core/Color.hpp"
#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/TaskManager.hpp"
#include "tcob/gfx/Image.hpp"

namespace tcob::gfx {

////////////////////////////////////////////////////////////

auto blur_filter::factor() const -> f64
{
    return 1.0 / 9;
}

auto blur_filter::offset() const -> u8
{
    return 0;
}

auto blur_filter::matrix() const -> std::array<i32, 25>
{
    return {0, 0, 0, 0, 0,
            0, 1, 1, 1, 0,
            0, 1, 1, 1, 0,
            0, 1, 1, 1, 0,
            0, 0, 0, 0, 0};
}

////////////////////////////////////////////////////////////

auto edge_detect_filter::factor() const -> f64
{
    return 1.5;
}

auto edge_detect_filter::offset() const -> u8
{
    return 0;
}

auto edge_detect_filter::matrix() const -> std::array<i32, 9>
{
    return {0, 1, 0,
            1, -4, 1,
            0, 1, 0};
}

////////////////////////////////////////////////////////////

auto emboss_filter::factor() const -> f64
{
    return 1.0;
}

auto emboss_filter::offset() const -> u8
{
    return 128;
}

auto emboss_filter::matrix() const -> std::array<i32, 9>
{
    return {-1, -1, 0,
            -1, 0, 1,
            0, 1, 1};
}

////////////////////////////////////////////////////////////

auto edge_enhance_filter::factor() const -> f64
{
    return 2.0;
}

auto edge_enhance_filter::offset() const -> u8
{
    return 0;
}

auto edge_enhance_filter::matrix() const -> std::array<i32, 9>
{
    return {0, 0, 0,
            -1, 1, 0,
            0, 0, 0};
}

////////////////////////////////////////////////////////////

auto motion_blur_filter::factor() const -> f64
{
    return 1.0 / 9.0;
}

auto motion_blur_filter::offset() const -> u8
{
    return 0;
}

auto motion_blur_filter::matrix() const -> std::array<i32, 81>
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

auto sharpen_filter::factor() const -> f64
{
    return 1.0;
}

auto sharpen_filter::offset() const -> u8
{
    return 0;
}

auto sharpen_filter::matrix() const -> std::array<i32, 25>
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
    auto const& info {img.info()};
    auto        retValue {image::CreateEmpty(info.Size, info.Format)};
    i32 const   bpp {info.bytes_per_pixel()};

    auto const srcBuffer {img.data()};
    auto       dstBuffer {retValue.data()};

    auto const [imgWidth, imgHeight] {info.Size};

    locate_service<task_manager>().run_parallel(
        [&](par_task const& ctx) {
            for (isize pixIdx {ctx.Start}; pixIdx < ctx.End; ++pixIdx) {
                isize const idx {pixIdx * bpp};
                u8 const    value {static_cast<u8>((srcBuffer[idx] * RedFactor) + (srcBuffer[idx + 1] * GreenFactor) + (srcBuffer[idx + 2] * BlueFactor))};
                dstBuffer[idx] = dstBuffer[idx + 1] = dstBuffer[idx + 2] = value;
                if (info.Format == image::format::RGBA) {
                    dstBuffer[idx + 3] = srcBuffer[idx + 3];
                }
            }
        },
        imgWidth * imgHeight);

    return retValue;
}

////////////////////////////////////////////////////////////

auto resize_nearest_neighbor::operator()(image const& img) const -> image
{
    auto const [newWidth, newHeight] {NewSize};

    auto const& info {img.info()};
    auto const [imgWidth, imgHeight] {info.Size};

    f64 const xFactor {static_cast<f64>(imgWidth) / newWidth};
    f64 const yFactor {static_cast<f64>(imgHeight) / newHeight};

    auto retValue {image::CreateEmpty(NewSize, info.Format)};

    locate_service<task_manager>().run_parallel(
        [&](par_task const& ctx) {
            for (isize pixIdx {ctx.Start}; pixIdx < ctx.End; ++pixIdx) {
                i32 const x {static_cast<i32>(pixIdx % newWidth)};
                i32 const y {static_cast<i32>(pixIdx / newWidth)};

                i32 const srcY {static_cast<i32>(y * yFactor)};
                i32 const srcX {static_cast<i32>(x * xFactor)};

                retValue.set_pixel({x, y}, img.get_pixel({srcX, srcY}));
            }
        },
        newWidth * newHeight);

    return retValue;
}

////////////////////////////////////////////////////////////

auto resize_bilinear::operator()(image const& img) const -> image
{
    auto const [newWidth, newHeight] {NewSize};

    auto const& info {img.info()};
    auto const [width, height] {info.Size};

    f64 const xFactor {static_cast<f64>(width) / newWidth};
    f64 const yFactor {static_cast<f64>(height) / newHeight};
    i32 const ymax {height - 1};
    i32 const xmax {width - 1};

    auto retValue {image::CreateEmpty(NewSize, info.Format)};

    locate_service<task_manager>().run_parallel(
        [&](par_task const& ctx) {
            for (isize pixIdx {ctx.Start}; pixIdx < ctx.End; ++pixIdx) {
                i32 const x {static_cast<i32>(pixIdx % newWidth)};
                i32 const y {static_cast<i32>(pixIdx / newWidth)};

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

                u8 const r {static_cast<u8>((dy2 * (dx2 * c1.R + dx1 * c2.R)) + (dy1 * (dx2 * c3.R + dx1 * c4.R)))};
                u8 const g {static_cast<u8>((dy2 * (dx2 * c1.G + dx1 * c2.G)) + (dy1 * (dx2 * c3.G + dx1 * c4.G)))};
                u8 const b {static_cast<u8>((dy2 * (dx2 * c1.B + dx1 * c2.B)) + (dy1 * (dx2 * c3.B + dx1 * c4.B)))};
                u8 const a {static_cast<u8>((dy2 * (dx2 * c1.A + dx1 * c2.A)) + (dy1 * (dx2 * c3.A + dx1 * c4.A)))};

                retValue.set_pixel({x, y}, {r, g, b, a});
            }
        },
        newWidth * newHeight);

    return retValue;
}

////////////////////////////////////////////////////////////

auto remove_alpha::operator()(image const& img) const -> image
{
    if (!image::information::HasAlpha(img.info().Format)) {
        return img;
    }

    auto const& info {img.info()};
    auto const [width, height] {info.Size};

    auto retValue {image::CreateEmpty(info.Size, image::format::RGB)};

    locate_service<task_manager>().run_parallel(
        [&](par_task const& ctx) {
            for (isize pixIdx {ctx.Start}; pixIdx < ctx.End; ++pixIdx) {
                i32 const x {static_cast<i32>(pixIdx % width)};
                i32 const y {static_cast<i32>(pixIdx / width)};

                color const src {img.get_pixel({x, y})};
                retValue.set_pixel({x, y}, color {src.R, src.G, src.B});
            }
        },
        width * height);

    return retValue;
}

}
