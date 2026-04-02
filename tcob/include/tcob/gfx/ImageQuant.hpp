// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <array>
#include <memory>
#include <span>
#include <vector>

#include "tcob/core/Color.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/core/spatial/KDTree.hpp"
#include "tcob/gfx/Image.hpp"
#include "tcob/gfx/procgen/Noise.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

class TCOB_API octree_quant final {
public:
    auto static GetPalette(image const& img, i32 maxColors) -> std::vector<color>;

private:
    explicit octree_quant(i32 maxColors);

    static constexpr i32 MAX_TREE_DEPTH {8};

    struct node {
        bool                                 IsLeaf {false};
        bool                                 IsEnqueued {false};
        i32                                  PixelCount {0};
        i32                                  RedSum {0};
        i32                                  GreenSum {0};
        i32                                  BlueSum {0};
        std::array<std::unique_ptr<node>, 8> Children {};
        node*                                Parent {nullptr};
        i32                                  ChildCount {0};
        i32                                  IndexInParent {0};
    };

    void build_tree(image const& img);
    void build_palette(node const* n, std::vector<color>& pal) const;

    void insert_color(color c);
    void reduce();
    auto get_quantized_color(color c) const -> color;
    void merge_leaf_nodes(node* n, i32 level);

    i32                                            _maxColors;
    i32                                            _leafCount {0};
    std::unique_ptr<node>                          _root;
    std::array<std::vector<node*>, MAX_TREE_DEPTH> _reducibleNodes;
};

////////////////////////////////////////////////////////////

class TCOB_API neuquant final {
public:
    auto static GetPalette(image const& img, i32 maxColors) -> std::vector<color>;

private:
    explicit neuquant(i32 maxColors);

    struct neuron {
        f64 R {0};
        f64 G {0};
        f64 B {0};
    };

    void train(image const& img);
    auto find_bmu(color c) const -> i32;
    void alter_neighbor(i32 index, color c, f64 alpha);

    std::vector<neuron> _network;
    i32                 _maxColors {};
};

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

class TCOB_API ditherer_base {
public:
    explicit ditherer_base(std::vector<color> palette);
    virtual ~ditherer_base() = default;

    auto         operator()(image const& img) const -> image;
    virtual auto to_indexed(image const& img) const -> std::vector<u32> = 0;

protected:
    auto find_nearest(f64 r, f64 g, f64 b) const -> u32;
    auto get_color(u32 idx) const -> color;

    std::vector<color> _palette;
    struct color_node {
        std::array<f64, 3> Position;
        u32                ID;

        auto get_dimensions() const -> std::array<f64, 3> { return Position; }
        auto operator==(color_node const& other) const -> bool { return ID == other.ID; }
    };

    kd_tree<color_node, 3> _tree;
};

////////////////////////////////////////////////////////////

class TCOB_API nearest_neighbor_dither final : public ditherer_base {
public:
    using ditherer_base::ditherer_base;

    auto to_indexed(image const& img) const -> std::vector<u32> override;
};

////////////////////////////////////////////////////////////

enum class bayer_matrix : u8 {
    Bayer2x2,
    Bayer4x4,
    Bayer8x8
};

class TCOB_API bayer_dither final : public ditherer_base {
public:
    explicit bayer_dither(std::vector<color> palette, bayer_matrix matrix = bayer_matrix::Bayer8x8);

    auto to_indexed(image const& img) const -> std::vector<u32> override;

private:
    std::span<f32 const> _matrix;
    usize                _matrixSize;

    static constexpr std::array<f32, 2 * 2> Bayer2x2 {{0.000000f, 0.750000f,
                                                       0.500000f, 0.250000f}};

    static constexpr std::array<f32, 4 * 4> Bayer4x4 {{0.000000f, 0.500000f, 0.125000f, 0.625000f,
                                                       0.750000f, 0.250000f, 0.875000f, 0.375000f,
                                                       0.187500f, 0.687500f, 0.062500f, 0.562500f,
                                                       0.937500f, 0.437500f, 0.812500f, 0.312500f}};

    static constexpr std::array<f32, 8 * 8> Bayer8x8 {{0.000000f, 0.500000f, 0.125000f, 0.625000f, 0.031250f, 0.531250f, 0.156250f, 0.656250f,
                                                       0.750000f, 0.250000f, 0.875000f, 0.375000f, 0.781250f, 0.281250f, 0.906250f, 0.406250f,
                                                       0.187500f, 0.687500f, 0.062500f, 0.562500f, 0.218750f, 0.718750f, 0.093750f, 0.593750f,
                                                       0.937500f, 0.437500f, 0.812500f, 0.312500f, 0.968750f, 0.468750f, 0.843750f, 0.343750f,
                                                       0.046875f, 0.546875f, 0.171875f, 0.671875f, 0.015625f, 0.515625f, 0.140625f, 0.640625f,
                                                       0.796875f, 0.296875f, 0.921875f, 0.421875f, 0.765625f, 0.265625f, 0.890625f, 0.390625f,
                                                       0.234375f, 0.734375f, 0.109375f, 0.609375f, 0.203125f, 0.703125f, 0.078125f, 0.578125f,
                                                       0.984375f, 0.484375f, 0.859375f, 0.359375f, 0.953125f, 0.453125f, 0.828125f, 0.328125f}};
};

////////////////////////////////////////////////////////////

class TCOB_API value_noise_dither final : public ditherer_base {
public:
    explicit value_noise_dither(std::vector<color> palette, size_i gridSize, u64 seed = static_cast<u64>(clock::now().time_since_epoch().count()));

    auto to_indexed(image const& img) const -> std::vector<u32> override;

private:
    value_noise _noise;
};

////////////////////////////////////////////////////////////

class TCOB_API atkinson_dither final : public ditherer_base {
public:
    explicit atkinson_dither(std::vector<color> palette);

    auto to_indexed(image const& img) const -> std::vector<u32> override;
};

////////////////////////////////////////////////////////////

class TCOB_API floyd_steinberg_dither final : public ditherer_base {
public:
    explicit floyd_steinberg_dither(std::vector<color> palette);

    auto to_indexed(image const& img) const -> std::vector<u32> override;
};

};
