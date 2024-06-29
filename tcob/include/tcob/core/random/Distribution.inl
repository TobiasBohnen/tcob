// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Distribution.hpp"

#include <cassert>
#include <numeric>

namespace tcob::random {

template <typename R, Arithmetic T>
inline auto uniform_distribution_base::operator()(R& rng, T min, T max) -> T
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

inline auto uniform_distribution_base::NextFloat(auto&& rng) -> f64
{
    return uniform_distribution_base {}(rng, 0.0, 1.0);
}

////////////////////////////////////////////////////////////

template <Arithmetic T>
inline uniform_distribution<T>::uniform_distribution(T min, T max)
    : _min {min}
    , _max {max}
{
}

template <Arithmetic T>
inline auto uniform_distribution<T>::operator()(auto&& rng) -> T
{
    uniform_distribution_base uniform;
    return uniform(rng, _min, _max);
}

////////////////////////////////////////////////////////////

inline binomial_distribution::binomial_distribution(i32 trials, f64 p)
    : _trials {trials}
    , _p {p}
{
}

inline auto binomial_distribution::operator()(auto&& rng) -> i32
{
    i32 retValue {0};
    for (i32 i {0}; i < _trials; ++i) {
        if (uniform_distribution_base::NextFloat(rng) < _p) {
            retValue++;
        }
    }
    return retValue;
}

////////////////////////////////////////////////////////////

inline bernoulli_distribution::bernoulli_distribution(f64 p)
    : _p {p}
{
}

inline auto bernoulli_distribution::operator()(auto&& rng) -> bool
{
    return uniform_distribution_base::NextFloat(rng) < _p;
}

////////////////////////////////////////////////////////////

inline beta_distribution::beta_distribution(f64 alpha, f64 beta)
    : _alpha {alpha}
    , _beta {beta}
{
}

inline auto beta_distribution::operator()(auto&& rng) -> f64
{
    gamma_distribution gamma_alpha {_alpha, 1.0};
    gamma_distribution gamma_beta {_beta, 1.0};

    f64 const x {gamma_alpha(rng)};
    f64 const y {gamma_beta(rng)};

    return x / (x + y);
}

////////////////////////////////////////////////////////////

inline cauchy_distribution::cauchy_distribution(f64 x0, f64 gamma)
    : _x0 {x0}
    , _gamma {gamma}
{
}

inline auto cauchy_distribution::operator()(auto&& rng) -> f64
{
    f64 const u {uniform_distribution_base::NextFloat(rng)};
    return _x0 + _gamma * std::tan(TAU_F / 2 * (u - 0.5f));
}

////////////////////////////////////////////////////////////

inline discrete_distribution::discrete_distribution(std::span<f64 const> probabilities)
{
    f64 const sum {std::accumulate(probabilities.begin(), probabilities.end(), 0.0)};

    _probs.push_back(probabilities[0] / sum);
    for (usize i {1}; i < probabilities.size(); ++i) {
        _probs.push_back(_probs[i - 1] + (probabilities[i] / sum));
    }
}

inline auto discrete_distribution::operator()(auto&& rng) -> i32
{
    f64 const u {uniform_distribution_base::NextFloat(rng)};

    for (usize i {0}; i < _probs.size(); ++i) {
        if (u < _probs[i]) {
            return static_cast<i32>(i);
        }
    }
    return static_cast<i32>(_probs.size() - 1);
}

////////////////////////////////////////////////////////////

inline gamma_distribution::gamma_distribution(f64 shape, f64 scale)
    : _shape {shape}
    , _scale {scale}
{
}

inline auto gamma_distribution::operator()(auto&& rng) -> f64
{
    f64 const d {_shape - 1.0 / 3.0};
    f64 const c {1.0 / sqrt(9.0 * d)};

    normal_distribution normal {0.0, 1.0};

    for (;;) {
        f64 x {}, v {};
        do {
            x = normal(rng);
            v = 1.0 + c * x;
        } while (v <= 0.0);

        v = v * v * v;
        f64 const u {uniform_distribution_base::NextFloat(rng)};

        if (u < 1.0 - 0.0331 * (x * x) * (x * x)) {
            return _scale * d * v;
        }

        if (std::log(u) < 0.5 * x * x + d * (1.0 - v + std::log(v))) {
            return _scale * d * v;
        }
    }
}

////////////////////////////////////////////////////////////

inline exponential_distribution::exponential_distribution(f64 lambda)
    : _lambda {lambda}
{
}

inline auto exponential_distribution::operator()(auto&& rng) -> f64
{
    f64 const u {uniform_distribution_base::NextFloat(rng)};
    return -std::log(1 - u) / _lambda;
}

////////////////////////////////////////////////////////////

inline negative_binomial_distribution::negative_binomial_distribution(i32 successes, f64 p)
    : _successes {successes}
    , _p {p}
{
}

inline auto negative_binomial_distribution::operator()(auto&& rng) -> i32
{
    gamma_distribution   gamma {static_cast<f64>(_successes), (1 - _p) / _p};
    poisson_distribution poisson {gamma(rng)};
    return poisson(rng);
}

////////////////////////////////////////////////////////////

inline normal_distribution::normal_distribution(f64 mean, f64 stdDev)
    : _mean {mean}
    , _stdDev {stdDev}
{
}

inline auto normal_distribution::operator()(auto&& rng) -> f64
{
    if (_toggle) {
        _toggle = false;
        return _x2 * _stdDev + _mean;
    }

    f64 v1 {}, v2 {}, s {0};
    do {
        v1 = 2 * uniform_distribution_base::NextFloat(rng) - 1;
        v2 = 2 * uniform_distribution_base::NextFloat(rng) - 1;
        s  = v1 * v1 + v2 * v2;
    } while (s >= 1 || s == 0);

    f64 const multiplier {std::sqrt(-2 * std::log(s) / s)};
    _x2     = v2 * multiplier;
    _toggle = true;
    return v1 * multiplier * _stdDev + _mean;
}

////////////////////////////////////////////////////////////

inline lognormal_distribution::lognormal_distribution(f64 logScale, f64 shape)
    : _normal {logScale, shape}
{
}

inline auto lognormal_distribution::operator()(auto&& rng) -> f64
{
    return std::exp(_normal(rng));
}

////////////////////////////////////////////////////////////

inline pareto_distribution::pareto_distribution(f64 alpha, f64 xm)
    : _alpha {alpha}
    , _xm {xm}
{
}

inline auto pareto_distribution::operator()(auto&& rng) -> f64
{
    f64 const u {uniform_distribution_base::NextFloat(rng)};
    return _xm / std::pow(u, 1.0f / _alpha);
}

////////////////////////////////////////////////////////////

inline piecewise_constant_distribution::piecewise_constant_distribution(std::span<f64 const> intervals, std::span<f64 const> weights)
    : _intervals {intervals.begin(), intervals.end()}
{
    _cumulativeWeights.resize(weights.size());
    std::partial_sum(weights.begin(), weights.end(), _cumulativeWeights.begin());
}

inline auto piecewise_constant_distribution::operator()(auto&& rng) -> f64
{
    uniform_distribution_base uniform;

    f64 const u {uniform(rng, 0., _cumulativeWeights.back())};

    auto const  it {std::upper_bound(_cumulativeWeights.begin(), _cumulativeWeights.end(), u)};
    isize const index {std::distance(_cumulativeWeights.begin(), it)};

    return uniform(rng, _intervals[index], _intervals[index + 1]);
}

////////////////////////////////////////////////////////////

inline poisson_distribution::poisson_distribution(f64 mean)
    : _mean {mean}
{
}

inline auto poisson_distribution::operator()(auto&& rng) -> i32
{
    f64 const L {std::exp(-_mean)};
    i32       k {0};
    f64       p {1.0};

    do {
        ++k;
        p *= uniform_distribution_base::NextFloat(rng);
    } while (p > L);

    return k - 1;
}

////////////////////////////////////////////////////////////

inline triangular_distribution::triangular_distribution(f64 min, f64 max, f64 peak)
    : _min {min}
    , _max {max}
    , _peak {peak}
{
}

inline auto triangular_distribution::operator()(auto&& rng) -> f64
{
    f64 const u {uniform_distribution_base::NextFloat(rng)};
    f64 const F {(_peak - _min) / (_max - _min)};
    return u < F ? _min + std::sqrt(u * (_max - _min) * (_peak - _min))
                 : _max - std::sqrt((1 - u) * (_max - _min) * (_max - _peak));
}

////////////////////////////////////////////////////////////

inline weibull_distribution::weibull_distribution(f64 shape, f64 scale)
    : _shape {shape}
    , _scale {scale}
{
}

inline auto weibull_distribution::operator()(auto&& rng) -> f64
{
    f64 const u {uniform_distribution_base::NextFloat(rng)};
    return _scale * std::pow(-std::log(1 - u), 1 / _shape);
}

////////////////////////////////////////////////////////////

inline bag_distribution::bag_distribution(i64 min, i64 max, isize period)
    : _min {min}
    , _max {max}
    , _period {period}
{
}

inline auto bag_distribution::operator()(auto&& rng) -> i64
{
    if (_seq.empty()) { gen_seq(rng); }

    i64 const retValue {_seq.back()};
    _seq.pop_back();
    return retValue;
}

inline void bag_distribution::gen_seq(auto&& rng)
{
    std::vector<i64> sequence;
    for (i64 i {_min}; i <= _max; ++i) { sequence.push_back(i); }
    for (isize i {0}; i < _period; ++i) { _seq.insert(_seq.end(), sequence.begin(), sequence.end()); }

    auto const range {_seq.size()};
    for (usize i {range - 1}; i > 0; --i) { std::swap(_seq[i], _seq[rng.next() % range]); }
}
}
