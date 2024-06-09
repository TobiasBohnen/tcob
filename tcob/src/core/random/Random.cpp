// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/core/random/Random.hpp"

namespace tcob::random {

auto static constexpr rotl(u64 x, u32 k) -> u64
{
    return (x << k) | (x >> (64 - k));
}

////////////////////////////////////////////////////////////

auto split_mix_32::operator()(state_type& state) -> result_type
{
    u32 z {state[0] += 0x9e3779b9};
    z = (z ^ (z >> 15)) * 0x85ebca6b;
    z = (z ^ (z >> 13)) * 0xc2b2ae35;
    return z ^ (z >> 16);
}

void split_mix_32::seed(state_type& state, seed_type seed) const
{
    state[0] = seed;
}

////////////////////////////////////////////////////////////

auto split_mix_64::operator()(state_type& state) -> result_type
{
    u64 z {state[0] += 0x9e3779b97f4a7c15ULL};
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
}

void split_mix_64::seed(state_type& state, seed_type seed) const
{
    state[0] = seed;
}

////////////////////////////////////////////////////////////

auto game_rand::operator()(state_type& state) -> result_type
{
    state[0] = (state[0] << 16) + (state[0] >> 16);
    state[0] += state[1];
    state[1] += state[0];
    return state[0];
}

void game_rand::seed(state_type& state, seed_type seed) const
{
    rng_split_mix_32 rnd {seed};
    state[0] = rnd.next();
    state[1] = rnd.next();
}

////////////////////////////////////////////////////////////

auto xorshift_64::operator()(state_type& state) -> result_type
{
    u64 x {state[0]};
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    return state[0] = x;
}

void xorshift_64::seed(state_type& state, seed_type seed) const
{
    state[0] = seed;
}

////////////////////////////////////////////////////////////

auto xorshift_64_star::operator()(state_type& state) -> result_type
{
    u64 x {state[0]};
    x ^= x >> 12; // a
    x ^= x << 25; // b
    x ^= x >> 27; // c
    state[0] = x;
    return x * 0x2545f4914f6cdd1dULL;
}

void xorshift_64_star::seed(state_type& state, seed_type seed) const
{
    state[0] = seed;
}

////////////////////////////////////////////////////////////

auto xoroshiro_128_plus::operator()(state_type& state) -> result_type
{
    u64 const s0 {state[0]};
    u64       s1 {state[1]};
    u64 const result {s0 + s1};

    s1 ^= s0;
    state[0] = rotl(s0, 24) ^ s1 ^ (s1 << 16); // a, b
    state[1] = rotl(s1, 37);                   // c

    return result;
}

void xoroshiro_128_plus::seed(state_type& state, seed_type seed) const
{
    rng_split_mix_64 rnd {seed};
    state[0] = rnd.next();
    state[1] = rnd.next();
}

////////////////////////////////////////////////////////////

auto xoroshiro_128_plus_plus::operator()(state_type& state) -> result_type
{
    u64 const s0 {state[0]};
    u64       s1 {state[1]};
    u64 const result {rotl(s0 + s1, 17) + s0};

    s1 ^= s0;
    state[0] = rotl(s0, 49) ^ s1 ^ (s1 << 21); // a, b
    state[1] = rotl(s1, 28);                   // c

    return result;
}

void xoroshiro_128_plus_plus::seed(state_type& state, seed_type seed) const
{
    rng_split_mix_64 rnd {seed};
    state[0] = rnd.next();
    state[1] = rnd.next();
}

////////////////////////////////////////////////////////////

auto xoroshiro_128_star_star::operator()(state_type& state) -> result_type
{
    u64 const s0 {state[0]};
    u64       s1 {state[1]};
    u64 const result {rotl(s0 * 5, 7) * 9};

    s1 ^= s0;
    state[0] = rotl(s0, 24) ^ s1 ^ (s1 << 16); // a, b
    state[1] = rotl(s1, 37);                   // c

    return result;
}

void xoroshiro_128_star_star::seed(state_type& state, seed_type seed) const
{
    rng_split_mix_64 rnd {seed};
    state[0] = rnd.next();
    state[1] = rnd.next();
}

////////////////////////////////////////////////////////////

auto xoshiro_256_plus::operator()(state_type& state) -> result_type
{
    u64 const result {state[0] + state[3]};

    u64 const t {state[1] << 17};

    state[2] ^= state[0];
    state[3] ^= state[1];
    state[1] ^= state[2];
    state[0] ^= state[3];

    state[2] ^= t;

    state[3] = rotl(state[3], 45);

    return result;
}

void xoshiro_256_plus::seed(state_type& state, seed_type seed) const
{
    rng_split_mix_64 rnd {seed};
    state[0] = rnd.next();
    state[1] = rnd.next();
    state[2] = rnd.next();
    state[3] = rnd.next();
}

////////////////////////////////////////////////////////////

auto xoshiro_256_plus_plus::operator()(state_type& state) -> result_type
{
    u64 const result {rotl(state[0] + state[3], 23) + state[0]};

    u64 const t {state[1] << 17};

    state[2] ^= state[0];
    state[3] ^= state[1];
    state[1] ^= state[2];
    state[0] ^= state[3];

    state[2] ^= t;

    state[3] = rotl(state[3], 45);

    return result;
}

void xoshiro_256_plus_plus::seed(state_type& state, seed_type seed) const
{
    rng_split_mix_64 rnd {seed};
    state[0] = rnd.next();
    state[1] = rnd.next();
    state[2] = rnd.next();
    state[3] = rnd.next();
}

////////////////////////////////////////////////////////////

auto xoshiro_256_star_star::operator()(state_type& state) -> result_type
{
    u64 const result {rotl(state[1] * 5, 7) * 9};

    u64 const t {state[1] << 17};

    state[2] ^= state[0];
    state[3] ^= state[1];
    state[1] ^= state[2];
    state[0] ^= state[3];

    state[2] ^= t;

    state[3] = rotl(state[3], 45);

    return result;
}

void xoshiro_256_star_star::seed(state_type& state, seed_type seed) const
{
    rng_split_mix_64 rnd {seed};
    state[0] = rnd.next();
    state[1] = rnd.next();
    state[2] = rnd.next();
    state[3] = rnd.next();
}

////////////////////////////////////////////////////////////

auto well_512_a::operator()(state_type& state) -> result_type
{
    u32 a {state[_index]};
    u32 c {state[(_index + 13) & 15]};
    u32 b {a ^ c ^ (a << 16) ^ (c << 15)};
    c = state[(_index + 9) & 15];
    c ^= (c >> 11);
    a = state[_index] = b ^ c;
    u32 d {a ^ ((a << 5) & 0xDA442D24UL)};
    _index        = (_index + 15) & 15;
    a             = state[_index];
    state[_index] = a ^ b ^ d ^ (a << 2) ^ (b << 18) ^ (c << 28);
    return state[_index];
}

void well_512_a::seed(state_type& state, seed_type seed) const
{
    rng_split_mix_32 rnd {seed};
    for (usize i {0}; i < state.size(); ++i) {
        state[i] = rnd.next();
    }
}

////////////////////////////////////////////////////////////

}
