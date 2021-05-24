// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <array>
#include <limits>
#include <random>

#include <tcob/core/Helper.hpp>

namespace tcob {

//based on: https://xoroshiro.di.unimi.it/xoroshiro128plus.c
class Xoroshiro128Plus final {
public:
    using result_type = u64;

    Xoroshiro128Plus();
    explicit Xoroshiro128Plus(u64 seed);

    auto operator()() -> u64
    {
        const u64 s0 = _state[0];
        u64 s1 = _state[1];
        const u64 result = s0 + s1;

        s1 ^= s0;
        _state[0] = rotl(s0, 24) ^ s1 ^ (s1 << 16); // a, b
        _state[1] = rotl(s1, 37); // c

        return result;
    }

    static constexpr auto min() -> u64
    {
        return 0;
    }

    static constexpr auto max() -> u64
    {
        return std::numeric_limits<u64>::max();
    }

private:
    std::array<u64, 2> _state { 0, 0xFBADA55C0DEBA5Eull };

    static constexpr u64 rotl(u64 x, u32 k)
    {
        return (x << k) | (x >> (64 - k));
    }
};

////////////////////////////////////////////////////////////

class Random final {
public:
    Random();
    explicit Random(u64 seed);

    template <Integral T>
    auto operator()(T min, T max) -> T
    {
        if (min == max) {
            return min;
        }

        return static_cast<T>(min + (_gen() % (max - min + 1)));
    }

    template <FloatingPoint T>
    auto operator()(T min, T max) -> T
    {
        if (min == max) {

            return min;
        }

        T fac { _gen() / static_cast<T>(Xoroshiro128Plus::max()) };
        return static_cast<T>(min + fac * (max - min));
    }

private:
    Xoroshiro128Plus _gen {};
};

}