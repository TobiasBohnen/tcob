// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ImageFilters.hpp"

#include <array>
#include <memory>
#include <vector>

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

auto grayscale_filter::operator()(image const& img) -> image
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

auto resize_nearest_neighbor::operator()(image const& img) -> image
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

auto resize_bilinear::operator()(image const& img) -> image
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

auto remove_alpha::operator()(image const& img) -> image
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

////////////////////////////////////////////////////////////

octree_quantizer::octree_quantizer(i32 maxColors)
    : _maxColors {maxColors}
    , _root {std::make_unique<node>()}
{
}

auto octree_quantizer::operator()(image const& img) -> image
{
    auto const& info {img.info()};

    auto const [width, height] {info.Size};
    for (i32 y {0}; y < height; ++y) {
        for (i32 x {0}; x < width; ++x) {
            insert_color(img.get_pixel({x, y}));
        }
    }

    while (_leafCount > _maxColors) {
        reduce();
    }

    image      retValue {image::CreateEmpty(info.Size, info.Format)};
    auto const bpp {info.bytes_per_pixel()};

    locate_service<task_manager>().run_parallel(
        [&](par_task const& ctx) {
            for (isize pixIdx {ctx.Start}; pixIdx < ctx.End; ++pixIdx) {
                retValue.set_pixel(pixIdx * bpp, get_quantized_color(img.get_pixel(pixIdx * bpp)));
            }
        },

        width * height);

    return retValue;
}

auto octree_quantizer::get_palette() const -> std::vector<color>
{
    std::vector<color> palette;
    palette.reserve(_leafCount);
    build_palette(_root.get(), palette);
    return palette;
}

void octree_quantizer::build_palette(node const* n, std::vector<color>& pal) const
{
    if (n->IsLeaf) {
        if (n->PixelCount > 0) {
            pal.emplace_back(static_cast<u8>(n->RedSum / n->PixelCount),
                             static_cast<u8>(n->GreenSum / n->PixelCount),
                             static_cast<u8>(n->BlueSum / n->PixelCount),
                             255);
        }
        return;
    }

    i32 totalPixelCount {0};
    for (auto const& child : n->Children) {
        if (child) {
            totalPixelCount += child->PixelCount;
            build_palette(child.get(), pal);
        }
    }

    if (n->PixelCount > totalPixelCount) {
        pal.emplace_back(
            static_cast<u8>(n->RedSum / n->PixelCount),
            static_cast<u8>(n->GreenSum / n->PixelCount),
            static_cast<u8>(n->BlueSum / n->PixelCount),
            255);
    }
}

void octree_quantizer::insert_color(color c)
{
    node* current {_root.get()};

    for (i32 level {0}; level < MAX_TREE_DEPTH; ++level) {
        if (current->IsLeaf) {
            break;
        }

        i32 const shift {7 - level};
        i32 const index {((c.R >> shift) & 1) << 2 | ((c.G >> shift) & 1) << 1 | ((c.B >> shift) & 1)};

        auto& child {current->Children[index]};
        if (!child) {
            child                = std::make_unique<node>();
            child->Parent        = current;
            child->IndexInParent = index;
            current->ChildCount++;

            if (level < MAX_TREE_DEPTH - 1) {
                _reducibleNodes[level + 1].push_back(current->Children[index].get());
            } else {
                child->IsLeaf = true;
                _leafCount++;
            }
        }

        current = child.get();
    }

    current->RedSum += c.R;
    current->GreenSum += c.G;
    current->BlueSum += c.B;
    current->PixelCount++;
}

void octree_quantizer::reduce()
{
    for (i32 level {MAX_TREE_DEPTH - 1}; level >= 0; --level) {
        auto& list {_reducibleNodes[level]};

        while (!list.empty()) {
            node* n {list.back()};
            list.pop_back();

            if (!n->IsLeaf && n->ChildCount > 0) {
                merge_leaf_nodes(n, level);
                return;
            }
        }
    }
}

void octree_quantizer::merge_leaf_nodes(node* n, i32 level)
{
    for (auto& child : n->Children) {
        if (child) {
            n->RedSum += child->RedSum;
            n->GreenSum += child->GreenSum;
            n->BlueSum += child->BlueSum;
            n->PixelCount += child->PixelCount;

            if (child->IsLeaf) {
                _leafCount--;
            } else {
                _leafCount -= count_leaves(child.get());
            }
            child.reset();
            n->ChildCount--;

            if (_leafCount < _maxColors) {
                return;
            }
        }
    }

    n->IsLeaf = true;
    _leafCount++;

    if (n->Parent && n->Parent != n && level > 0) {
        _reducibleNodes[level - 1].push_back(n->Parent);
    }
}

auto octree_quantizer::count_leaves(node* n) -> i32
{
    i32 retValue {0};
    for (auto& child : n->Children) {
        if (child) {
            if (child->IsLeaf) {
                retValue++;
            } else {
                retValue += count_leaves(child.get());
            }
        }
    }
    return retValue;
}

auto octree_quantizer::get_quantized_color(color c) const -> color
{
    node* current {_root.get()};

    for (i32 level {0}; level < MAX_TREE_DEPTH; ++level) {
        if (current->IsLeaf) {
            break;
        }

        i32 const shift {7 - level};
        i32 const index {((c.R >> shift) & 1) << 2 | ((c.G >> shift) & 1) << 1 | ((c.B >> shift) & 1)};

        if (current->Children[index]) {
            current = current->Children[index].get();
        } else {
            break;
        }
    }

    if (current->PixelCount == 0) { return colors::Transparent; }

    return {static_cast<u8>(current->RedSum / current->PixelCount),
            static_cast<u8>(current->GreenSum / current->PixelCount),
            static_cast<u8>(current->BlueSum / current->PixelCount),
            255};
}

}
