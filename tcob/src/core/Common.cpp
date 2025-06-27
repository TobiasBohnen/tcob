// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/core/Common.hpp"

#include <cmath>
#include <cstdlib>

namespace tcob::helper {

auto round_to_multiple(i32 num, i32 step) -> i32
{
    if (step <= 1) { return num; }

    i32 const rem {std::abs(num) % step};
    if (rem == 0) { return num; }

    i32 const adjustment {rem > step / 2 ? step - rem : -rem};
    return num < 0 ? num - adjustment : num + adjustment;
}

auto round_to_multiple(f32 num, f32 step) -> f32
{
    if (step <= 0.0f) { return num; }

    f32 const rem {std::abs(std::fmod(num, step))};
    if (rem == 0.0f) { return num; }

    f32 const adjustment {rem > step / 2 ? step - rem : -rem};
    return num < 0.0f ? num - adjustment : num + adjustment;
}

auto round_up_to_multiple(i32 num, i32 step) -> i32
{
    if (step <= 1) { return num; }

    i32 const rem {std::abs(num) % step};
    if (rem == 0) { return num; }

    return num < 0
        ? -(std::abs(num) - rem)
        : num + step - rem;
}

auto round_down_to_multiple(i32 num, i32 step) -> i32
{
    if (step <= 1) { return num; }

    i32 const rem {std::abs(num) % step};
    if (rem == 0) { return num; }

    return num < 0
        ? -(std::abs(num) + rem)
        : num - rem;
}

auto get_bits(u32 i, i32 offset, i32 count) -> u32
{
    return static_cast<u32>(i >> offset) & ((1 << count) - 1);
}

auto hash_combine(usize h1, usize h2) -> usize
{
    // from Boost
    return h1 ^= h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2);
}

}

////////////////////////////////////////////////////////////
