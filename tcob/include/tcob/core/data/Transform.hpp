// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

////////////////////////////////////////////////////////////
// based on Transform.hpp from:
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2019 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <tcob/core/data/Point.hpp>
#include <tcob/core/data/Size.hpp>
#include <tcob/tcob_config.hpp>
namespace tcob {
////////////////////////////////////////////////////////////
/// \brief Define a 3x3 transform matrix
///
////////////////////////////////////////////////////////////
class Transform final {
public:
    ////////////////////////////////////////////////////////////
    constexpr Transform()
        : _matrix {
            1.f, 0.f, 0.f,
            0.f, 1.f, 0.f,
            0.f, 0.f, 1.f
        }
    {
    }

    ////////////////////////////////////////////////////////////
    constexpr Transform(f32 a00, f32 a01, f32 a02,
        f32 a10, f32 a11, f32 a12,
        f32 a20, f32 a21, f32 a22)
        : _matrix {
            a00, a10, a20,
            a01, a11, a21,
            a02, a12, a22
        }
    {
    }

    ////////////////////////////////////////////////////////////
    constexpr auto matrix4() const -> mat4
    {
        const f32* a { _matrix.data() };
        return {
            a[0], a[1], 0.f, a[2],
            a[3], a[4], 0.f, a[5],
            0.f, 0.f, 1.f, 0.f,
            a[6], a[7], 0.f, a[8]
        };
    }

    auto matrix3() const -> const mat3&;

    void to_identity();

    auto inverse() const -> Transform;

    ////////////////////////////////////////////////////////////
    constexpr auto transform_point(const PointF& point) const -> PointF
    {
        const f32* a { _matrix.data() };
        return { a[0] * point.X + a[3] * point.Y + a[6],
            a[1] * point.X + a[4] * point.Y + a[7] };
    }

    ////////////////////////////////////////////////////////////
    constexpr void translate(const PointF& offset)
    {
        _matrix[6] += _matrix[0] * offset.X + _matrix[3] * offset.Y;
        _matrix[7] += _matrix[1] * offset.X + _matrix[4] * offset.Y;
        _matrix[8] += _matrix[2] * offset.X + _matrix[5] * offset.Y;
    }

    ////////////////////////////////////////////////////////////
    void rotate(f32 angle)
    {
        const f32 rad { angle * TAU_F / 360.0f };
        const f32 cos { std::cos(rad) };
        const f32 sin { std::sin(rad) };

        const f32* a { _matrix.data() };
        _matrix = {
            a[0] * cos + a[3] * sin,
            a[1] * cos + a[4] * sin,
            a[2] * cos + a[5] * sin,
            a[0] * -sin + a[3] * cos,
            a[1] * -sin + a[4] * cos,
            a[2] * -sin + a[5] * cos,
            a[6], a[7], a[8]
        };
    }

    ////////////////////////////////////////////////////////////
    void rotate_at(f32 angle, const PointF& center)
    {
        const f32 rad { angle * TAU_F / 360.0f };
        const f32 cos { std::cos(rad) };
        const f32 sin { std::sin(rad) };

        const f32 x1 { (center.X * (1 - cos) + center.Y * sin) };
        const f32 y1 { (center.Y * (1 - cos) - center.X * sin) };

        const f32* a { _matrix.data() };
        _matrix = {
            a[0] * cos + a[3] * sin,
            a[1] * cos + a[4] * sin,
            a[2] * cos + a[5] * sin,
            a[0] * -sin + a[3] * cos,
            a[1] * -sin + a[4] * cos,
            a[2] * -sin + a[5] * cos,

            a[0] * x1 + a[3] * y1 + a[6],
            a[1] * x1 + a[4] * y1 + a[7],
            a[2] * x1 + a[5] * y1 + a[8]
        };
    }

    ////////////////////////////////////////////////////////////
    constexpr void scale(const SizeF& factors)
    {
        _matrix[0] *= factors.Width;
        _matrix[1] *= factors.Width;
        _matrix[2] *= factors.Width;
        _matrix[3] *= factors.Height;
        _matrix[4] *= factors.Height;
        _matrix[5] *= factors.Height;
    }

