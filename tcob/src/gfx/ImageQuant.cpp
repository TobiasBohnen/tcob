// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ImageQuant.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <memory>
#include <utility>
#include <vector>

#include "tcob/core/Color.hpp"
#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/core/TaskManager.hpp"
#include "tcob/gfx/Image.hpp"

namespace tcob::gfx {

////////////////////////////////////////////////////////////

octree_quant::octree_quant(i32 maxColors)
    : _maxColors {maxColors}
    , _root {std::make_unique<node>()}
{
}

auto octree_quant::GetPalette(image const& img, i32 maxColors) -> std::vector<color>
{
    octree_quant quant {maxColors};
    quant.build_tree(img);
    std::vector<color> palette;
    palette.reserve(quant._leafCount);
    quant.build_palette(quant._root.get(), palette);
    return palette;
}

void octree_quant::build_tree(image const& img)
{
    auto const& info {img.info()};

    auto const [w, h] {info.Size};
    for (i32 y {0}; y < h; ++y) {
        for (i32 x {0}; x < w; ++x) {
            insert_color(img.get_pixel({x, y}));
        }
    }

    while (_leafCount > _maxColors) {
        reduce();
    }
}

void octree_quant::build_palette(node const* n, std::vector<color>& pal) const
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

void octree_quant::insert_color(color c)
{
    node* current {_root.get()};

    for (i32 level {0}; level < MAX_TREE_DEPTH; ++level) {
        if (current->IsLeaf) { break; }

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

void octree_quant::reduce()
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

void octree_quant::merge_leaf_nodes(node* n, i32 level)
{
    for (auto& child : n->Children) {
        if (child) {
            assert(child->IsLeaf);
            _leafCount--;

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

auto octree_quant::get_quantized_color(color c) const -> color
{
    node* current {_root.get()};

    for (i32 level {0}; level < MAX_TREE_DEPTH; ++level) {
        if (current->IsLeaf) { break; }

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

auto neuquant::GetPalette(image const& img, i32 maxColors) -> std::vector<color>
{
    neuquant quant {maxColors};
    quant.train(img);
    std::vector<color> palette;
    palette.reserve(maxColors);

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
    i32 const step {std::clamp(lengthCount / (_maxColors * 30), 1, 30)};
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

            for (i32 k {1}; k <= static_cast<i32>(radius); ++k) {
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
        _tree.add({.Position = {static_cast<f64>(c.R), static_cast<f64>(c.G), static_cast<f64>(c.B)}, .ID = i});
    }
}

auto ditherer_base::operator()(image const& img) const -> image
{
    auto const& info {img.info()};

    image      retValue {image::CreateEmpty(info.Size, info.Format)};
    auto const inds {to_indexed(img)};
    for (isize idx {0}; idx < info.Size.area(); ++idx) {
        retValue.set_pixel(idx, get_color(inds[idx]));
    }

    return retValue;
}

auto ditherer_base::find_nearest(f64 r, f64 g, f64 b) const -> u32
{
    return _tree.find_nearest(std::array<f64, 3> {r, g, b})->ID;
}

auto ditherer_base::get_color(u32 idx) const -> color
{
    return _palette[idx];
}

////////////////////////////////////////////////////////////

auto nearest_dither::to_indexed(image const& img) const -> std::vector<u32>
{
    auto const& info {img.info()};
    auto const  area {info.Size.area()};

    std::vector<u32> retValue(area);

    locate_service<task_manager>().run_parallel(
        [&](par_task const& ctx) {
            for (isize i {ctx.Start}; i < ctx.End; ++i) {
                color const c {img.get_pixel(i)};
                retValue[i] = find_nearest(
                    static_cast<f64>(c.R),
                    static_cast<f64>(c.G),
                    static_cast<f64>(c.B));
            }
        },
        area);

    return retValue;
}

////////////////////////////////////////////////////////////

bayer_dither::bayer_dither(std::vector<color> palette, bayer_matrix matrix)
    : ditherer_base {std::move(palette)}
{
    switch (matrix) {
    case bayer_matrix::Bayer2x2:
        _matrix     = Bayer2x2;
        _matrixSize = 2;
        break;
    case bayer_matrix::Bayer4x4:
        _matrix     = Bayer4x4;
        _matrixSize = 4;
        break;
    case bayer_matrix::Bayer8x8:
        _matrix     = Bayer8x8;
        _matrixSize = 8;
        break;
    }
}

auto bayer_dither::to_indexed(image const& img) const -> std::vector<u32>
{
    auto const& info {img.info()};

    std::vector<u32> retValue;
    retValue.resize(info.Size.area());
    auto const [w, h] {info.Size};

    for (i32 y {0}; y < h; ++y) {
        for (i32 x {0}; x < w; ++x) {
            isize const idx {(y * w + x)};

            color const original {img.get_pixel(idx)};
            f64 const   threshold {(_matrix[((y % _matrixSize) * _matrixSize) + (x % _matrixSize)] - 0.5) * 64.0};

            f64 const r {std::clamp(static_cast<f64>(original.R) + threshold, 0.0, 255.0)};
            f64 const g {std::clamp(static_cast<f64>(original.G) + threshold, 0.0, 255.0)};
            f64 const b {std::clamp(static_cast<f64>(original.B) + threshold, 0.0, 255.0)};

            retValue[idx] = find_nearest(r, g, b);
        }
    }

    return retValue;
}

////////////////////////////////////////////////////////////

value_noise_dither::value_noise_dither(std::vector<color> palette, size_i gridSize, u64 seed)
    : ditherer_base {std::move(palette)}
    , _noise {gridSize, 1.0f, seed}
{
}

auto value_noise_dither::to_indexed(image const& img) const -> std::vector<u32>
{
    auto const& info {img.info()};

    std::vector<u32> retValue;
    retValue.resize(info.Size.area());
    auto const [w, h] {info.Size};

    for (i32 y {0}; y < h; ++y) {
        for (i32 x {0}; x < w; ++x) {
            isize const idx {(y * w) + x};
            color const original {img.get_pixel(idx)};
            f64 const   threshold {(static_cast<f64>(_noise({static_cast<f32>(x) / w, static_cast<f32>(y) / h})) - 0.5) * 32.0};

            f64 const r {std::clamp(static_cast<f64>(original.R) + threshold, 0.0, 255.0)};
            f64 const g {std::clamp(static_cast<f64>(original.G) + threshold, 0.0, 255.0)};
            f64 const b {std::clamp(static_cast<f64>(original.B) + threshold, 0.0, 255.0)};

            retValue[idx] = find_nearest(r, g, b);
        }
    }

    return retValue;
}

////////////////////////////////////////////////////////////

atkinson_dither::atkinson_dither(std::vector<color> palette)
    : ditherer_base {std::move(palette)}
{
}

auto atkinson_dither::to_indexed(image const& img) const -> std::vector<u32>
{
    auto const& info {img.info()};

    std::vector<u32> retValue;
    retValue.resize(info.Size.area());
    auto const [w, h] {info.Size};

    std::vector<f64> currErrors(w * 3, 0.0);
    std::vector<f64> nextErrors(w * 3, 0.0);
    std::vector<f64> next2Errors(w * 3, 0.0);

    for (i32 y {0}; y < h; ++y) {
        for (i32 x {0}; x < w; ++x) {
            isize const idx {(y * w + x)};
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

            if (x + 1 < w) {
                currErrors[((x + 1) * 3) + 0] += errR * factor;
                currErrors[((x + 1) * 3) + 1] += errG * factor;
                currErrors[((x + 1) * 3) + 2] += errB * factor;
            }

            if (x + 2 < w) {
                currErrors[((x + 2) * 3) + 0] += errR * factor;
                currErrors[((x + 2) * 3) + 1] += errG * factor;
                currErrors[((x + 2) * 3) + 2] += errB * factor;
            }

            if (y + 1 < h) {
                if (x > 0) {
                    nextErrors[((x - 1) * 3) + 0] += errR * factor;
                    nextErrors[((x - 1) * 3) + 1] += errG * factor;
                    nextErrors[((x - 1) * 3) + 2] += errB * factor;
                }
                nextErrors[(x * 3) + 0] += errR * factor;
                nextErrors[(x * 3) + 1] += errG * factor;
                nextErrors[(x * 3) + 2] += errB * factor;
                if (x + 1 < w) {
                    nextErrors[((x + 1) * 3) + 0] += errR * factor;
                    nextErrors[((x + 1) * 3) + 1] += errG * factor;
                    nextErrors[((x + 1) * 3) + 2] += errB * factor;
                }
            }
            if (y + 2 < h) {
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

auto floyd_steinberg_dither::to_indexed(image const& img) const -> std::vector<u32>
{
    auto const& info {img.info()};

    std::vector<u32> retValue;
    retValue.resize(info.Size.area());
    auto const [w, h] {info.Size};

    std::vector<f64> currErrors(w * 3, 0.0);
    std::vector<f64> nextErrors(w * 3, 0.0);

    constexpr f64 coeff_7_16 {7.0 / 16.0};
    constexpr f64 coeff_3_16 {3.0 / 16.0};
    constexpr f64 coeff_5_16 {5.0 / 16.0};
    constexpr f64 coeff_1_16 {1.0 / 16.0};

    for (i32 y {0}; y < h; ++y) {
        for (i32 x {0}; x < w; ++x) {
            isize const idx {(y * w + x)};
            color const original {img.get_pixel(idx)};

            f64 const r {std::clamp(static_cast<f64>(original.R) + currErrors[(x * 3) + 0], 0.0, 255.0)};
            f64 const g {std::clamp(static_cast<f64>(original.G) + currErrors[(x * 3) + 1], 0.0, 255.0)};
            f64 const b {std::clamp(static_cast<f64>(original.B) + currErrors[(x * 3) + 2], 0.0, 255.0)};

            retValue[idx] = find_nearest(r, g, b);

            color const quantized {get_color(retValue[idx])};

            f64 const errR {r - static_cast<f64>(quantized.R)};
            f64 const errG {g - static_cast<f64>(quantized.G)};
            f64 const errB {b - static_cast<f64>(quantized.B)};

            if (x + 1 < w) {
                currErrors[((x + 1) * 3) + 0] += errR * coeff_7_16;
                currErrors[((x + 1) * 3) + 1] += errG * coeff_7_16;
                currErrors[((x + 1) * 3) + 2] += errB * coeff_7_16;
            }

            if (y + 1 < h) {
                if (x > 0) {
                    nextErrors[((x - 1) * 3) + 0] += errR * coeff_3_16;
                    nextErrors[((x - 1) * 3) + 1] += errG * coeff_3_16;
                    nextErrors[((x - 1) * 3) + 2] += errB * coeff_3_16;
                }
                nextErrors[(x * 3) + 0] += errR * coeff_5_16;
                nextErrors[(x * 3) + 1] += errG * coeff_5_16;
                nextErrors[(x * 3) + 2] += errB * coeff_5_16;
                if (x + 1 < w) {
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
