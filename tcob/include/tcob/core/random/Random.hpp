// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <chrono>
#include <span>
#include <vector>

#include "tcob/core/Concepts.hpp"
#include "tcob/core/random/Distribution.hpp"
#include "tcob/core/random/Engine.hpp"

namespace tcob {

namespace random {

    ////////////////////////////////////////////////////////////

    template <RandomEngine E, typename D>
    class prng final {
    public:
        using random_engine_type = E;
        using state_type         = typename E::state_type;
        using seed_type          = typename E::seed_type;
        using result_type        = typename E::result_type;
        using distribution_type  = D;

        explicit prng(seed_type seed = static_cast<seed_type>(clock::now().time_since_epoch().count()), auto&&... distArgs);
        explicit prng(state_type state, auto&&... distArgs);

        auto operator()(auto&&... distArgs);

        auto next() -> result_type;
        auto state() const -> state_type const&;

    private:
        random_engine_type _engine {};
        state_type         _state {};
        distribution_type  _distribution;
    };

    ////////////////////////////////////////////////////////////

    using prng_split_mix_32            = prng<split_mix_32, core_uniform_distribution>;
    using prng_split_mix_64            = prng<split_mix_64, core_uniform_distribution>;
    using prng_game_rand               = prng<game_rand, core_uniform_distribution>;
    using prng_xorshift_64             = prng<xorshift_64, core_uniform_distribution>;
    using prng_xorshift_64_star        = prng<xorshift_64_star, core_uniform_distribution>;
    using prng_xoroshiro_128_plus      = prng<xoroshiro_128_plus, core_uniform_distribution>;
    using prng_xoroshiro_128_plus_plus = prng<xoroshiro_128_plus_plus, core_uniform_distribution>;
    using prng_xoroshiro_128_star_star = prng<xoroshiro_128_star_star, core_uniform_distribution>;
    using prng_xoshiro_256_plus        = prng<xoshiro_256_plus, core_uniform_distribution>;
    using prng_xoshiro_256_plus_plus   = prng<xoshiro_256_plus_plus, core_uniform_distribution>;
    using prng_xoshiro_256_star_star   = prng<xoshiro_256_star_star, core_uniform_distribution>;
    using prng_well_512_a              = prng<well_512_a, core_uniform_distribution>;

    ////////////////////////////////////////////////////////////

    template <i32 N, RandomEngine E = xoroshiro_128_plus_plus>
    class dice final {
        static_assert(N > 0, "N must be greater than 0");
        using state_type = typename E::state_type;
        using seed_type  = typename E::seed_type;

    public:
        explicit dice(seed_type seed = static_cast<seed_type>(clock::now().time_since_epoch().count()));
        explicit dice(state_type state);

        auto state() const -> state_type const&;

        auto roll() -> i32;

        auto roll_n(usize n) -> std::vector<i32>;
        auto roll_n_sum(usize n) -> i32;

    private:
        prng<E, core_uniform_distribution> _random {};
    };

    ////////////////////////////////////////////////////////////

    template <typename T, RandomEngine E = xoroshiro_128_plus_plus>
    class shuffle final {
    public:
        using state_type = typename E::state_type;
        using seed_type  = typename E::seed_type;

        explicit shuffle(seed_type seed = static_cast<seed_type>(clock::now().time_since_epoch().count()));
        explicit shuffle(state_type state);

        auto state() const -> state_type const&;

        void operator()(std::span<T> span);

    private:
        prng<E, core_uniform_distribution> _random {};
    };

}

using rng = random::prng_xoroshiro_128_plus_plus;

TCOB_API auto get_random_ID() -> uid;

}

#include "Random.inl"
