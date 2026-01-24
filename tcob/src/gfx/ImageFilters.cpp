// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ImageFilters.hpp"

#include <algorithm>
#include <array>
#include <memory>
#include <utility>
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
    build_tree(img);

    image retValue {image::CreateEmpty(info.Size, info.Format)};

    locate_service<task_manager>().run_parallel(
        [&](par_task const& ctx) {
            for (isize pixIdx {ctx.Start}; pixIdx < ctx.End; ++pixIdx) {
                retValue.set_pixel(pixIdx, get_quantized_color(img.get_pixel(pixIdx)));
            }
        },
        width * height);

    return retValue;
}

auto octree_quantizer::GetPalette(image const& img, i32 maxColors) -> std::vector<color>
{
    octree_quantizer quant {maxColors};
    quant.build_tree(img);
    std::vector<color> palette;
    palette.reserve(quant._leafCount);
    quant.build_palette(quant._root.get(), palette);
    return palette;
}

void octree_quantizer::build_tree(image const& img)
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
        _network[i] = {.R = val, .G = val, .B = val};
    }
}

auto neuquant::operator()(image const& img) -> image
{
    train(img);

    auto const& info {img.info()};
    image       retValue {image::CreateEmpty(info.Size, info.Format)};

    locate_service<task_manager>().run_parallel(
        [&](par_task const& ctx) {
            for (isize i {ctx.Start}; i < ctx.End; ++i) {
                color const c {img.get_pixel(i)};
                i32 const   bmu {find_bmu(c)};

                color const quant {static_cast<u8>(_network[bmu].R),
                                   static_cast<u8>(_network[bmu].G),
                                   static_cast<u8>(_network[bmu].B)};

                retValue.set_pixel(i, quant);
            }
        },
        info.Size.area());

    return retValue;
}

auto neuquant::GetPalette(image const& img, i32 maxColors) -> std::vector<color>
{
    neuquant quant {maxColors};
    quant.train(img);
    std::vector<color> palette;
    palette.reserve(quant._maxColors);

    for (auto const& n : quant._network) {
        palette.emplace_back(
            static_cast<u8>(std::clamp(n.R, 0.0, 255.0)),
            static_cast<u8>(std::clamp(n.G, 0.0, 255.0)),
            static_cast<u8>(std::clamp(n.B, 0.0, 255.0)),
            255);
    }

    return palette;
}

void neuquant::train(image const& img)
{
    auto const& info {img.info()};
    i32 const   lengthCount {info.Size.area()};

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
            color const c {img.get_pixel((pixelPos % lengthCount))};
            i32 const   bmu {find_bmu(c)};
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
    i32 bestIdx {0};
    f64 minDist {1e30};

    for (i32 i {0}; i < _maxColors; ++i) {
        f64 const dr {_network[i].R - c.R};
        f64 const dg {_network[i].G - c.G};
        f64 const db {_network[i].B - c.B};
        f64 const dist {(dr * dr) + (dg * dg) + (db * db)};

        if (dist < minDist) {
            minDist = dist;
            bestIdx = i;
        }
    }
    return bestIdx;
}

void neuquant::alter_neighbor(i32 index, color c, f64 alpha)
{
    neuron& n {_network[index]};

    n.R += alpha * (static_cast<f64>(c.R) - n.R);
    n.G += alpha * (static_cast<f64>(c.G) - n.G);
    n.B += alpha * (static_cast<f64>(c.B) - n.B);
}

////////////////////////////////////////////////////////////

ditherer_base::ditherer_base(std::vector<color> palette)
    : _palette {std::move(palette)}
    , _tree {{{0, 0, 0}, {255, 255, 255}}}
{
    for (u32 i {0}; i < _palette.size(); ++i) {
        color const c {_palette[i]};
        _tree.add({.Position = {static_cast<f32>(c.R), static_cast<f32>(c.G), static_cast<f32>(c.B)}, .ID = i});
    }
}

auto ditherer_base::operator()(image const& img) -> image
{
    auto const& info {img.info()};
    auto const  width {info.Size.Width};
    auto const  height {info.Size.Height};

    image      retValue {image::CreateEmpty(info.Size, info.Format)};
    auto const inds {to_indexed(img)};
    for (isize idx {0}; idx < width * height; ++idx) {
        retValue.set_pixel(idx, get_color(inds[idx]));
    }

    return retValue;
}

