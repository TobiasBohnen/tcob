// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Random.hpp"

#include <cassert>
#include <cmath>

namespace tcob::random {

template <typename R, Arithmetic T>

inline auto uniform_distribution::operator()(R& rng, T min, T max) -> T
{
    assert(min <= max);
    if (min == max) { return min; }

    using result_type = typename R::result_type;
    static_assert(sizeof(result_type) == 8 || sizeof(result_type) == 4);
    if constexpr (FloatingPoint<T>) {
        using int_type   = std::conditional_t<sizeof(result_type) == 4, i32, std::conditional_t<sizeof(result_type) == 8, i64, void>>;
        using float_type = std::conditional_t<sizeof(result_type) == 4, f32, std::conditional_t<sizeof(result_type) == 8, f64, void>>;
        float_type const value {
            static_cast<int_type>(rng.next() >> ((sizeof(float_type) * 8) - std::numeric_limits<float_type>::digits))
            / static_cast<float_type>(1LL << std::numeric_limits<float_type>::digits)};
        return static_cast<T>(min + (value * (max - min)));
    } else if constexpr (Integral<T>) {
        result_type const range {static_cast<result_type>(max - min + 1)};
        result_type       value {rng.next()};
        return static_cast<T>(min + static_cast<T>(value % range));
    } else {
        return T {};
    }
}

template <typename R>
inline auto normal_distribution::operator()(R& rng, f32 mean, f32 stdDev) -> f32
{
    f32 x1 {0.0f};
    if (_toggle) {
        x1 = _x2;
    } else {
        f32                  v1 {0.0f}, v2 {0.0f};
        f32                  s {0.0f};
        uniform_distribution uniform {};
        for (;;) {
            v1 = 2 * uniform(rng, 0.0f, 1.0f) - 1;
            v2 = 2 * uniform(rng, 0.0f, 1.0f) - 1;
            s  = v1 * v1 + v2 * v2;

            if (s < 1 && s != 0) { break; }
        }

        x1  = v1 * std::sqrt(-2 * std::log(s) / s);
        _x2 = v2 * std::sqrt(-2 * std::log(s) / s);
    }
    _toggle = !_toggle;
    return x1 * stdDev + mean;
}

////////////////////////////////////////////////////////////

template <RandomEngine E, typename D>
inline random_number_generator<E, D>::random_number_generator(seed_type seed)
{
    _engine.seed(_state, seed);
}

template <RandomEngine E, typename D>
inline random_number_generator<E, D>::random_number_generator(state_type state)
    : _state {state}
{
}

template <RandomEngine E, typename D>
inline auto random_number_generator<E, D>::operator()(auto&&... arg)
{
    if constexpr (sizeof...(arg) == 0) {
        return _distribution(*this, min(), max());
    } else {
        return _distribution(*this, arg...);
    }
}

template <RandomEngine E, typename D>
inline auto random_number_generator<E, D>::next() -> result_type
{
    return _engine(_state);
}

template <RandomEngine E, typename D>
inline auto random_number_generator<E, D>::get_state() const -> state_type const&
{
    return _state;
}

template <RandomEngine E, typename D>
template <typename X>
inline void random_number_generator<E, D>::shuffle(std::span<X> span)
{
    if (span.size() <= 1) { return; }

    for (usize i {span.size() - 1}; i > 0; --i) {
        usize const j {operator()(static_cast<usize>(0), i)};
        using std::swap;
        swap(span[i], span[j]);
    }
}

template <RandomEngine E, typename D>
inline auto constexpr random_number_generator<E, D>::min() -> result_type
{
    return std::numeric_limits<result_type>::min();
}

template <RandomEngine E, typename D>
inline auto constexpr random_number_generator<E, D>::max() -> result_type
{
    return std::numeric_limits<result_type>::max() - 1;
}

////////////////////////////////////////////////////////////

template <i32 N, typename RandomGenerator>
inline dice<N, RandomGenerator>::dice(seed_type seed)
    : _random {seed}
{
}

template <i32 N, typename RandomGenerator>
inline dice<N, RandomGenerator>::dice(state_type state)
    : _random {state}
{
}

template <i32 N, typename RandomGenerator>
inline auto dice<N, RandomGenerator>::roll() -> i32
{
    return _random(1, N);
}

template <i32 N, typename RandomGenerator>
inline auto dice<N, RandomGenerator>::roll_n(usize n) -> std::vector<i32>
{
    std::vector<i32> retValue(n);
    std::generate_n(retValue.begin(), n, [&]() { return roll(); });
    return retValue;
}

template <i32 N, typename RandomGenerator>
inline auto dice<N, RandomGenerator>::get_state() const -> state_type const&
{
    return _random.get_state();
}

}
