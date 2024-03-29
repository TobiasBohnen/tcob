// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <array>
#include <chrono>
#include <span>

#include "tcob/core/Concepts.hpp"

namespace tcob {
////////////////////////////////////////////////////////////
namespace random {

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
    ////////////////////////////////////////////////////////////

    class uniform_distribution {
    public:
        template <typename R, Arithmetic T>
        auto operator()(R& rng, T min, T max) -> T;
    };

    class normal_distribution {
    public:
        template <typename R>
        auto operator()(R& rng, f32 mean, f32 stdDev) -> f32;

    private:
        [[maybe_unused]] bool _toggle {false};
        [[maybe_unused]] f32  _x2 {0.0f};
    };

    ////////////////////////////////////////////////////////////

    template <RandomEngine E, typename D = uniform_distribution>
    class random_number_generator final {
    public:
        using random_engine_type = E;
        using state_type         = typename E::state_type;
        using seed_type          = typename E::seed_type;
        using result_type        = typename E::result_type;
        using distribution_type  = D;

        explicit random_number_generator(seed_type seed = static_cast<seed_type>(clock::now().time_since_epoch().count()));
        explicit random_number_generator(state_type state);

        auto operator()(auto&&... arg);

        auto next() -> result_type;
        auto get_state() const -> state_type const&;

        template <typename X>
        void shuffle(std::span<X> span);

        auto constexpr static min() -> result_type;
        auto constexpr static max() -> result_type;

    private:
        random_engine_type _engine {};
        state_type         _state {};
        distribution_type  _distribution {};
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

    ////////////////////////////////////////////////////////////

    template <i32 N, typename RandomGenerator = rng_xoroshiro_128_plus_plus>
    class dice final {
        static_assert(N > 0, "N must be greater than 0");
        using state_type = typename RandomGenerator::state_type;
        using seed_type  = typename RandomGenerator::seed_type;

    public:
        explicit dice(seed_type seed = static_cast<seed_type>(clock::now().time_since_epoch().count()));
        explicit dice(state_type state);

        auto get_state() const -> state_type const&;

        auto roll() -> i32;

        auto roll_n(usize n) -> std::vector<i32>;

    private:
        RandomGenerator _random {};
    };

}

using rng = random::rng_xoroshiro_128_plus_plus;

}

#include "Random.inl"
