// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/Transform.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////
transform const transform::Identity;

void transform::rotate(radian_f angle)
{
    f32 const cos {angle.cos()};
    f32 const sin {angle.sin()};

    f32 const* a {Matrix.data()};
    Matrix = {(a[0] * cos) + (a[3] * sin),
              (a[1] * cos) + (a[4] * sin),
              (a[2] * cos) + (a[5] * sin),
              (a[0] * -sin) + (a[3] * cos),
              (a[1] * -sin) + (a[4] * cos),
              (a[2] * -sin) + (a[5] * cos),
              a[6], a[7], a[8]};
}

void transform::rotate_at(radian_f angle, point_f center)
{
    f32 const cos {angle.cos()};
    f32 const sin {angle.sin()};

    f32 const x1 {((center.X * (1 - cos)) + (center.Y * sin))};
    f32 const y1 {((center.Y * (1 - cos)) - (center.X * sin))};

    f32 const* a {Matrix.data()};
    Matrix = {(a[0] * cos) + (a[3] * sin),
              (a[1] * cos) + (a[4] * sin),
              (a[2] * cos) + (a[5] * sin),
              (a[0] * -sin) + (a[3] * cos),
              (a[1] * -sin) + (a[4] * cos),
              (a[2] * -sin) + (a[5] * cos),

              (a[0] * x1) + (a[3] * y1) + a[6],
              (a[1] * x1) + (a[4] * y1) + a[7],
              (a[2] * x1) + (a[5] * y1) + a[8]};
}

void transform::skew(std::pair<radian_f, radian_f> const& skew)
{
    f32 const skewX {skew.first.tan()};
    f32 const skewY {skew.second.tan()};

    f32 const* a {Matrix.data()};
    Matrix = {a[0] + (a[3] * skewY),
              a[1] + (a[4] * skewY),
              a[2] + (a[5] * skewY),
              (a[0] * skewX) + a[3],
              (a[1] * skewX) + a[4],
              (a[2] * skewX) + a[5],
              a[6], a[7], a[8]};
}

void transform::skew_at(std::pair<radian_f, radian_f> const& skew, point_f center)
{
    f32 const skewX {skew.first.tan()};
    f32 const skewY {skew.second.tan()};

    f32 const x1 {(center.X * -skewX)};
    f32 const y1 {(center.Y * -skewY)};

    f32 const* a {Matrix.data()};
    Matrix = {a[0] + (a[3] * skewY),
              a[1] + (a[4] * skewY),
              a[2] + (a[5] * skewY),
              (a[0] * skewX) + a[3],
              (a[1] * skewX) + a[4],
              (a[2] * skewX) + a[5],
              (a[0] * x1) + (a[3] * y1) + a[6],
              (a[1] * x1) + (a[4] * y1) + a[7],
              (a[2] * x1) + (a[5] * y1) + a[8]};
}

}
