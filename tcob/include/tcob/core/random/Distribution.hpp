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

    template <typename R>
    auto random01(R& rng) -> f32;

    template <typename R>
    auto static Random01(R& rng) -> f32;
};

////////////////////////////////////////////////////////////

class bernoulli_distribution {
public:
    bernoulli_distribution(f32 p);

    template <typename R>
    auto operator()(R& rng) -> bool;

private:
    f32 _p;
};

////////////////////////////////////////////////////////////

class beta_distribution {
public:
    beta_distribution(f32 a, f32 b);

    template <typename R>
    auto operator()(R& rng) -> f32;

private:
    f32 _a;
    f32 _b;
};

////////////////////////////////////////////////////////////

class cauchy_distribution {
public:
    cauchy_distribution(f32 x0, f32 gamma);

    template <typename R>
    auto operator()(R& rng) -> f32;

private:
    f32 _x0;
    f32 _gamma;
};

////////////////////////////////////////////////////////////

class discrete_distribution {
public:
    discrete_distribution(std::span<f32> probabilities);

    template <typename R>
    auto operator()(R& rng) -> i32;

private:
    std::vector<f32> _probs;
};

////////////////////////////////////////////////////////////

class exponential_distribution {
public:
    exponential_distribution(f32 lambda);

    template <typename R>
    auto operator()(R& rng) -> f32;

private:
    f32 _lambda;
};

////////////////////////////////////////////////////////////

class gamma_distribution {
public:
    gamma_distribution(f32 shape);

    template <typename R>
    auto operator()(R& rng) -> f32;

private:
    f32 _shape;
};

////////////////////////////////////////////////////////////

class normal_distribution {
public:
    normal_distribution(f32 mean, f32 stdDev);

    template <typename R>
    auto operator()(R& rng) -> f32;

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

    template <typename R>
    auto operator()(R& rng) -> f32;

private:
    f32 _alpha;
    f32 _xm;
};

////////////////////////////////////////////////////////////

class poisson_distribution {
public:
    poisson_distribution(f32 mean);

    template <typename R>
    auto operator()(R& rng) -> i32;

private:
    f32 _mean;
};

////////////////////////////////////////////////////////////

class triangular_distribution {
public:
    triangular_distribution(f32 min, f32 max, f32 peak);

    template <typename R>
    auto operator()(R& rng) -> f32;

private:
    f32 _min;
    f32 _max;
    f32 _peak;
};

////////////////////////////////////////////////////////////

class weibull_distribution {
public:
    weibull_distribution(f32 shape, f32 scale);

    template <typename R>
    auto operator()(R& rng) -> f32;

private:
    f32 _shape;
    f32 _scale;
};

////////////////////////////////////////////////////////////

class bag_distribution {
public:
    bag_distribution(i64 min, i64 max, isize period);

    template <typename R>
    auto operator()(R& rng) -> i64;

private:
    template <typename R>
    void gen_seq(R& rng);

    i64              _min;
    i64              _max;
    isize            _period;
    std::vector<i64> _seq;
};

}

#include "Distribution.inl"
