// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <array>

#include "tcob/core/Size.hpp"
#include "tcob/gfx/Image.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

template <i32 Width, i32 Height>
class convolution_filter {
    static constexpr i32 Size {Width * Height};

public:
    virtual ~convolution_filter() = default;

    bool IncludeAlpha {false};

    auto operator()(image const& img) const -> image;

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

class TCOB_API grayscale_filter final {
public:
    f32 RedFactor {0.299f};
    f32 GreenFactor {0.587f};
    f32 BlueFactor {0.114f};

    auto operator()(image const& img) const -> image;
};

////////////////////////////////////////////////////////////

class TCOB_API nearest_neighbor_resizer {
public:
    size_i NewSize {};

    auto operator()(image const& img) const -> image;
};

////////////////////////////////////////////////////////////

class TCOB_API bilinear_resizer final {
public:
    size_i NewSize {};

    auto operator()(image const& img) const -> image;
};

////////////////////////////////////////////////////////////

class TCOB_API alpha_remover final {
public:
    auto operator()(image const& img) const -> image;
};

};

#include "ImageFilters.inl"
