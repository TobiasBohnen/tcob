// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <ostream>

#include "tcob/core/AngleUnits.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Size.hpp"

namespace tcob {
////////////////////////////////////////////////////////////

class [[nodiscard]] TCOB_API transform final {
public:
    constexpr transform();
    constexpr transform(f32 a00, f32 a01, f32 a02,
                        f32 a10, f32 a11, f32 a12,
                        f32 a20, f32 a21, f32 a22);

    mat3 Matrix {};

    auto constexpr as_matrix4 [[nodiscard]] () const -> mat4;

    auto constexpr as_inverted() const -> transform;

    auto constexpr is_translate_only() const -> bool;

    void constexpr to_identity();

    void constexpr translate(point_f offset);

    void rotate(degree_f angle);
    void rotate_at(degree_f angle, point_f center);

    void constexpr scale(size_f factors);
    void constexpr scale_at(size_f factors, point_f center);

    void skew(std::pair<degree_f, degree_f> const& skew);
    void skew_at(std::pair<degree_f, degree_f> const& skew, point_f center);

    void constexpr combine(transform const& xform);

    auto constexpr transform_point(point_f point) const -> point_f;

    static transform const Identity;
};

auto constexpr operator==(transform const& left, transform const& right) -> bool;

auto constexpr operator*(transform const& left, point_f right) -> point_f;

auto constexpr operator*(transform const& left, transform const& right) -> transform;

auto constexpr operator*=(transform& left, transform const& right) -> transform&;

inline auto operator<<(std::ostream& os, transform const& m) -> std::ostream&;

}

#include "Transform.inl"
