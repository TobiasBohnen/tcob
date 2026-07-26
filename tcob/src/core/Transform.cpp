// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/core/Transform.hpp"

#include <utility>

#include "tcob/core/AngleUnits.hpp"
#include "tcob/core/Point.hpp"

namespace tcob {
////////////////////////////////////////////////////////////

transform const transform::Identity;

void transform::rotate(radian_f angle)
{
    f32 const cos {angle.cos()};
    f32 const sin {angle.sin()};

    f32 const a0 {Matrix[0]}, a1 {Matrix[1]}, a2 {Matrix[2]};
    f32 const a3 {Matrix[3]}, a4 {Matrix[4]}, a5 {Matrix[5]};

    Matrix[0] = (a0 * cos) + (a3 * sin);
    Matrix[1] = (a1 * cos) + (a4 * sin);
    Matrix[2] = (a2 * cos) + (a5 * sin);
    Matrix[3] = (a0 * -sin) + (a3 * cos);
    Matrix[4] = (a1 * -sin) + (a4 * cos);
    Matrix[5] = (a2 * -sin) + (a5 * cos);
}

void transform::rotate_at(radian_f angle, point_f center)
{
    f32 const cos {angle.cos()};
    f32 const sin {angle.sin()};

    f32 const x1 {((center.X * (1 - cos)) + (center.Y * sin))};
    f32 const y1 {((center.Y * (1 - cos)) - (center.X * sin))};

    f32 const a0 {Matrix[0]}, a1 {Matrix[1]}, a2 {Matrix[2]};
    f32 const a3 {Matrix[3]}, a4 {Matrix[4]}, a5 {Matrix[5]};
    f32 const a6 {Matrix[6]}, a7 {Matrix[7]}, a8 {Matrix[8]};

    Matrix[0] = (a0 * cos) + (a3 * sin);
    Matrix[1] = (a1 * cos) + (a4 * sin);
    Matrix[2] = (a2 * cos) + (a5 * sin);
    Matrix[3] = (a0 * -sin) + (a3 * cos);
    Matrix[4] = (a1 * -sin) + (a4 * cos);
    Matrix[5] = (a2 * -sin) + (a5 * cos);
    Matrix[6] = (a0 * x1) + (a3 * y1) + a6;
    Matrix[7] = (a1 * x1) + (a4 * y1) + a7;
    Matrix[8] = (a2 * x1) + (a5 * y1) + a8;
}

void transform::skew(std::pair<radian_f, radian_f> skew)
{
    f32 const skewX {skew.first.tan()};
    f32 const skewY {skew.second.tan()};

    f32 const a0 {Matrix[0]}, a1 {Matrix[1]}, a2 {Matrix[2]};
    f32 const a3 {Matrix[3]}, a4 {Matrix[4]}, a5 {Matrix[5]};

    Matrix[0] = a0 + (a3 * skewY);
    Matrix[1] = a1 + (a4 * skewY);
    Matrix[2] = a2 + (a5 * skewY);
    Matrix[3] = (a0 * skewX) + a3;
    Matrix[4] = (a1 * skewX) + a4;
    Matrix[5] = (a2 * skewX) + a5;
}

void transform::skew_at(std::pair<radian_f, radian_f> skew, point_f center)
{
    f32 const skewX {skew.first.tan()};
    f32 const skewY {skew.second.tan()};

    f32 const x1 {(center.X * -skewX)};
    f32 const y1 {(center.Y * -skewY)};

    f32 const a0 {Matrix[0]}, a1 {Matrix[1]}, a2 {Matrix[2]};
    f32 const a3 {Matrix[3]}, a4 {Matrix[4]}, a5 {Matrix[5]};
    f32 const a6 {Matrix[6]}, a7 {Matrix[7]}, a8 {Matrix[8]};

    Matrix[0] = a0 + (a3 * skewY);
    Matrix[1] = a1 + (a4 * skewY);
    Matrix[2] = a2 + (a5 * skewY);
    Matrix[3] = (a0 * skewX) + a3;
    Matrix[4] = (a1 * skewX) + a4;
    Matrix[5] = (a2 * skewX) + a5;
    Matrix[6] = (a0 * x1) + (a3 * y1) + a6;
    Matrix[7] = (a1 * x1) + (a4 * y1) + a7;
    Matrix[8] = (a2 * x1) + (a5 * y1) + a8;
}

}
