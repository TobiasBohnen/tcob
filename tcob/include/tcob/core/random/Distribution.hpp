// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/core/Concepts.hpp"

namespace tcob::random {
////////////////////////////////////////////////////////////

class uniform_distribution_base {
public:
    template <typename R, Arithmetic T>
    auto operator()(R& rng, T min, T max) -> T;

    auto static NextFloat(auto&& rng) -> f32;
};

////////////////////////////////////////////////////////////

template <Arithmetic T>
class uniform_distribution {
public:
    uniform_distribution(T min, T max);

    auto operator()(auto&& rng) -> T;

private:
    T _min;
    T _max;
};

////////////////////////////////////////////////////////////

class bernoulli_distribution {
public:
    bernoulli_distribution(f32 p);

    auto operator()(auto&& rng) -> bool;

private:
    f32 _p;
};

////////////////////////////////////////////////////////////

class cauchy_distribution {
public:
    cauchy_distribution(f32 x0, f32 gamma);

    auto operator()(auto&& rng) -> f32;

private:
    f32 _x0;
    f32 _gamma;
};

////////////////////////////////////////////////////////////

class discrete_distribution {
public:
    discrete_distribution(std::span<f32> probabilities);

    auto operator()(auto&& rng) -> i32;

private:
    std::vector<f32> _probs;
};

////////////////////////////////////////////////////////////

class exponential_distribution {
public:
    exponential_distribution(f32 lambda);

    auto operator()(auto&& rng) -> f32;

private:
    f32 _lambda;
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

class pareto_distribution {
public:
    pareto_distribution(f32 alpha, f32 xm);

    auto operator()(auto&& rng) -> f32;

private:
    f32 _alpha;
    f32 _xm;
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

class poisson_distribution {
public:
    poisson_distribution(f32 mean);

    auto operator()(auto&& rng) -> i32;

private:
    f32 _mean;
};

////////////////////////////////////////////////////////////

class triangular_distribution {
public:
    triangular_distribution(f32 min, f32 max, f32 peak);

    auto operator()(auto&& rng) -> f32;

private:
    f32 _min;
    f32 _max;
    f32 _peak;
};

////////////////////////////////////////////////////////////

class weibull_distribution {
public:
    weibull_distribution(f32 shape, f32 scale);

    auto operator()(auto&& rng) -> f32;

private:
    f32 _shape;
    f32 _scale;
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
