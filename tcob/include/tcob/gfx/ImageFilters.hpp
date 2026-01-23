// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <array>
#include <memory>
#include <vector>

#include "tcob/core/Color.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/core/spatial/KDTree.hpp"
#include "tcob/gfx/Image.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

class TCOB_API filter_base {
public:
    filter_base()          = default;
    virtual ~filter_base() = default;

    virtual auto operator()(image const& img) -> image = 0;
};

////////////////////////////////////////////////////////////

template <i32 Width, i32 Height>
class convolution_filter : public filter_base {
    static constexpr i32 Size {Width * Height};

public:
    bool IncludeAlpha {false};

    auto operator()(image const& img) -> image override;

protected:
    virtual auto factor() const -> f64                   = 0;
    virtual auto offset() const -> u8                    = 0;
    virtual auto matrix() const -> std::array<i32, Size> = 0;
};

////////////////////////////////////////////////////////////

class TCOB_API blur_filter final : public convolution_filter<5, 5> {
protected:
    auto factor() const -> f64 override;
    auto offset() const -> u8 override;
    auto matrix() const -> std::array<i32, 25> override;
};

////////////////////////////////////////////////////////////

class TCOB_API edge_detect_filter final : public convolution_filter<3, 3> {
protected:
    auto factor() const -> f64 override;
    auto offset() const -> u8 override;
    auto matrix() const -> std::array<i32, 9> override;
};

////////////////////////////////////////////////////////////

class TCOB_API edge_enhance_filter final : public convolution_filter<3, 3> {
protected:
    auto factor() const -> f64 override;
    auto offset() const -> u8 override;
    auto matrix() const -> std::array<i32, 9> override;
};

////////////////////////////////////////////////////////////

class TCOB_API emboss_filter final : public convolution_filter<3, 3> {
protected:
    auto factor() const -> f64 override;
    auto offset() const -> u8 override;
    auto matrix() const -> std::array<i32, 9> override;
};

////////////////////////////////////////////////////////////

class TCOB_API motion_blur_filter final : public convolution_filter<9, 9> {
protected:
    auto factor() const -> f64 override;
    auto offset() const -> u8 override;
    auto matrix() const -> std::array<i32, 81> override;
};

////////////////////////////////////////////////////////////

class TCOB_API sharpen_filter final : public convolution_filter<5, 5> {
protected:
    auto factor() const -> f64 override;
    auto offset() const -> u8 override;
    auto matrix() const -> std::array<i32, 25> override;
};

////////////////////////////////////////////////////////////

class TCOB_API grayscale_filter final : public filter_base {
public:
    f32 RedFactor {0.299f};
    f32 GreenFactor {0.587f};
    f32 BlueFactor {0.114f};

    auto operator()(image const& img) -> image override;
};

////////////////////////////////////////////////////////////

class TCOB_API resize_nearest_neighbor : public filter_base {
public:
    size_i NewSize {};

    auto operator()(image const& img) -> image override;
};

////////////////////////////////////////////////////////////

class TCOB_API resize_bilinear final : public filter_base {
public:
    size_i NewSize {};

    auto operator()(image const& img) -> image override;
};

////////////////////////////////////////////////////////////

class TCOB_API remove_alpha final : public filter_base {
public:
    auto operator()(image const& img) -> image override;
};

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

class TCOB_API octree_quantizer final : public filter_base {
public:
    explicit octree_quantizer(i32 maxColors);

    auto operator()(image const& img) -> image override;

    auto static GetPalette(image const& img, i32 maxColors) -> std::vector<color>;

private:
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
    auto count_leaves(node* n) -> i32;

    i32                                            _maxColors;
    i32                                            _leafCount {0};
    std::unique_ptr<node>                          _root;
    std::array<std::vector<node*>, MAX_TREE_DEPTH> _reducibleNodes;
};

////////////////////////////////////////////////////////////

class TCOB_API neuquant final : public filter_base {
public:
    explicit neuquant(i32 maxColors);

    auto operator()(image const& img) -> image override;

    auto static GetPalette(image const& img, i32 maxColors) -> std::vector<color>;

private:
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

class TCOB_API ditherer_base : public filter_base {
public:
    ditherer_base(std::vector<color> palette);
    virtual ~ditherer_base() = default;

    auto         operator()(image const& img) -> image override;
    virtual auto to_indexed(image const& img) -> std::vector<u32> = 0;

protected:
    auto find_nearest(f64 r, f64 g, f64 b) const -> u32;
    auto get_color(u32 idx) const -> color;

    std::vector<color> _palette;
    struct color_node {
        std::array<f32, 3> Position;
        u32                ID;

        auto get_dimensions() const -> std::array<f32, 3> { return Position; }
        auto operator==(color_node const& other) const -> bool { return ID == other.ID; }
    };

    kd_tree<color_node, 3> _tree;
};

////////////////////////////////////////////////////////////

class TCOB_API ordered_dither final : public ditherer_base {
public:
    explicit ordered_dither(std::vector<color> palette);

    auto to_indexed(image const& img) -> std::vector<u32> override;

private:
    auto get_threshold(i32 x, i32 y) const -> f64;

    static constexpr std::array<f64, 8 * 8> bayer8x8 {
        {0.000000, 0.500000, 0.125000, 0.625000, 0.031250, 0.531250, 0.156250, 0.656250,
         0.750000, 0.250000, 0.875000, 0.375000, 0.781250, 0.281250, 0.906250, 0.406250,
         0.187500, 0.687500, 0.062500, 0.562500, 0.218750, 0.718750, 0.093750, 0.593750,
         0.937500, 0.437500, 0.812500, 0.312500, 0.968750, 0.468750, 0.843750, 0.343750,
         0.046875, 0.546875, 0.171875, 0.671875, 0.015625, 0.515625, 0.140625, 0.640625,
         0.796875, 0.296875, 0.921875, 0.421875, 0.765625, 0.265625, 0.890625, 0.390625,
         0.234375, 0.734375, 0.109375, 0.609375, 0.203125, 0.703125, 0.078125, 0.578125,
         0.984375, 0.484375, 0.859375, 0.359375, 0.953125, 0.453125, 0.828125, 0.328125}};
};

////////////////////////////////////////////////////////////

class TCOB_API atkinson_dither final : public ditherer_base {
public:
    explicit atkinson_dither(std::vector<color> palette);

    auto to_indexed(image const& img) -> std::vector<u32> override;
};

////////////////////////////////////////////////////////////

class TCOB_API floyd_steinberg_dither final : public ditherer_base {
public:
    explicit floyd_steinberg_dither(std::vector<color> palette);

    auto to_indexed(image const& img) -> std::vector<u32> override;
};

};

#include "ImageFilters.inl"
