// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ImageFilters.hpp"

#include <algorithm>
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
    auto const getColor {[](node const* target) -> color {
        return {
            static_cast<u8>(target->RedSum / target->PixelCount),
            static_cast<u8>(target->GreenSum / target->PixelCount),
            static_cast<u8>(target->BlueSum / target->PixelCount),
            255};
    }};

    if (n->IsLeaf) {
        if (n->PixelCount > 0) { pal.push_back(getColor(n)); }
        return;
    }

    for (auto const& child : n->Children) {
        if (child) {
            build_palette(child.get(), pal);
        }
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
                node* childPtr {current->Children[index].get()};
                if (!childPtr->IsEnqueued) {
                    _reducibleNodes[level + 1].push_back(childPtr);
                    childPtr->IsEnqueued = true;
                }
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

            n->IsEnqueued = false;

            if (!n->IsLeaf && n->ChildCount > 0) {
                merge_leaf_nodes(n, level);
                return;
            }
        }
    }
}

void octree_quantizer::merge_leaf_nodes(node* n, i32 level)
{
    _leafCount -= count_leaves(n);

    for (auto& child : n->Children) {
        if (child) {
            n->RedSum += child->RedSum;
            n->GreenSum += child->GreenSum;
            n->BlueSum += child->BlueSum;
            n->PixelCount += child->PixelCount;

            child.reset();
        }
    }

    n->ChildCount = 0;
    n->IsLeaf     = true;
    n->IsEnqueued = false;
    _leafCount++;

    if (n->Parent && !n->Parent->IsEnqueued && level > 0) {
        _reducibleNodes[level - 1].push_back(n->Parent);
        n->Parent->IsEnqueued = true;
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

////////////////////////////////////////////////////////////

neuquant::neuquant(i32 maxColors)
    : _maxColors {maxColors}
{
    _network.resize(_maxColors);
    for (i32 i {0}; i < _maxColors; ++i) {
        f64 const val {(i * 255.0) / std::max(1, _maxColors - 1)};
        _network[i] = {.r = val, .g = val, .b = val};
    }
}

auto neuquant::operator()(image const& img) -> image
{
    train(img);

    auto const& info {img.info()};
    image       retValue {image::CreateEmpty(info.Size, info.Format)};
    auto const  bpp {info.bytes_per_pixel()};

    locate_service<task_manager>().run_parallel(
        [&](par_task const& ctx) {
            for (isize i {ctx.Start}; i < ctx.End; ++i) {
                color const c {img.get_pixel(i * bpp)};
                i32 const   bmu {find_bmu(c)};

                color quant;
                quant.R = static_cast<u8>(_network[bmu].r);
                quant.G = static_cast<u8>(_network[bmu].g);
                quant.B = static_cast<u8>(_network[bmu].b);
                quant.A = 255;

                retValue.set_pixel(i * bpp, quant);
            }
        },
        info.Size.area());

    return retValue;
}

auto neuquant::get_palette() const -> std::vector<color>
{
    std::vector<color> palette;
    palette.reserve(_maxColors);

    for (auto const& n : _network) {
        palette.emplace_back(
            static_cast<u8>(std::clamp(n.r, 0.0, 255.0)),
            static_cast<u8>(std::clamp(n.g, 0.0, 255.0)),
            static_cast<u8>(std::clamp(n.b, 0.0, 255.0)),
            255);
    }

    return palette;
}

void neuquant::train(image const& img)
{
    auto const& info {img.info()};
    i32 const   lengthCount {info.Size.area()};
    i32 const   bpp {info.bytes_per_pixel()};

    i32 const cycleCount {100};
    i32 const sampleFac {30};
    i32 const step {sampleFac > 0 ? sampleFac : 1};
    u32 const primeStep {499};

    f64       alpha {1.0};
    f64       radius {std::max(static_cast<f64>(_maxColors >> 3), 2.0)};
    f64 const radDecay {radius / cycleCount};

    usize pixelPos {0};

    for (i32 i {0}; i < cycleCount; ++i) {
        if (alpha < 0.001) { break; }

        for (i32 j {0}; j < lengthCount; j += step) {
            color const c {img.get_pixel((pixelPos % lengthCount) * bpp)};

            i32 const bmu {find_bmu(c)};
            alter_neighbor(bmu, c, alpha);

            i32 const rad {static_cast<i32>(radius)};
            for (i32 k {1}; k <= rad; ++k) {
                f64 const distAlpha {alpha * ((radius - k) / radius)};
                if (bmu + k < _maxColors) { alter_neighbor(bmu + k, c, distAlpha); }
                if (bmu - k >= 0) { alter_neighbor(bmu - k, c, distAlpha); }
            }

            pixelPos += primeStep;
        }

        alpha -= (1.0 / cycleCount);
        radius -= radDecay;
    }
}

auto neuquant::find_bmu(color c) const -> i32
{
    i32 best_idx {0};
    f64 min_dist {1e30};

    for (i32 i {0}; i < _maxColors; ++i) {
        f64 const dr {_network[i].r - c.R};
        f64 const dg {_network[i].g - c.G};
        f64 const db {_network[i].b - c.B};
        f64 const dist {(dr * dr) + (dg * dg) + (db * db)};

        if (dist < min_dist) {
            min_dist = dist;
            best_idx = i;
        }
    }
    return best_idx;
}

void neuquant::alter_neighbor(i32 index, color c, f64 alpha)
{
    neuron& n {_network[index]};

    n.r += alpha * (static_cast<f64>(c.R) - n.r);
    n.g += alpha * (static_cast<f64>(c.G) - n.g);
    n.b += alpha * (static_cast<f64>(c.B) - n.b);
}

}
