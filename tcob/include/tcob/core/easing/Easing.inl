// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Easing.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <span>
#include <vector>

#include "tcob/core/AngleUnits.hpp"
#include "tcob/core/Point.hpp"

namespace tcob::easing {

template <typename T>
inline curve<T>::curve(std::span<point const> elements)
    : _elements {elements.begin(), elements.end()}
{
    // TODO: ensure positions between 0 and 1; and sorted
}

template <typename T>
inline auto curve<T>::operator()(f64 t) const -> type
{
    usize const size {_elements.size()};
    if (size == 0) { return T {}; }
    if (size == 1) { return _elements.back().Value; }

    usize index {0};
    for (usize i {1}; i < size; ++i) {
        if (_elements[i].Position > t) { break; }
        index = i;
    }
    if (index == size - 1) { return _elements.back().Value; }

    point const& current {_elements[index]};
    point const& next {_elements[index + 1]};
    f64 const    pos {(t - current.Position) / (next.Position - current.Position)};

    if constexpr (Lerpable<type>) {
        return type::Lerp(current.Value, next.Value, pos);
    } else {
        return static_cast<type>(current.Value + ((next.Value - current.Value) * pos));
    }
}

////////////////////////////////////////////////////////////

template <typename T>
inline auto power<T>::operator()(f64 t) const -> type
{
    if (Exponent <= 0 && t == 0) { return Start; }

    if constexpr (Lerpable<type>) {
        return type::Lerp(Start, End, std::pow(t, Exponent));
    } else {
        return static_cast<type>(Start + ((End - Start) * std::pow(t, Exponent)));
    }
}

////////////////////////////////////////////////////////////

template <typename T>
inline auto inverse_power<T>::operator()(f64 t) const -> type
{
    if (Exponent <= 0 && t == 0) { return Start; }

    if constexpr (Lerpable<type>) {
        return type::Lerp(Start, End, 1 - std::pow(1 - t, Exponent));
    } else {
        return static_cast<type>(Start + ((End - Start) * (1 - std::pow(1 - t, Exponent))));
    }
}

////////////////////////////////////////////////////////////

template <typename T>
inline auto inout_power<T>::operator()(f64 t) const -> type
{
    if (Exponent <= 0 && t == 0) { return Start; }

    auto const midpoint {Start + (End - Start) * 0.5};

    if (t < 0.5) {
        f64 const scaledT {t * 2.0};
        if constexpr (Lerpable<type>) {
            return type::Lerp(Start, midpoint, std::pow(scaledT, Exponent));
        } else {
            return static_cast<type>(Start + ((midpoint - Start) * std::pow(scaledT, Exponent)));
        }
    } else {
        f64 const scaledT {(t - 0.5) * 2.0};
        if constexpr (Lerpable<type>) {
            return type::Lerp(midpoint, End, 1 - std::pow(1 - scaledT, Exponent));
        } else {
            return static_cast<type>(midpoint + ((End - midpoint) * (1 - std::pow(1 - scaledT, Exponent))));
        }
    }
}

////////////////////////////////////////////////////////////

template <typename T>
inline auto exponential<T>::operator()(f64 t) const -> type
{
    if (t <= 0.0) { return Start; }

    if constexpr (Lerpable<type>) {
        return type::Lerp(Start, End, std::pow(2.0, 10.0 * (t - 1.0)));
    } else {
        return static_cast<type>(Start + (End - Start) * std::pow(2.0, 10.0 * (t - 1.0)));
    }
}

////////////////////////////////////////////////////////////

template <typename T>
inline auto inverse_exponential<T>::operator()(f64 t) const -> type
{
    if (t >= 1.0) { return End; }

    if constexpr (Lerpable<type>) {
        return type::Lerp(Start, End, 1.0 - std::pow(2.0, -10.0 * t));
    } else {
        return static_cast<type>(Start + (End - Start) * (1.0 - std::pow(2.0, -10.0 * t)));
    }
}

////////////////////////////////////////////////////////////

template <typename T>
inline auto inout_exponential<T>::operator()(f64 t) const -> type
{
    if (t <= 0.0) { return Start; }
    if (t >= 1.0) { return End; }

    f64 const factor {t < 0.5
                          ? std::pow(2.0, (20.0 * t) - 10.0) / 2.0
                          : (2.0 - std::pow(2.0, (-20.0 * t) + 10.0)) / 2.0};
    if constexpr (Lerpable<type>) {
        return type::Lerp(Start, End, factor);
    } else {
        return static_cast<type>(Start + (End - Start) * factor);
    }
}

////////////////////////////////////////////////////////////

template <typename T>
inline auto linear<T>::operator()(f64 t) const -> type
{
    if (t == 0) { return Start; }

    if constexpr (Lerpable<type>) {
        return type::Lerp(Start, End, t);
    } else {
        return static_cast<type>(Start + static_cast<f64>((End - Start) * t));
    }
}

////////////////////////////////////////////////////////////

inline auto circular::operator()(f64 t) const -> type
{
    return point_f::FromDirection(degree_f::Lerp(Start, End, t));
}

////////////////////////////////////////////////////////////

template <typename T>
inline auto smoothstep<T>::operator()(f64 t) const -> type
{
    if (t == 0) { return Start; }

    f64 const e {t * t * (3. - 2. * t)};
    if constexpr (Lerpable<type>) {
        return type::Lerp(Start, End, e);
    } else {
        return static_cast<type>(Start + static_cast<f64>((End - Start) * e));
    }
}

template <typename T>
inline auto smootherstep<T>::operator()(f64 t) const -> type
{
    if (t == 0) { return Start; }

    f64 const e {t * t * t * (t * (t * 6. - 15.) + 10.)};
    if constexpr (Lerpable<type>) {
        return type::Lerp(Start, End, e);
    } else {
        return static_cast<type>(Start + static_cast<f64>((End - Start) * e));
    }
}

////////////////////////////////////////////////////////////

template <typename T>
inline auto sine_wave<T>::operator()(f64 t) const -> type
{
    f64 const val {get_wavevalue(Frequency * t)};
    if constexpr (Lerpable<type>) {
        return type::Lerp(Min, Max, val);
    } else {
        return static_cast<type>(Min + static_cast<f64>((Max - Min) * val));
    }
}

template <typename T>
inline auto sine_wave<T>::get_wavevalue(f64 t) const -> f64
{
    return (std::sin((TAU * t) + (0.75 * TAU) + Phase) + 1.) / 2.;
}

////////////////////////////////////////////////////////////

template <typename T>
inline auto triangle_wave<T>::operator()(f64 t) const -> type
{
    f64 const val {get_wavevalue((Frequency * t) + Phase)};
    if constexpr (Lerpable<type>) {
        return type::Lerp(Min, Max, val);
    } else {
        return static_cast<type>(Min + static_cast<f64>((Max - Min) * val));
    }
}

template <typename T>
inline auto triangle_wave<T>::get_wavevalue(f64 t) const -> f64
{
    return 2 * std::abs(std::round(t) - t);
}

////////////////////////////////////////////////////////////

template <typename T>
inline auto square_wave<T>::operator()(f64 t) const -> type
{
    f64 const val {get_wavevalue(Frequency * t)};
    if constexpr (Lerpable<type>) {
        return type::Lerp(Min, Max, val);
    } else {
        return static_cast<type>(Min + static_cast<f64>((Max - Min) * val));
    }
}

template <typename T>
inline auto square_wave<T>::get_wavevalue(f64 t) const -> f64
{
    f64 const x {std::round(t + Phase) / 2};
    return 2. * (x - std::floor(x));
}

////////////////////////////////////////////////////////////

inline auto square_wave<bool>::operator()(f64 t) const -> type
{
    return get_wavevalue(Frequency * t) < 0.5f;
}

inline auto square_wave<bool>::get_wavevalue(f64 t) const -> f64
{
    f64 const x {std::round(t + Phase) / 2};
    return 2. * (x - std::floor(x));
}

////////////////////////////////////////////////////////////

template <typename T>
inline auto sawtooth_wave<T>::operator()(f64 t) const -> type
{
    f64 const val {get_wavevalue((Frequency * t) + Phase)};
    if constexpr (Lerpable<type>) {
        return type::Lerp(Min, Max, val);
    } else {
        return static_cast<type>(Min + static_cast<f64>((Max - Min) * val));
    }
}

template <typename T>
inline auto sawtooth_wave<T>::get_wavevalue(f64 t) const -> f64
{
    return t - std::floor(t);
}

////////////////////////////////////////////////////////////

inline auto quad_bezier_curve::operator()(f64 t) const -> type
{
    f64 const oneMinusT {1. - t};

    f64 const exp0 {oneMinusT * oneMinusT};
    f64 const exp1 {2. * t * oneMinusT};
    f64 const exp2 {t * t};

    f32 const x {static_cast<f32>((exp0 * StartPoint.X) + (exp1 * ControlPoint.X) + (exp2 * EndPoint.X))};
    f32 const y {static_cast<f32>((exp0 * StartPoint.Y) + (exp1 * ControlPoint.Y) + (exp2 * EndPoint.Y))};
    return {x, y};
}

////////////////////////////////////////////////////////////

inline auto cubic_bezier_curve::operator()(f64 t) const -> type
{
    f64 const oneMinusT {1. - t};

    f64 const exp0 {oneMinusT * oneMinusT * oneMinusT};
    f64 const exp1 {3 * t * oneMinusT * oneMinusT};
    f64 const exp2 {3 * t * t * oneMinusT};
    f64 const exp3 {t * t * t};

    f32 const x {static_cast<f32>((exp0 * StartPoint.X) + (exp1 * ControlPoint0.X) + (exp2 * ControlPoint1.X) + (exp3 * EndPoint.X))};
    f32 const y {static_cast<f32>((exp0 * StartPoint.Y) + (exp1 * ControlPoint0.Y) + (exp2 * ControlPoint1.Y) + (exp3 * EndPoint.Y))};
    return {x, y};
}

////////////////////////////////////////////////////////////

inline auto bezier_curve::operator()(f64 t) const -> type
{
    std::vector<type> points {ControlPoints};
    usize             numPoints(ControlPoints.size());
    f32 const         oneMinusT {1.0f - static_cast<f32>(t)};

    while (numPoints > 1) {
        for (usize i {0}; i < numPoints - 1; ++i) {
            points[i] = (points[i] * oneMinusT) + (points[i + 1] * t);
        }
        --numPoints;
    }

    return points[0];
}

////////////////////////////////////////////////////////////

inline auto catmull_rom::operator()(f64 t) const -> type
{
    if (ControlPoints.size() < 4) { return {}; }

    f64 const curveP {t * (ControlPoints.size() - 1)};
    i32 const curveNum {static_cast<i32>(curveP)};

    i32 const b {curveNum};
    i32 const a {b > 0 ? b - 1 : 0};
    i32 const c {b + 1};
    i32 const d {std::min<i32>(c + 1, static_cast<i32>(ControlPoints.size()) - 1)};

    type const& p0 {ControlPoints[static_cast<u32>(a)]};
    type const& p1 {ControlPoints[static_cast<u32>(b)]};
    type const& p2 {ControlPoints[static_cast<u32>(c)]};
    type const& p3 {ControlPoints[static_cast<u32>(d)]};

    f64 const exp0 {curveP - curveNum};
    f64 const exp1 {exp0 * exp0};
    f64 const exp2 {exp1 * exp0};

    f32 const x {static_cast<f32>(0.5 * ((2 * p1.X) + (-p0.X + p2.X) * exp0 + (2 * p0.X - 5 * p1.X + 4 * p2.X - p3.X) * exp1 + (-p0.X + 3 * p1.X - 3 * p2.X + p3.X) * exp2))};
    f32 const y {static_cast<f32>(0.5 * ((2 * p1.Y) + (-p0.Y + p2.Y) * exp0 + (2 * p0.Y - 5 * p1.Y + 4 * p2.Y - p3.Y) * exp1 + (-p0.Y + 3 * p1.Y - 3 * p2.Y + p3.Y) * exp2))};
    return {x, y};
}

////////////////////////////////////////////////////////////

template <auto Func>
inline auto function<Func>::operator()(f64 t) const -> type
{
    return Func(t);
}

////////////////////////////////////////////////////////////

template <typename T>
inline callable<T>::callable(T obj)
    : _obj {std::move(obj)}
{
}

template <typename T>
inline auto callable<T>::operator()(f64 t) const -> type
{
    return _obj(t);
}
}