    ////////////////////////////////////////////////////////////
    constexpr void scale_at(const SizeF& factors, const PointF& center)
    {
        const f32 x1 { (center.X * (1 - factors.Width)) };
        const f32 y1 { (center.Y * (1 - factors.Height)) };

        _matrix[6] += _matrix[0] * x1 + _matrix[3] * y1;
        _matrix[7] += _matrix[1] * x1 + _matrix[4] * y1;
        _matrix[8] += _matrix[2] * x1 + _matrix[5] * y1;
        _matrix[0] *= factors.Width;
        _matrix[1] *= factors.Width;
        _matrix[2] *= factors.Width;
        _matrix[3] *= factors.Height;
        _matrix[4] *= factors.Height;
        _matrix[5] *= factors.Height;
    }

    ////////////////////////////////////////////////////////////
    void skew(const PointF& skew)
    {
        const f32 skewX { std::tan(skew.X * TAU_F / 360.0f) };
        const f32 skewY { std::tan(skew.Y * TAU_F / 360.0f) };

        const f32* a { _matrix.data() };
        _matrix = {
            a[0] + a[3] * skewY,
            a[1] + a[4] * skewY,
            a[2] + a[5] * skewY,
            a[0] * skewX + a[3],
            a[1] * skewX + a[4],
            a[2] * skewX + a[5],
            a[6], a[7], a[8]
        };
    }

    ////////////////////////////////////////////////////////////
    void skew_at(const PointF& skew, const PointF& center)
    {
        const f32 skewX { std::tan(skew.X * TAU_F / 360.0f) };
        const f32 skewY { std::tan(skew.Y * TAU_F / 360.0f) };

        const f32 x1 { (center.X * -skewX) };
        const f32 y1 { (center.Y * -skewY) };

        const f32* a { _matrix.data() };
        _matrix = {
            a[0] + a[3] * skewY,
            a[1] + a[4] * skewY,
            a[2] + a[5] * skewY,
            a[0] * skewX + a[3],
            a[1] * skewX + a[4],
            a[2] * skewX + a[5],
            a[0] * x1 + a[3] * y1 + a[6],
            a[1] * x1 + a[4] * y1 + a[7],
            a[2] * x1 + a[5] * y1 + a[8]
        };
    }

    ////////////////////////////////////////////////////////////
    constexpr void combine(const Transform& transform)
    {
        const f32* a { _matrix.data() };
        const f32* b { transform._matrix.data() };

        _matrix = {
            a[0] * b[0] + a[3] * b[1] + a[6] * b[2], a[1] * b[0] + a[4] * b[1] + a[7] * b[2], a[2] * b[0] + a[5] * b[1] + a[8] * b[2],
            a[0] * b[3] + a[3] * b[4] + a[6] * b[5], a[1] * b[3] + a[4] * b[4] + a[7] * b[5], a[2] * b[3] + a[5] * b[4] + a[8] * b[5],
            a[0] * b[6] + a[3] * b[7] + a[6] * b[8], a[1] * b[6] + a[4] * b[7] + a[7] * b[8], a[2] * b[6] + a[5] * b[7] + a[8] * b[8]
        };
    }

    static const Transform Identity;

private:
    mat3 _matrix;
};

////////////////////////////////////////////////////////////
inline auto operator==(const Transform& left, const Transform& right) -> bool
{
    return left.matrix3() == right.matrix3();
}

////////////////////////////////////////////////////////////
inline auto operator!=(const Transform& left, const Transform& right) -> bool
{
    return !(left == right);
}

////////////////////////////////////////////////////////////
inline auto operator*(const Transform& left, const PointF& right) -> PointF
{
    return left.transform_point(right);
}

////////////////////////////////////////////////////////////
inline auto operator*(const Transform& left, const Transform& right) -> Transform
{
    Transform retValue { left };
    retValue.combine(right);
    return retValue;
}

////////////////////////////////////////////////////////////
inline auto operator*=(Transform& left, const Transform& right) -> Transform&
{
    left.combine(right);
    return left;
}

}