// Copyright (c) 2024 Tobias Bohnen
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

class TCOB_API filter_base {
public:
    filter_base()          = default;
    virtual ~filter_base() = default;

    auto virtual operator()(image const& img) const -> image = 0;
};

////////////////////////////////////////////////////////////

template <i32 Width, i32 Height>
class convolution_filter : public filter_base {
    static constexpr i32 Size {Width * Height};

public:
    bool IncludeAlpha {false};

    auto operator()(image const& img) const -> image override;

protected:
    auto virtual get_factor() const -> f64                   = 0;
    auto virtual get_offset() const -> u8                    = 0;
    auto virtual get_matrix() const -> std::array<i32, Size> = 0;
};

////////////////////////////////////////////////////////////

class TCOB_API blur_filter final : public convolution_filter<5, 5> {
protected:
    auto get_factor() const -> f64 override;
    auto get_offset() const -> u8 override;
    auto get_matrix() const -> std::array<i32, 25> override;
};

////////////////////////////////////////////////////////////

class TCOB_API edge_detect_filter final : public convolution_filter<3, 3> {
protected:
    auto get_factor() const -> f64 override;
    auto get_offset() const -> u8 override;
    auto get_matrix() const -> std::array<i32, 9> override;
};

////////////////////////////////////////////////////////////

class TCOB_API edge_enhance_filter final : public convolution_filter<3, 3> {
protected:
    auto get_factor() const -> f64 override;
    auto get_offset() const -> u8 override;
    auto get_matrix() const -> std::array<i32, 9> override;
};

////////////////////////////////////////////////////////////

class TCOB_API emboss_filter final : public convolution_filter<3, 3> {
protected:
    auto get_factor() const -> f64 override;
    auto get_offset() const -> u8 override;
    auto get_matrix() const -> std::array<i32, 9> override;
};

////////////////////////////////////////////////////////////

class TCOB_API motion_blur_filter final : public convolution_filter<9, 9> {
protected:
    auto get_factor() const -> f64 override;
    auto get_offset() const -> u8 override;
    auto get_matrix() const -> std::array<i32, 81> override;
};

////////////////////////////////////////////////////////////

class TCOB_API sharpen_filter final : public convolution_filter<5, 5> {
protected:
    auto get_factor() const -> f64 override;
    auto get_offset() const -> u8 override;
    auto get_matrix() const -> std::array<i32, 25> override;
};

////////////////////////////////////////////////////////////

class TCOB_API grayscale_filter final : public filter_base {
public:
    f32 RedFactor {0.299f};
    f32 GreenFactor {0.587f};
    f32 BlueFactor {0.114f};

    auto operator()(image const& img) const -> image override;
};

////////////////////////////////////////////////////////////

class TCOB_API resize_nearest_neighbor final : public filter_base {
public:
    size_i NewSize {};

    auto operator()(image const& img) const -> image override;
};

////////////////////////////////////////////////////////////

class TCOB_API resize_bilinear final : public filter_base {
public:
    size_i NewSize {};

    auto operator()(image const& img) const -> image override;
};

}

#include "ImageFilters.inl"
