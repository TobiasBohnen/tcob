// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <span>
#include <vector>

#include "tcob/core/Concepts.hpp"

namespace tcob::random {
////////////////////////////////////////////////////////////

class core_uniform_distribution {
public:
    template <typename R, Arithmetic T>
    auto operator()(R& rng, T min, T max) -> T;

    auto static NextFloat(auto&& rng) -> f64;
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
    bernoulli_distribution(f64 p);

    auto operator()(auto&& rng) -> bool;

private:
    f64 _p;
};

////////////////////////////////////////////////////////////

class beta_distribution {
public:
    beta_distribution(f64 alpha, f64 beta);

    auto operator()(auto&& rng) -> f64;

private:
    f64 _alpha;
    f64 _beta;
};

////////////////////////////////////////////////////////////

class binomial_distribution {
public:
    binomial_distribution(i32 trials, f64 p);

    auto operator()(auto&& rng) -> i32;

private:
    i32 _trials;
    f64 _p;
};

////////////////////////////////////////////////////////////

class cauchy_distribution {
public:
    cauchy_distribution(f64 x0, f64 gamma);

    auto operator()(auto&& rng) -> f64;

private:
    f64 _x0;
    f64 _gamma;
};

////////////////////////////////////////////////////////////

class discrete_distribution {
public:
    discrete_distribution(std::span<f64 const> probabilities);

    auto operator()(auto&& rng) -> i32;

private:
    std::vector<f64> _probs;
};

////////////////////////////////////////////////////////////

class exponential_distribution {
public:
    exponential_distribution(f64 lambda);

    auto operator()(auto&& rng) -> f64;

private:
    f64 _lambda;
};

////////////////////////////////////////////////////////////

class gamma_distribution {
public:
    gamma_distribution(f64 shape, f64 scale);

    auto operator()(auto&& rng) -> f64;

private:
    f64 _shape;
    f64 _scale;
};

////////////////////////////////////////////////////////////

class negative_binomial_distribution {
public:
    negative_binomial_distribution(i32 successes, f64 p);

    auto operator()(auto&& rng) -> i32;

private:
    i32 _successes;
    f64 _p;
};

////////////////////////////////////////////////////////////

class normal_distribution {
public:
    normal_distribution(f64 mean, f64 stdDev);

    auto operator()(auto&& rng) -> f64;

private:
    f64  _mean;
    f64  _stdDev;
    bool _toggle {false};
    f64  _x2 {0.0f};
};

////////////////////////////////////////////////////////////

class lognormal_distribution {
public:
    lognormal_distribution(f64 logScale, f64 shape);

    auto operator()(auto&& rng) -> f64;

private:
    normal_distribution _normal;
};

////////////////////////////////////////////////////////////

class pareto_distribution {
public:
    pareto_distribution(f64 alpha, f64 xm);

    auto operator()(auto&& rng) -> f64;

private:
    f64 _alpha;
    f64 _xm;
};

////////////////////////////////////////////////////////////

class piecewise_constant_distribution {
public:
    piecewise_constant_distribution(std::span<f64 const> intervals, std::span<f64 const> weights);

    auto operator()(auto&& rng) -> f64;

private:
    std::vector<f64> _intervals;
    std::vector<f64> _cumulativeWeights;
};

////////////////////////////////////////////////////////////

class poisson_distribution {
public:
    poisson_distribution(f64 mean);

    auto operator()(auto&& rng) -> i32;

private:
    f64 _mean;
};

////////////////////////////////////////////////////////////

class triangular_distribution {
public:
    triangular_distribution(f64 min, f64 max, f64 peak);

    auto operator()(auto&& rng) -> f64;

private:
    f64 _min;
    f64 _max;
    f64 _peak;
};

////////////////////////////////////////////////////////////

class weibull_distribution {
public:
    weibull_distribution(f64 shape, f64 scale);

    auto operator()(auto&& rng) -> f64;

private:
    f64 _shape;
    f64 _scale;
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
