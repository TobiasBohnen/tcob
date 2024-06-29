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

inline auto uniform_distribution_base::NextFloat(auto&& rng) -> f32
{
    return uniform_distribution_base {}(rng, 0.0f, 1.0f);
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

inline bernoulli_distribution::bernoulli_distribution(f32 p)
    : _p {p}
{
}

inline auto bernoulli_distribution::operator()(auto&& rng) -> bool
{
    return uniform_distribution_base::NextFloat(rng) < _p;
}

////////////////////////////////////////////////////////////

inline cauchy_distribution::cauchy_distribution(f32 x0, f32 gamma)
    : _x0 {x0}
    , _gamma {gamma}
{
}

inline auto cauchy_distribution::operator()(auto&& rng) -> f32
{
    f32 const u {uniform_distribution_base::NextFloat(rng)};
    return _x0 + _gamma * std::tan(TAU_F / 2 * (u - 0.5f));
}

////////////////////////////////////////////////////////////

inline discrete_distribution::discrete_distribution(std::span<f32> probabilities)
{
    [[maybe_unused]] f32 sum {0.0f};
    for (f32 const prob : probabilities) {
        assert(prob >= 0.0);
        sum += prob;
    }
    assert(std::abs(sum - 1.0f) < 1e-9);

    _probs.push_back(probabilities[0]);
    for (usize i {1}; i < probabilities.size(); ++i) {
        _probs.push_back(_probs[i - 1] + probabilities[i]);
    }
}

inline auto discrete_distribution::operator()(auto&& rng) -> i32
{
    f32 const u {uniform_distribution_base::NextFloat(rng)};

    for (usize i {0}; i < _probs.size(); ++i) {
        if (u < _probs[i]) {
            return static_cast<i32>(i);
        }
    }
    return static_cast<i32>(_probs.size() - 1);
}

////////////////////////////////////////////////////////////

inline exponential_distribution::exponential_distribution(f32 lambda)
    : _lambda {lambda}
{
}

inline auto exponential_distribution::operator()(auto&& rng) -> f32
{
    f32 const u {uniform_distribution_base::NextFloat(rng)};
    return -std::log(1 - u) / _lambda;
}

////////////////////////////////////////////////////////////

inline normal_distribution::normal_distribution(f32 mean, f32 stdDev)
    : _mean {mean}
    , _stdDev {stdDev}
{
}

inline auto normal_distribution::operator()(auto&& rng) -> f32
{
    if (_toggle) {
        _toggle = false;
        return _x2 * _stdDev + _mean;
    }

    f32 v1 {}, v2 {}, s {0};
    do {
        v1 = 2 * uniform_distribution_base::NextFloat(rng) - 1;
        v2 = 2 * uniform_distribution_base::NextFloat(rng) - 1;
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

inline auto pareto_distribution::operator()(auto&& rng) -> f32
{
    f32 const u {uniform_distribution_base::NextFloat(rng)};
    return _xm / std::pow(u, 1.0f / _alpha);
}

////////////////////////////////////////////////////////////

inline piecewise_constant_distribution::piecewise_constant_distribution(std::span<f32 const> intervals, std::span<f32 const> weights)
    : _intervals {intervals.begin(), intervals.end()}
{
    _cumulativeWeights.resize(weights.size());
    std::partial_sum(weights.begin(), weights.end(), _cumulativeWeights.begin());
}

inline auto piecewise_constant_distribution::operator()(auto&& rng) -> f32
{
    uniform_distribution_base uniform;

    f32 const u {uniform(rng, 0.f, _cumulativeWeights.back())};

    auto const  it {std::upper_bound(_cumulativeWeights.begin(), _cumulativeWeights.end(), u)};
    isize const index {std::distance(_cumulativeWeights.begin(), it)};

    return uniform(rng, _intervals[index], _intervals[index + 1]);
}

////////////////////////////////////////////////////////////

inline poisson_distribution::poisson_distribution(f32 mean)
    : _mean {mean}
{
}

inline auto poisson_distribution::operator()(auto&& rng) -> i32
{
    f32 const L {std::exp(-_mean)};
    i32       k {0};
    f32       p {1.0};

    do {
        ++k;
        p *= uniform_distribution_base::NextFloat(rng);
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

inline auto triangular_distribution::operator()(auto&& rng) -> f32
{
    f32 const u {uniform_distribution_base::NextFloat(rng)};
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

inline auto weibull_distribution::operator()(auto&& rng) -> f32
{
    f32 const u {uniform_distribution_base::NextFloat(rng)};
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
