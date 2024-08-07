// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/core/Common.hpp"

#if defined(_MSC_VER) and !defined(__llvm__)
    #include <intrin.h>
    #define bswap16 _byteswap_ushort
    #define bswap32 _byteswap_ulong
    #define bswap64 _byteswap_uint64
#else
    #define bswap16 __builtin_bswap16
    #define bswap32 __builtin_bswap32
    #define bswap64 __builtin_bswap64
#endif

namespace tcob::helper {

auto byteswap(u16 val) -> u16
{
    return bswap16(val);
}

auto byteswap(u32 val) -> u32
{
    return bswap32(val);
}

auto byteswap(u64 val) -> u64
{
    return bswap64(val);
}

auto round_to_multiple(i32 num, i32 step) -> i32
{
    if (step <= 1) {
        return num;
    }

    i32 const rem {std::abs(num) % step};
    if (rem == 0) {
        return num;
    }

    i32 const adjustment {rem > step / 2 ? step - rem : -rem};
    return num < 0 ? num - adjustment : num + adjustment;
}

auto round_up_to_multiple(i32 num, i32 step) -> i32
{
    if (step <= 1) {
        return num;
    }

    i32 const rem {std::abs(num) % step};
    if (rem == 0) {
        return num;
    }

    return num < 0
        ? -(std::abs(num) - rem)
        : num + step - rem;
}

auto round_down_to_multiple(i32 num, i32 step) -> i32
{
    if (step <= 1) {
        return num;
    }

    i32 const rem {std::abs(num) % step};
    if (rem == 0) {
        return num;
    }

    return num < 0
        ? -(std::abs(num) + rem)
        : num - rem;
}

auto get_bits(i32 i, i32 offset, i32 count) -> u32
{
    return static_cast<u32>(i >> offset) & ((1 << count) - 1);
}

}

////////////////////////////////////////////////////////////
