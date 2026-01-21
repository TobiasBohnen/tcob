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

class TCOB_API neuquant : public filter_base {
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

class ditherer_base : public filter_base {
public:
    virtual ~ditherer_base() = default;

    ditherer_base(std::vector<color> palette);

    auto         operator()(image const& img) -> image override;
    virtual auto to_indexed(image const& img) -> std::vector<u32> = 0;

protected:
    auto find_nearest(color c) const -> u32;
    auto get_color(u32 idx) const -> color;

    std::vector<color> _palette;
};

////////////////////////////////////////////////////////////

class TCOB_API ordered_dither : public ditherer_base {
public:
    explicit ordered_dither(std::vector<color> palette, i32 matrixSize = 8);

    auto to_indexed(image const& img) -> std::vector<u32> override;

private:
    auto get_threshold(i32 x, i32 y) const -> f64;

    void generate_bayer_matrix();

    std::vector<f64> _bayerMatrix;
    i32              _matrixSize;
};

////////////////////////////////////////////////////////////

class TCOB_API atkinson_dither : public ditherer_base {
public:
    explicit atkinson_dither(std::vector<color> palette);

    auto to_indexed(image const& img) -> std::vector<u32> override;
};

////////////////////////////////////////////////////////////

class TCOB_API floyd_steinberg_dither : public ditherer_base {
public:
    explicit floyd_steinberg_dither(std::vector<color> palette);

    auto to_indexed(image const& img) -> std::vector<u32> override;
};

};

#include "ImageFilters.inl"
