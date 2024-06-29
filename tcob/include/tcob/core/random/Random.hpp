// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <chrono>
#include <span>

#include "tcob/core/Concepts.hpp"
#include "tcob/core/random/Distribution.hpp"
#include "tcob/core/random/Engine.hpp"

namespace tcob {

namespace random {

    ////////////////////////////////////////////////////////////

    template <RandomEngine E, typename D = uniform_distribution_base>
    class random_number_generator final {
    public:
        using random_engine_type = E;
        using state_type         = typename E::state_type;
        using seed_type          = typename E::seed_type;
        using result_type        = typename E::result_type;
        using distribution_type  = D;

        explicit random_number_generator(seed_type seed = static_cast<seed_type>(clock::now().time_since_epoch().count()), auto&&... distArgs);
        explicit random_number_generator(state_type state, auto&&... distArgs);

        auto operator()(auto&&... distArgs);

        auto next() -> result_type;
        auto get_state() const -> state_type const&;

    private:
        random_engine_type _engine {};
        state_type         _state {};
        distribution_type  _distribution;
    };

    ////////////////////////////////////////////////////////////

    using rng_split_mix_32            = random_number_generator<split_mix_32>;
    using rng_split_mix_64            = random_number_generator<split_mix_64>;
    using rng_game_rand               = random_number_generator<game_rand>;
    using rng_xorshift_64             = random_number_generator<xorshift_64>;
    using rng_xorshift_64_star        = random_number_generator<xorshift_64_star>;
    using rng_xoroshiro_128_plus      = random_number_generator<xoroshiro_128_plus>;
    using rng_xoroshiro_128_plus_plus = random_number_generator<xoroshiro_128_plus_plus>;
    using rng_xoroshiro_128_star_star = random_number_generator<xoroshiro_128_star_star>;
    using rng_xoshiro_256_plus        = random_number_generator<xoshiro_256_plus>;
    using rng_xoshiro_256_plus_plus   = random_number_generator<xoshiro_256_plus_plus>;
    using rng_xoshiro_256_star_star   = random_number_generator<xoshiro_256_star_star>;
    using rng_well_512_a              = random_number_generator<well_512_a>;

    ////////////////////////////////////////////////////////////

    template <i32 N, RandomEngine E = xoroshiro_128_plus_plus>
    class dice final {
        static_assert(N > 0, "N must be greater than 0");
        using state_type = typename E::state_type;
        using seed_type  = typename E::seed_type;

    public:
        explicit dice(seed_type seed = static_cast<seed_type>(clock::now().time_since_epoch().count()));
        explicit dice(state_type state);

        auto get_state() const -> state_type const&;

        auto roll() -> i32;

        auto roll_n(usize n) -> std::vector<i32>;

    private:
        random_number_generator<E> _random {};
    };

    ////////////////////////////////////////////////////////////

    template <typename T, RandomEngine E = xoroshiro_128_plus_plus>
    class shuffle final {
    public:
        using state_type = typename E::state_type;
        using seed_type  = typename E::seed_type;

        explicit shuffle(seed_type seed = static_cast<seed_type>(clock::now().time_since_epoch().count()));
        explicit shuffle(state_type state);

        auto get_state() const -> state_type const&;

        void operator()(std::span<T> span);

    private:
        random_number_generator<E> _random {};
    };

}

using rng = random::rng_xoroshiro_128_plus_plus;

}

#include "Random.inl"
