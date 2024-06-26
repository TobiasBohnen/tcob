// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Distribution.hpp"

#include <cassert>

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

inline beta_distribution::beta_distribution(f32 a, f32 b)
    : _a {a}
    , _b {b}
{
}

template <typename R>
inline auto beta_distribution::operator()(R& rng) -> f32
{
    gamma_distribution gamma_alpha {_a};
    gamma_distribution gamma_beta {_b};

    f32 const gamma_alpha_val {gamma_alpha(rng)};
    f32 const gamma_beta_val {gamma_beta(rng)};

    return gamma_alpha_val / (gamma_alpha_val + gamma_beta_val);
}

////////////////////////////////////////////////////////////

inline exponential_distribution::exponential_distribution(f32 lambda)
    : _lambda {lambda}
{
}

template <typename R>
inline auto exponential_distribution::operator()(R& rng) -> f32
{
    uniform_distribution uniform {};

    f32 const u {uniform(rng, 0.0f, 1.0f)};
    return -std::log(1 - u) / _lambda;
}

////////////////////////////////////////////////////////////

inline gamma_distribution::gamma_distribution(f32 shape)
    : _shape {shape}
{
}

template <typename R>
inline auto gamma_distribution::operator()(R& rng) -> f32
{
    uniform_distribution uniform {};

    f32 x {0};
    if (_shape == 1.0) {
        f32 u {0};
        do {
            u = uniform(rng, 0.0f, 1.0f);
        } while (u <= 1e-7 || u >= 1.0);

        return -std::log(u);
    }

    f32 u {0}, v {0};
    if (_shape < 1.0) {
        do {
            u = uniform(rng, 0.0f, 1.0f);
        } while (u <= 1e-7 || u >= 1.0);

        do {
            v = uniform(rng, 0.0f, 1.0f);
        } while (v <= 1e-7 || v >= 1.0);

        x = std::pow(u, 1.0f / _shape);
        if (v <= std::exp(-x)) {
            return x;
        }
    } else {
        do {
            v = uniform(rng, 0.0f, 1.0f);
        } while (v <= 1e-7 || v >= 1.0);

        do {
            u = uniform(rng, 0.0f, 1.0f);
        } while (u <= 1e-7 || u >= 1.0);

        x = -std::log(u * v);
        f32 const d {(_shape - 1.0f / 3.0f) * std::sqrt(9.0f * x) / 2.0f - 1.0f};
        f32 const c {x < 1.0 ? std::exp(d) : d};

        do {
            u = uniform(rng, 0.0f, 1.0f);
            v = uniform(rng, 0.0f, 1.0f);
        } while (v <= 1e-7 || u > std::pow(c * (1.0f - 1.0f / _shape), _shape - 1.0f));

        x = c * u * u * u;
    }

    return x;
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
    if (_toggle) {
        _toggle = false;
        return _x2 * _stdDev + _mean;
    }

    f32                  v1 {}, v2 {}, s {0};
    uniform_distribution uniform {};
    do {
        v1 = 2 * uniform(rng, 0.0f, 1.0f) - 1;
        v2 = 2 * uniform(rng, 0.0f, 1.0f) - 1;
        s  = v1 * v1 + v2 * v2;
    } while (s >= 1 || s == 0);

    f32 const multiplier {std::sqrt(-2 * std::log(s) / s)};
    _x2     = v2 * multiplier;
    _toggle = true;
    return v1 * multiplier * _stdDev + _mean;
}

////////////////////////////////////////////////////////////

inline pareto_distribution::pareto_distribution(f32 alpha, f32 xm)
    : _alpha {alpha}
    , _xm {xm}
{
}

template <typename R>
inline auto pareto_distribution::operator()(R& rng) -> f32
{
    uniform_distribution uniform {};

    f32 const u {uniform(rng, 0.0f, 1.0f)};
    return _xm / std::pow(u, 1.0f / _alpha);
}

////////////////////////////////////////////////////////////

inline poisson_distribution::poisson_distribution(f32 mean)
    : _mean {mean}
{
}

template <typename R>
inline auto poisson_distribution::operator()(R& rng) -> i32
{
    uniform_distribution uniform {};

    f32 const L {std::exp(-_mean)};
    i32       k {0};
    f32       p {1.0};

    do {
        ++k;
        p *= uniform(rng, 0.0f, 1.0f);
    } while (p > L);

    return k - 1;
}

////////////////////////////////////////////////////////////

inline triangular_distribution::triangular_distribution(f32 min, f32 max, f32 peak)
    : _min {min}
    , _max {max}
    , _peak {peak}
{
}

template <typename R>
inline auto triangular_distribution::operator()(R& rng) -> f32
{
    uniform_distribution uniform {};

    f32 const u {uniform(rng, 0.0f, 1.0f)};
    f32 const F {(_peak - _min) / (_max - _min)};
    return u < F ? _min + std::sqrt(u * (_max - _min) * (_peak - _min))
                 : _max - std::sqrt((1 - u) * (_max - _min) * (_max - _peak));
}

////////////////////////////////////////////////////////////

inline weibull_distribution::weibull_distribution(f32 shape, f32 scale)
    : _shape {shape}
    , _scale {scale}
{
}

template <typename R>
inline auto weibull_distribution::operator()(R& rng) -> f32
{
    uniform_distribution uniform {};

    f32 const u {uniform(rng, 0.0f, 1.0f)};
    return _scale * std::pow(-std::log(1 - u), 1 / _shape);
}

////////////////////////////////////////////////////////////

inline bag_distribution::bag_distribution(i64 min, i64 max, isize period)
    : _min {min}
    , _max {max}
    , _period {period}
{
}

template <typename R>
inline auto bag_distribution::operator()(R& rng) -> i64
{
    if (_seq.empty()) { gen_seq(rng); }

    i64 const retValue {_seq.back()};
    _seq.pop_back();
    return retValue;
}

template <typename R>
inline void bag_distribution::gen_seq(R& rng)
{
    std::vector<i64> sequence;
    for (i64 i {_min}; i <= _max; ++i) { sequence.push_back(i); }
    for (isize i {0}; i < _period; ++i) { _seq.insert(_seq.end(), sequence.begin(), sequence.end()); }

    auto const range {_seq.size()};
    for (usize i {range - 1}; i > 0; --i) { std::swap(_seq[i], _seq[rng.next() % range]); }
}
}
