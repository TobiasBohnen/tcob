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
        result_type const unbiasedMax {std::numeric_limits<result_type>::max() / range * range - 1};
        result_type       value;
        do {
            value = rng.next();
        } while (value > unbiasedMax);
        return static_cast<T>(min + static_cast<T>(value % range));
    } else {
        return T {};
    }
}

////////////////////////////////////////////////////////////

inline normal_distribution::normal_distribution(f32 mean, f32 stdDev)
    : _mean {mean}
    , _stdDev {stdDev}
{
}

template <typename R>
inline auto normal_distribution::operator()(R& rng) -> f32
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
    return x1 * _stdDev + _mean;
}

////////////////////////////////////////////////////////////

inline rep_seq_distribution::rep_seq_distribution(i64 min, i64 max, isize period)
    : _min {min}
    , _max {max}
    , _period {period}
{
}

template <typename R>
inline auto rep_seq_distribution::operator()(R& rng) -> i64
{
    if (_seq.empty()) { gen_seq(rng); }

    i64 const retValue {_seq.back()};
    _seq.pop_back();
    return retValue;
}

template <typename R>
inline void rep_seq_distribution::gen_seq(R& rng)
{
    std::vector<i64> sequence;
    for (i64 i {_min}; i <= _max; ++i) { sequence.push_back(i); }
    for (isize i {0}; i < _period; ++i) { _seq.insert(_seq.end(), sequence.begin(), sequence.end()); }

    auto const range {_seq.size()};
    for (usize i {range - 1}; i > 0; --i) { std::swap(_seq[i], _seq[rng.next() % range]); }
}

////////////////////////////////////////////////////////////

template <RandomEngine E, typename D>
inline random_number_generator<E, D>::random_number_generator(seed_type seed, auto&&... distArgs)
    : _distribution {distArgs...}
{
    _engine.seed(_state, seed);
}

template <RandomEngine E, typename D>
inline random_number_generator<E, D>::random_number_generator(state_type state, auto&&... distArgs)
    : _state {state}
    , _distribution {distArgs...}
{
}

template <RandomEngine E, typename D>
inline auto random_number_generator<E, D>::operator()(auto&&... arg)
{
    return _distribution(*this, arg...);
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

////////////////////////////////////////////////////////////

template <i32 N, RandomEngine E>
inline dice<N, E>::dice(seed_type seed)
    : _random {seed}
{
}

template <i32 N, RandomEngine E>
inline dice<N, E>::dice(state_type state)
    : _random {state}
{
}

template <i32 N, RandomEngine E>
inline auto dice<N, E>::roll() -> i32
{
    return _random(1, N);
}

template <i32 N, RandomEngine E>
inline auto dice<N, E>::roll_n(usize n) -> std::vector<i32>
{
    std::vector<i32> retValue(n);
    std::generate_n(retValue.begin(), n, [&]() { return roll(); });
    return retValue;
}

template <i32 N, RandomEngine E>
inline auto dice<N, E>::get_state() const -> state_type const&
{
    return _random.get_state();
}

////////////////////////////////////////////////////////////

template <typename T, RandomEngine E>
inline shuffle<T, E>::shuffle(seed_type seed)
    : _random {seed}
{
}

template <typename T, RandomEngine E>
inline shuffle<T, E>::shuffle(state_type state)
    : _random {state}
{
}

template <typename T, RandomEngine E>
inline auto shuffle<T, E>::get_state() const -> state_type const&
{
    return _random.get_state();
}

template <typename T, RandomEngine E>
inline void shuffle<T, E>::operator()(std::span<T> span)
{
    if (span.size() <= 1) { return; }

    for (usize i {span.size() - 1}; i > 0; --i) {
        usize const j {_random(static_cast<usize>(0), i)};
        using std::swap;
        swap(span[i], span[j]);
    }
}

}
