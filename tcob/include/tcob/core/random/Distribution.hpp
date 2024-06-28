// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/core/Concepts.hpp"

////////////////////////////////////////////////////////////
namespace tcob::random {

class uniform_distribution {
public:
    template <typename R, Arithmetic T>
    auto operator()(R& rng, T min, T max) -> T;

    auto static NextFloat(auto&& rng) -> f32;
};

////////////////////////////////////////////////////////////

class bernoulli_distribution {
public:
    auto operator()(auto&& rng, f32 p) -> bool;
};

////////////////////////////////////////////////////////////

class cauchy_distribution {
public:
    auto operator()(auto&& rng, f32 x0, f32 gamma) -> f32;
};

////////////////////////////////////////////////////////////

class exponential_distribution {
public:
    auto operator()(auto&& rng, f32 lambda) -> f32;
};

////////////////////////////////////////////////////////////

class pareto_distribution {
public:
    auto operator()(auto&& rng, f32 alpha, f32 xm) -> f32;
};

////////////////////////////////////////////////////////////

class poisson_distribution {
public:
    auto operator()(auto&& rng, f32 mean) -> i32;
};

////////////////////////////////////////////////////////////

class triangular_distribution {
public:
    auto operator()(auto&& rng, f32 min, f32 max, f32 peak) -> f32;
};

////////////////////////////////////////////////////////////

class weibull_distribution {
public:
    auto operator()(auto&& rng, f32 shape, f32 scale) -> f32;
};

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

class discrete_distribution {
public:
    discrete_distribution(std::span<f32 const> probabilities);

    auto operator()(auto&& rng) -> i32;

private:
    std::vector<f32> _probs;
};

////////////////////////////////////////////////////////////

class normal_distribution {
public:
    normal_distribution(f32 mean, f32 stdDev);

    auto operator()(auto&& rng) -> f32;

private:
    f32  _mean;
    f32  _stdDev;
    bool _toggle {false};
    f32  _x2 {0.0f};
};

////////////////////////////////////////////////////////////

class piecewise_constant_distribution {
public:
    piecewise_constant_distribution(std::span<f32 const> intervals, std::span<f32 const> weights);

    auto operator()(auto&& rng) -> f32;

private:
    std::vector<f32> _intervals;
    std::vector<f32> _cumulativeWeights;
};

////////////////////////////////////////////////////////////

class bag_distribution {
public:
    bag_distribution(i64 min, i64 max, isize period);

    auto operator()(auto&& rng) -> i64;

private:
    void gen_seq(auto&& rng);

    i64              _min;
    i64              _max;
    isize            _period;
    std::vector<i64> _seq;
};

}

#include "Distribution.inl"
