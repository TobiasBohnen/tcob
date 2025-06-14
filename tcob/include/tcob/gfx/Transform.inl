// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Transform.hpp"

#include <ostream>

#include "tcob/core/Point.hpp"
#include "tcob/core/Size.hpp"

namespace tcob::gfx {

auto constexpr operator==(transform const& left, transform const& right) -> bool
{
    return left.Matrix == right.Matrix;
}

auto constexpr operator*(transform const& left, point_f right) -> point_f
{
    return left.transform_point(right);
}

auto constexpr operator*(transform const& left, transform const& right) -> transform
{
    transform retValue {left};
    retValue.combine(right);
    return retValue;
}

auto constexpr operator*=(transform& left, transform const& right) -> transform&
{
    left.combine(right);
    return left;
}

constexpr transform::transform()
    : Matrix {1.0f, 0.0f, 0.0f,
              0.0f, 1.0f, 0.0f,
              0.0f, 0.0f, 1.0f}
{
}

constexpr transform::transform(f32 a00, f32 a01, f32 a02,
                               f32 a10, f32 a11, f32 a12,
                               f32 a20, f32 a21, f32 a22)
    : Matrix {a00, a10, a20,
              a01, a11, a21,
              a02, a12, a22}
{
}

auto constexpr transform::as_matrix4() const -> mat4
{
    f32 const* a {Matrix.data()};
    return {a[0], a[1], 0.0f, a[2],
            a[3], a[4], 0.0f, a[5],
            0.0f, 0.0f, 1.0f, 0.0f,
            a[6], a[7], 0.0f, a[8]};
}

auto constexpr transform::as_inverted() const -> transform
{
    f32 const det {(Matrix[0] * (Matrix[8] * Matrix[4] - Matrix[5] * Matrix[7]))
                   - (Matrix[1] * (Matrix[8] * Matrix[3] - Matrix[5] * Matrix[6]))
                   + (Matrix[2] * (Matrix[7] * Matrix[3] - Matrix[4] * Matrix[6]))};

    if (det != 0.0f) {
        return {((Matrix[8] * Matrix[4]) - (Matrix[5] * Matrix[7])) / det,
                -((Matrix[8] * Matrix[3]) - (Matrix[5] * Matrix[6])) / det,
                ((Matrix[7] * Matrix[3]) - (Matrix[6] * Matrix[4])) / det,
                -((Matrix[8] * Matrix[1]) - (Matrix[2] * Matrix[7])) / det,
                ((Matrix[8] * Matrix[0]) - (Matrix[2] * Matrix[6])) / det,
                -((Matrix[7] * Matrix[0]) - (Matrix[1] * Matrix[6])) / det,
                ((Matrix[5] * Matrix[1]) - (Matrix[4] * Matrix[2])) / det,
                -((Matrix[5] * Matrix[0]) - (Matrix[2] * Matrix[3])) / det,
                ((Matrix[4] * Matrix[0]) - (Matrix[1] * Matrix[3])) / det};
    }

    return Identity;
}

auto constexpr transform::is_translate_only() const -> bool
{
    return Matrix[0] == 1.0f && Matrix[1] == 0.0f && Matrix[2] == 0.0f && Matrix[3] == 0.0f && Matrix[4] == 1.0f && Matrix[5] == 0.0f;
}

void constexpr transform::to_identity()
{
    Matrix[0] = 1.0f;
    Matrix[1] = 0.0f;
    Matrix[2] = 0.0f;

    Matrix[3] = 0.0f;
    Matrix[4] = 1.0f;
    Matrix[5] = 0.0f;

    Matrix[6] = 0.0f;
    Matrix[7] = 0.0f;
    Matrix[8] = 1.0f;
}

auto constexpr transform::transform_point(point_f point) const -> point_f
{
    f32 const* a {Matrix.data()};
    return {(a[0] * point.X) + (a[3] * point.Y) + a[6],
            (a[1] * point.X) + (a[4] * point.Y) + a[7]};
}

void constexpr transform::translate(point_f offset)
{
    Matrix[6] += Matrix[0] * offset.X + Matrix[3] * offset.Y;
    Matrix[7] += Matrix[1] * offset.X + Matrix[4] * offset.Y;
    Matrix[8] += Matrix[2] * offset.X + Matrix[5] * offset.Y;
}

void constexpr transform::scale(size_f factors)
{
    Matrix[0] *= factors.Width;
    Matrix[1] *= factors.Width;
    Matrix[2] *= factors.Width;
    Matrix[3] *= factors.Height;
    Matrix[4] *= factors.Height;
    Matrix[5] *= factors.Height;
}

void constexpr transform::scale_at(size_f factors, point_f center)
{
    f32 const x1 {(center.X * (1 - factors.Width))};
    f32 const y1 {(center.Y * (1 - factors.Height))};

    Matrix[6] += Matrix[0] * x1 + Matrix[3] * y1;
    Matrix[7] += Matrix[1] * x1 + Matrix[4] * y1;
    Matrix[8] += Matrix[2] * x1 + Matrix[5] * y1;
    Matrix[0] *= factors.Width;
    Matrix[1] *= factors.Width;
    Matrix[2] *= factors.Width;
    Matrix[3] *= factors.Height;
    Matrix[4] *= factors.Height;
    Matrix[5] *= factors.Height;
}

void constexpr transform::combine(transform const& xform)
{
    f32 const* a {Matrix.data()};
    f32 const* b {xform.Matrix.data()};

    Matrix = {(a[0] * b[0]) + (a[3] * b[1]) + (a[6] * b[2]), (a[1] * b[0]) + (a[4] * b[1]) + (a[7] * b[2]), (a[2] * b[0]) + (a[5] * b[1]) + (a[8] * b[2]),
              (a[0] * b[3]) + (a[3] * b[4]) + (a[6] * b[5]), (a[1] * b[3]) + (a[4] * b[4]) + (a[7] * b[5]), (a[2] * b[3]) + (a[5] * b[4]) + (a[8] * b[5]),
              (a[0] * b[6]) + (a[3] * b[7]) + (a[6] * b[8]), (a[1] * b[6]) + (a[4] * b[7]) + (a[7] * b[8]), (a[2] * b[6]) + (a[5] * b[7]) + (a[8] * b[8])};
}

inline auto operator<<(std::ostream& os, transform const& m) -> std::ostream&
{
    os << "[" << m.Matrix[0] << "," << m.Matrix[1] << "," << m.Matrix[2] << ","
       << m.Matrix[3] << "," << m.Matrix[4] << "," << m.Matrix[5] << ","
       << m.Matrix[6] << "," << m.Matrix[7] << "," << m.Matrix[8] << "]";
    return os;
}

}
