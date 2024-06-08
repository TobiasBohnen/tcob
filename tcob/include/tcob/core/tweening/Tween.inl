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

namespace funcs {

    template <typename T>
    inline linear_chain<T>::linear_chain(std::span<type const> elements)
        : _elements {elements.begin(), elements.end()}
    {
    }

    template <typename T>
    inline auto linear_chain<T>::operator()(f64 t) const -> type
    {
        if (_elements.empty()) {
            return T {};
        }

        usize const size {_elements.size() - 1};
        f64 const   relElapsed {size * t};
        usize const index {static_cast<usize>(relElapsed)};
        if (index >= size) {
            return _elements[index];
        }

        type const& current {_elements[index]};
        type const& next {_elements[index + 1]};

        if constexpr (Lerpable<type>) {
            return type::Lerp(current, next, relElapsed - static_cast<f64>(index));
        } else {
            return static_cast<type>(current + ((next - current) * (relElapsed - static_cast<f64>(index))));
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

        f32 const x {static_cast<f32>((exp0 * Start.X) + (exp1 * ControlPoint.X) + (exp2 * End.X))};
        f32 const y {static_cast<f32>((exp0 * Start.Y) + (exp1 * ControlPoint.Y) + (exp2 * End.Y))};
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

        f32 const x {static_cast<f32>((exp0 * Start.X) + (exp1 * ControlPoint0.X) + (exp2 * ControlPoint1.X) + (exp3 * End.X))};
        f32 const y {static_cast<f32>((exp0 * Start.Y) + (exp1 * ControlPoint0.Y) + (exp2 * ControlPoint1.Y) + (exp3 * End.Y))};
        return {x, y};
    }

    ////////////////////////////////////////////////////////////

    inline auto bezier_curve::operator()(f64 t) const -> type
    {
        std::vector<point_f> points {ControlPoints};
        usize                numPoints(ControlPoints.size());
        f32 const            oneMinusT {1.0f - static_cast<f32>(t)};

        while (numPoints > 1) {
            for (usize i {0}; i < numPoints - 1; ++i) {
                points[i] = (points[i] * oneMinusT) + (points[i + 1] * t);
            }
            --numPoints;
        }

        return points[0];
    }

    ////////////////////////////////////////////////////////////

    template <typename T>
    inline auto random<T>::operator()(f64) const -> type
    {
        return RNG(MinValue, MaxValue);
    }

    ////////////////////////////////////////////////////////////

    template <auto Func>
    inline auto function<Func>::operator()(f64 t) const -> type
    {
        return Func(t);
    }

    ////////////////////////////////////////////////////////////

    template <typename T>
    inline callable<T>::callable(T& obj)
        : _obj {obj}
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