auto ditherer_base::find_nearest(f64 r, f64 g, f64 b) const -> u32
{
    return _tree.find_nearest(std::array<f32, 3> {{static_cast<f32>(r), static_cast<f32>(g), static_cast<f32>(b)}})->ID;
}

auto ditherer_base::get_color(u32 idx) const -> color
{
    return _palette[idx];
}

////////////////////////////////////////////////////////////

ordered_dither::ordered_dither(std::vector<color> palette)
    : ditherer_base {std::move(palette)}
{
}

auto ordered_dither::to_indexed(image const& img) -> std::vector<u32>
{
    auto const&      info {img.info()};
    std::vector<u32> retValue;
    retValue.resize(info.Size.area());
    auto const height {info.Size.Height};
    auto const width {info.Size.Width};

    for (i32 y {0}; y < height; ++y) {
        for (i32 x {0}; x < width; ++x) {
            isize const idx {(y * width + x)};

            color const original {img.get_pixel(idx)};
            f64 const   threshold {get_threshold(x, y)};

            f64 const r {std::clamp(static_cast<f64>(original.R) + threshold, 0.0, 255.0)};
            f64 const g {std::clamp(static_cast<f64>(original.G) + threshold, 0.0, 255.0)};
            f64 const b {std::clamp(static_cast<f64>(original.B) + threshold, 0.0, 255.0)};

            retValue[idx] = find_nearest(r, g, b);
        }
    }

    return retValue;
}

auto ordered_dither::get_threshold(i32 x, i32 y) const -> f64
{
    i32 const mx {x % 8};
    i32 const my {y % 8};
    f64 const normalized {bayer8x8[(my * 8) + mx]};
    return (normalized - 0.5) * 64.0;
}

////////////////////////////////////////////////////////////

atkinson_dither::atkinson_dither(std::vector<color> palette)
    : ditherer_base {std::move(palette)}
{
}

auto atkinson_dither::to_indexed(image const& img) -> std::vector<u32>
{
    auto const&      info {img.info()};
    std::vector<u32> retValue;
    retValue.resize(info.Size.area());
    auto const width {info.Size.Width};
    auto const height {info.Size.Height};

    std::vector<f64> currErrors(width * 3, 0.0);
    std::vector<f64> nextErrors(width * 3, 0.0);
    std::vector<f64> next2Errors(width * 3, 0.0);

    for (i32 y {0}; y < height; ++y) {
        for (i32 x {0}; x < width; ++x) {
            isize const idx {(y * width + x)};
            color const original {img.get_pixel(idx)};

            f64 const r {std::clamp(static_cast<f64>(original.R) + currErrors[(x * 3) + 0], 0.0, 255.0)};
            f64 const g {std::clamp(static_cast<f64>(original.G) + currErrors[(x * 3) + 1], 0.0, 255.0)};
            f64 const b {std::clamp(static_cast<f64>(original.B) + currErrors[(x * 3) + 2], 0.0, 255.0)};

            retValue[idx] = find_nearest(r, g, b);

            color const quantized {get_color(retValue[idx])};
            f64 const   errR {r - static_cast<f64>(quantized.R)};
            f64 const   errG {g - static_cast<f64>(quantized.G)};
            f64 const   errB {b - static_cast<f64>(quantized.B)};

            f64 const factor {1.0 / 8.0};

            if (x + 1 < width) {
                currErrors[((x + 1) * 3) + 0] += errR * factor;
                currErrors[((x + 1) * 3) + 1] += errG * factor;
                currErrors[((x + 1) * 3) + 2] += errB * factor;
            }

            if (x + 2 < width) {
                currErrors[((x + 2) * 3) + 0] += errR * factor;
                currErrors[((x + 2) * 3) + 1] += errG * factor;
                currErrors[((x + 2) * 3) + 2] += errB * factor;
            }

            if (y + 1 < height) {
                if (x > 0) {
                    nextErrors[((x - 1) * 3) + 0] += errR * factor;
                    nextErrors[((x - 1) * 3) + 1] += errG * factor;
                    nextErrors[((x - 1) * 3) + 2] += errB * factor;
                }
                nextErrors[(x * 3) + 0] += errR * factor;
                nextErrors[(x * 3) + 1] += errG * factor;
                nextErrors[(x * 3) + 2] += errB * factor;
                if (x + 1 < width) {
                    nextErrors[((x + 1) * 3) + 0] += errR * factor;
                    nextErrors[((x + 1) * 3) + 1] += errG * factor;
                    nextErrors[((x + 1) * 3) + 2] += errB * factor;
                }
            }
            if (y + 2 < height) {
                next2Errors[(x * 3) + 0] += errR * factor;
                next2Errors[(x * 3) + 1] += errG * factor;
                next2Errors[(x * 3) + 2] += errB * factor;
            }
        }

        currErrors.swap(nextErrors);
        nextErrors.swap(next2Errors);
        std::ranges::fill(next2Errors, 0.0);
    }

    return retValue;
}

