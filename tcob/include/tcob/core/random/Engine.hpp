// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <array>

#include "tcob/core/Concepts.hpp"

////////////////////////////////////////////////////////////
namespace tcob::random {

template <typename T>
concept RandomEngine =
    requires(T& rnd, typename T::state_type& state, typename T::seed_type seed) {
        typename T::state_type;
        typename T::seed_type;
        typename T::result_type;

        {
            rnd.operator()(state)
        } -> std::same_as<typename T::result_type>;

        {
            rnd.seed(state, seed)
        };
    };

////////////////////////////////////////////////////////////

class TCOB_API split_mix_32 final {
public:
    using state_type  = std::array<u32, 1>;
    using seed_type   = u32;
    using result_type = u32;

    auto operator()(state_type& state) -> result_type;
    void seed(state_type& state, seed_type seed) const;
};

////////////////////////////////////////////////////////////

// based on: https://xorshift.di.unimi.it/splitmix64.c
class TCOB_API split_mix_64 final {
public:
    using state_type  = std::array<u64, 1>;
    using seed_type   = u64;
    using result_type = u64;

    auto operator()(state_type& state) -> result_type;
    void seed(state_type& state, seed_type seed) const;
};

////////////////////////////////////////////////////////////

class TCOB_API game_rand final {
public:
    using state_type  = std::array<u32, 2>;
    using seed_type   = u32;
    using result_type = u32;

    auto operator()(state_type& state) -> result_type;
    void seed(state_type& state, seed_type seed) const;
};

////////////////////////////////////////////////////////////

class TCOB_API xorshift_64 final {
public:
    using state_type  = std::array<u64, 1>;
    using seed_type   = u64;
    using result_type = u64;

    auto operator()(state_type& state) -> result_type;
    void seed(state_type& state, seed_type seed) const;
};

////////////////////////////////////////////////////////////

class TCOB_API xorshift_64_star final {
public:
    using state_type  = std::array<u64, 1>;
    using seed_type   = u64;
    using result_type = u64;

    auto operator()(state_type& state) -> result_type;
    void seed(state_type& state, seed_type seed) const;
};

////////////////////////////////////////////////////////////

// based on: https://xoroshiro.di.unimi.it/xoroshiro128plus.c
class TCOB_API xoroshiro_128_plus final {
public:
    using state_type  = std::array<u64, 2>;
    using seed_type   = u64;
    using result_type = u64;

    auto operator()(state_type& state) -> result_type;
    void seed(state_type& state, seed_type seed) const;
};

////////////////////////////////////////////////////////////

// based on: https://xoroshiro.di.unimi.it/xoroshiro128plusplus.c
class TCOB_API xoroshiro_128_plus_plus final {
public:
    using state_type  = std::array<u64, 2>;
    using seed_type   = u64;
    using result_type = u64;

    auto operator()(state_type& state) -> result_type;
    void seed(state_type& state, seed_type seed) const;
};

////////////////////////////////////////////////////////////

// based on: https://xoroshiro.di.unimi.it/xoroshiro128starstar.c
class TCOB_API xoroshiro_128_star_star final {
public:
    using state_type  = std::array<u64, 2>;
    using seed_type   = u64;
    using result_type = u64;

    auto operator()(state_type& state) -> result_type;
    void seed(state_type& state, seed_type seed) const;
};

////////////////////////////////////////////////////////////

// based on: https://prng.di.unimi.it/xoshiro256plus.c
class TCOB_API xoshiro_256_plus final {
public:
    using state_type  = std::array<u64, 4>;
    using seed_type   = u64;
    using result_type = u64;

    auto operator()(state_type& state) -> result_type;
    void seed(state_type& state, seed_type seed) const;
};

////////////////////////////////////////////////////////////

// based on: https://prng.di.unimi.it/xoshiro256plusplus.c
class TCOB_API xoshiro_256_plus_plus final {
public:
    using state_type  = std::array<u64, 4>;
    using seed_type   = u64;
    using result_type = u64;

    auto operator()(state_type& state) -> result_type;
    void seed(state_type& state, seed_type seed) const;
};

////////////////////////////////////////////////////////////

// based on: https://prng.di.unimi.it/xoshiro256starstar.c
class TCOB_API xoshiro_256_star_star final {
public:
    using state_type  = std::array<u64, 4>;
    using seed_type   = u64;
    using result_type = u64;

    auto operator()(state_type& state) -> result_type;
    void seed(state_type& state, seed_type seed) const;
};

////////////////////////////////////////////////////////////

// based on: http://lomont.org/papers/2008/Lomont_PRNG_2008.pdf
class TCOB_API well_512_a final {
public:
    using state_type  = std::array<u32, 16>;
    using seed_type   = u32;
    using result_type = u32;

    auto operator()(state_type& state) -> result_type;
    void seed(state_type& state, seed_type seed) const;

private:
    usize _index {0};
};

}
