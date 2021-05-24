// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

////////////////////////////////////////////////////////////
//based on Transform.cpp from:
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

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <tcob/core/data/Transform.hpp>

namespace tcob {
////////////////////////////////////////////////////////////
const Transform Transform::Identity;

auto Transform::matrix3() const -> const mat3&
{
    return _matrix;
}

void Transform::to_identity()
{
    // Identity matrix
    _matrix = {
        1.f, 0.f, 0.f,
        0.f, 1.f, 0.f,
        0.f, 0.f, 1.f
    };
}

////////////////////////////////////////////////////////////
auto Transform::inverse() const -> Transform
{
    // 1 - 1 3 - 2 4 - 3 5 - 4 7 - 5 12 - 6 13 - 7 15 - 8
    // Compute the determinant
    const f32 det { _matrix[0] * (_matrix[8] * _matrix[4] - _matrix[5] * _matrix[7]) - _matrix[1] * (_matrix[8] * _matrix[3] - _matrix[5] * _matrix[6]) + _matrix[2] * (_matrix[7] * _matrix[3] - _matrix[4] * _matrix[6]) };

    // Compute the inverse if the determinant is not zero
    // (don't use an epsilon because the determinant may *really* be tiny)
    if (det != 0.f) {
        return { (_matrix[8] * _matrix[4] - _matrix[5] * _matrix[7]) / det,
            -(_matrix[8] * _matrix[3] - _matrix[5] * _matrix[6]) / det,
            (_matrix[7] * _matrix[3] - _matrix[4] * _matrix[6]) / det,
            -(_matrix[8] * _matrix[1] - _matrix[2] * _matrix[7]) / det,
            (_matrix[8] * _matrix[0] - _matrix[2] * _matrix[6]) / det,
            -(_matrix[7] * _matrix[0] - _matrix[1] * _matrix[6]) / det,
            (_matrix[5] * _matrix[1] - _matrix[2] * _matrix[4]) / det,
            -(_matrix[5] * _matrix[0] - _matrix[2] * _matrix[3]) / det,
            (_matrix[4] * _matrix[0] - _matrix[1] * _matrix[3]) / det };
    } else {
        return Identity;
    }
}

} // namespace sf