////////////////////////////////////////////////////////////

floyd_steinberg_dither::floyd_steinberg_dither(std::vector<color> palette)
    : ditherer_base {std::move(palette)}
{
}

auto floyd_steinberg_dither::to_indexed(image const& img) -> std::vector<u32>
{
    auto const&      info {img.info()};
    std::vector<u32> retValue;
    retValue.resize(info.Size.area());
    auto const width {info.Size.Width};
    auto const height {info.Size.Height};

    std::vector<f64> currErrors(width * 3, 0.0);
    std::vector<f64> nextErrors(width * 3, 0.0);

    constexpr f64 coeff_7_16 {7.0 / 16.0};
    constexpr f64 coeff_3_16 {3.0 / 16.0};
    constexpr f64 coeff_5_16 {5.0 / 16.0};
    constexpr f64 coeff_1_16 {1.0 / 16.0};

    for (i32 y {0}; y < height; ++y) {
        for (i32 x {0}; x < width; ++x) {
            isize const idx {(y * width + x)};
            color const original {img.get_pixel(idx)};

            f64 const r {std::clamp(static_cast<f64>(original.R) + currErrors[(x * 3) + 0], 0.0, 255.0)};
            f64 const g {std::clamp(static_cast<f64>(original.G) + currErrors[(x * 3) + 1], 0.0, 255.0)};
            f64 const b {std::clamp(static_cast<f64>(original.B) + currErrors[(x * 3) + 2], 0.0, 255.0)};

            retValue[idx] = find_nearest(r, g, b);

            color const quantized {get_color(retValue[idx])};

            f64 const errR {r - static_cast<f64>(quantized.R)};
            f64 const errG {g - static_cast<f64>(quantized.G)};
            f64 const errB {b - static_cast<f64>(quantized.B)};

            if (x + 1 < width) {
                currErrors[((x + 1) * 3) + 0] += errR * coeff_7_16;
                currErrors[((x + 1) * 3) + 1] += errG * coeff_7_16;
                currErrors[((x + 1) * 3) + 2] += errB * coeff_7_16;
            }

            if (y + 1 < height) {
                if (x > 0) {
                    nextErrors[((x - 1) * 3) + 0] += errR * coeff_3_16;
                    nextErrors[((x - 1) * 3) + 1] += errG * coeff_3_16;
                    nextErrors[((x - 1) * 3) + 2] += errB * coeff_3_16;
                }
                nextErrors[(x * 3) + 0] += errR * coeff_5_16;
                nextErrors[(x * 3) + 1] += errG * coeff_5_16;
                nextErrors[(x * 3) + 2] += errB * coeff_5_16;
                if (x + 1 < width) {
                    nextErrors[((x + 1) * 3) + 0] += errR * coeff_1_16;
                    nextErrors[((x + 1) * 3) + 1] += errG * coeff_1_16;
                    nextErrors[((x + 1) * 3) + 2] += errB * coeff_1_16;
                }
            }
        }

        currErrors.swap(nextErrors);
        std::ranges::fill(nextErrors, 0.0);
    }

    return retValue;
}

}
