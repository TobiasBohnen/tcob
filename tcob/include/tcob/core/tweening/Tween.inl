// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Tween.hpp"

namespace tcob::tweening {

template <TweenFunction Func>
inline tween<Func>::tween(milliseconds duration, func_type&& ptr)
    : tween_base {duration}
    , _function {std::move(ptr)}
{
}

template <TweenFunction Func>
inline auto tween<Func>::add_output(value_type* dest) -> connection
{
    return Value.Changed.connect([dest](value_type const& val) { *dest = val; });
}

template <TweenFunction Func>
inline void tween<Func>::update_values()
{
    Value = _function(get_progress());
}

////////////////////////////////////////////////////////////

template <typename Tween>
auto make_unique_tween(milliseconds duration, auto&&... args) -> std::unique_ptr<Tween>
{
    return std::unique_ptr<Tween>(new Tween {duration, typename Tween::func_type {args...}});
}

template <typename Tween>
auto make_shared_tween(milliseconds duration, auto&&... args) -> std::shared_ptr<Tween>
{
    return std::shared_ptr<Tween>(new Tween {duration, typename Tween::func_type {args...}});
}

////////////////////////////////////////////////////////////

inline void queue::push(auto&&... autom)
{
    (_queue.push(autom), ...);
}

////////////////////////////////////////////////////////////

namespace func {

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
        if (Exponent <= 0 && t == 0) {
            return StartValue;
        }

        if constexpr (Lerpable<type>) {
            return type::Lerp(StartValue, EndValue, std::pow(t, Exponent));
        } else {
            return static_cast<type>(StartValue + ((EndValue - StartValue) * std::pow(t, Exponent)));
        }
    }

    ////////////////////////////////////////////////////////////

    template <typename T>
    inline auto inverse_power<T>::operator()(f64 t) const -> type
    {
        if (Exponent <= 0 && t == 0) {
            return StartValue;
        }

        if constexpr (Lerpable<type>) {
            return type::Lerp(StartValue, EndValue, 1 - std::pow(1 - t, Exponent));
        } else {
            return static_cast<type>(StartValue + ((EndValue - StartValue) * (1 - std::pow(1 - t, Exponent))));
        }
    }

    ////////////////////////////////////////////////////////////

    template <typename T>
    inline auto linear<T>::operator()(f64 t) const -> type
    {
        if (t == 0) {
            return StartValue;
        }

        if constexpr (Lerpable<type>) {
            return type::Lerp(StartValue, EndValue, t);
        } else {
            return static_cast<type>(StartValue + static_cast<f64>((EndValue - StartValue) * t));
        }
    }

    ////////////////////////////////////////////////////////////

    inline auto circular::operator()(f64 t) const -> type
    {
        degree_f const angle {degree_f::Lerp(Start, End, t)};
        return {angle.cos(), angle.sin()};
    }

    ////////////////////////////////////////////////////////////

    template <typename T>
    inline auto smoothstep<T>::operator()(f64 t) const -> type
    {
        if (t == 0) {
            return Edge0;
        }

        f64 const e {t * t * (3. - 2. * t)};
        if constexpr (Lerpable<type>) {
            return type::Lerp(Edge0, Edge1, e);
        } else {
            return static_cast<type>(Edge0 + static_cast<f64>((Edge1 - Edge0) * e));
        }
    }

    template <typename T>
    inline auto smootherstep<T>::operator()(f64 t) const -> type
    {
        if (t == 0) {
            return Edge0;
        }

        f64 const e {t * t * t * (t * (t * 6. - 15.) + 10.)};
        if constexpr (Lerpable<type>) {
            return type::Lerp(Edge0, Edge1, e);
        } else {
            return static_cast<type>(Edge0 + static_cast<f64>((Edge1 - Edge0) * e));
        }
    }

    ////////////////////////////////////////////////////////////

    template <typename T>
    inline auto sine_wave<T>::operator()(f64 t) const -> type
    {
        f64 const val {get_wavevalue(Frequency * t)};
        if constexpr (Lerpable<type>) {
            return type::Lerp(MinValue, MaxValue, val);
        } else {
            return static_cast<type>(MinValue + static_cast<f64>((MaxValue - MinValue) * val));
        }
    }

    template <typename T>
    inline auto sine_wave<T>::get_wavevalue(f64 t) const -> f64
    {
        return (std::sin((TAU * t) + (0.75 * TAU) + Phase) + 1.) / 2.;
    }

    ////////////////////////////////////////////////////////////

    template <typename T>
    inline auto triange_wave<T>::operator()(f64 t) const -> type
    {
        f64 const val {get_wavevalue((Frequency * t) + Phase)};
        if constexpr (Lerpable<type>) {
            return type::Lerp(MinValue, MaxValue, val);
        } else {
            return static_cast<type>(MinValue + static_cast<f64>((MaxValue - MinValue) * val));
        }
    }

    template <typename T>
    inline auto triange_wave<T>::get_wavevalue(f64 t) const -> f64
    {
        return 2 * std::abs(std::round(t) - t);
    }

    ////////////////////////////////////////////////////////////

    template <typename T>
    inline auto square_wave<T>::operator()(f64 t) const -> type
    {
        f64 const val {get_wavevalue(Frequency * t)};
        if constexpr (Lerpable<type>) {
            return type::Lerp(MinValue, MaxValue, val);
        } else {
            return static_cast<type>(MinValue + static_cast<f64>((MaxValue - MinValue) * val));
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
            return type::Lerp(MinValue, MaxValue, val);
        } else {
            return static_cast<type>(MinValue + static_cast<f64>((MaxValue - MinValue) * val));
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

        f32 const x {static_cast<f32>((exp0 * Begin.X) + (exp1 * ControlPoint.X) + (exp2 * End.X))};
        f32 const y {static_cast<f32>((exp0 * Begin.Y) + (exp1 * ControlPoint.Y) + (exp2 * End.Y))};
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

        f32 const x {static_cast<f32>((exp0 * Begin.X) + (exp1 * ControlPoint0.X) + (exp2 * ControlPoint1.X) + (exp3 * End.X))};
        f32 const y {static_cast<f32>((exp0 * Begin.Y) + (exp1 * ControlPoint0.Y) + (exp2 * ControlPoint1.Y) + (exp3 * End.Y))};
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

////////////////////////////////////////////////////////////
}
