// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <concepts>
#include <span>
#include <type_traits>
#include <vector>

#include "tcob/core/AngleUnits.hpp"
#include "tcob/core/Point.hpp"

namespace tcob::easing {
////////////////////////////////////////////////////////////

template <typename T>
concept Function =
    requires(T& t, f32 time) {
        typename T::type;

        { t.operator()(time) } -> std::same_as<typename T::type>;
    };

////////////////////////////////////////////////////////////

template <typename T>
concept Lerpable =
    requires(T const& t, f32 time) {
        { T::Lerp(t, t, time) } -> std::same_as<T>;
    };

////////////////////////////////////////////////////////////

template <typename T>
class curve final {
public:
    using type = T;

    struct point {
        f32  Position;
        type Value;
    };

    curve(std::span<point const> elements);

    auto operator()(f64 t) const -> type;

private:
    std::vector<point> _elements {};
};

////////////////////////////////////////////////////////////

template <typename T>
class power final {
public:
    using type = T;

    type Start {};
    type End {};
    f64  Exponent {1.0};

    auto operator()(f64 t) const -> type;
};

////////////////////////////////////////////////////////////

template <typename T>
class inverse_power final {
public:
    using type = T;

    type Start {};
    type End {};
    f64  Exponent {1.0};

    auto operator()(f64 t) const -> type;
};

////////////////////////////////////////////////////////////

template <typename T>
class inout_power final {
public:
    using type = T;

    type Start {};
    type End {};
    f64  Exponent {1.0};

    auto operator()(f64 t) const -> type;
};

////////////////////////////////////////////////////////////

template <typename T>
class exponential final {
public:
    using type = T;

    type Start {};
    type End {};

    auto operator()(f64 t) const -> type;
};

////////////////////////////////////////////////////////////

template <typename T>
class inverse_exponential final {
public:
    using type = T;

    type Start {};
    type End {};

    auto operator()(f64 t) const -> type;
};

////////////////////////////////////////////////////////////

template <typename T>
class inout_exponential final {
public:
    using type = T;

    type Start {};
    type End {};

    auto operator()(f64 t) const -> type;
};

////////////////////////////////////////////////////////////

template <typename T>
class linear final {
public:
    using type = T;

    type Start {};
    type End {};

    auto operator()(f64 t) const -> type;
};

////////////////////////////////////////////////////////////

class circular final {
public:
    using type = point_f;

    degree_f Start {};
    degree_f End {};

    auto operator()(f64 t) const -> type;
};

////////////////////////////////////////////////////////////

template <typename T>
class smoothstep final {
public:
    using type = T;

    type Start {};
    type End {};

    auto operator()(f64 t) const -> type;
};

////////////////////////////////////////////////////////////

template <typename T>
class smootherstep final {
public:
    using type = T;

    type Start {};
    type End {};

    auto operator()(f64 t) const -> type;
};

////////////////////////////////////////////////////////////

template <typename T>
class sine_wave final {
public:
    using type = T;

    type Min {};
    type Max {};
    f64  Frequency {1.0};
    f64  Phase {0.0};

    auto operator()(f64 t) const -> type;

private:
    auto get_wavevalue(f64 t) const -> f64;
};

////////////////////////////////////////////////////////////

template <typename T>
class triange_wave final {
public:
    using type = T;

    type Min {};
    type Max {};
    f64  Frequency {1.0};
    f64  Phase {0.0};

    auto operator()(f64 t) const -> type;

private:
    auto get_wavevalue(f64 t) const -> f64;
};

////////////////////////////////////////////////////////////

template <typename T>
class square_wave final {
public:
    using type = T;

    type Min {};
    type Max {};
    f64  Frequency {1.0};
    f64  Phase {0.0};

    auto operator()(f64 t) const -> type;

private:
    auto get_wavevalue(f64 t) const -> f64;
};

template <>
class square_wave<bool> final {
public:
    using type = bool;

    f64 Frequency {1.0};
    f64 Phase {0.0};

    auto operator()(f64 t) const -> type;

private:
    auto get_wavevalue(f64 t) const -> f64;
};

////////////////////////////////////////////////////////////

template <typename T>
class sawtooth_wave final {
public:
    using type = T;

    type Min {};
    type Max {};
    f64  Frequency {1.0};
    f64  Phase {0.0};

    auto operator()(f64 t) const -> type;

private:
    auto get_wavevalue(f64 t) const -> f64;
};

////////////////////////////////////////////////////////////

class quad_bezier_curve final {
public:
    using type = point_f;

    type StartPoint {};
    type ControlPoint {};
    type EndPoint {};

    auto operator()(f64 t) const -> type;
};

////////////////////////////////////////////////////////////

class cubic_bezier_curve final {
public:
    using type = point_f;

    type StartPoint {};
    type ControlPoint0 {};
    type ControlPoint1 {};
    type EndPoint {};

    auto operator()(f64 t) const -> type;
};

////////////////////////////////////////////////////////////

class bezier_curve final {
public:
    using type = point_f;

    std::vector<type> ControlPoints;

    auto operator()(f64 t) const -> type;
};

////////////////////////////////////////////////////////////

class catmull_rom final {
public:
    using type = point_f;

    std::vector<type> ControlPoints;

    auto operator()(f64 t) const -> type;
};

////////////////////////////////////////////////////////////

template <auto Func>
class function final {
public:
    using type = std::invoke_result_t<decltype(Func), f64>;

    auto operator()(f64 t) const -> type;
};

////////////////////////////////////////////////////////////

template <typename T>
class callable final {
public:
    using type = std::invoke_result_t<decltype(&T::operator()), T, f64>;

    callable(T obj);

    auto operator()(f64 t) const -> type;

private:
    T _obj;
};

////////////////////////////////////////////////////////////

}

#include "Easing.inl"
