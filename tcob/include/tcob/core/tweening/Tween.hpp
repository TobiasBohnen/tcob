// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <concepts>
#include <queue>
#include <span>
#include <type_traits>
#include <vector>

#include "tcob/core/AngleUnits.hpp"
#include "tcob/core/Common.hpp"
#include "tcob/core/Interfaces.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Property.hpp"
#include "tcob/core/Signal.hpp"

namespace tcob::tweening {
////////////////////////////////////////////////////////////

template <typename T>
concept TweenFunction =
    requires(T& t, f32 time) {
        typename T::type;

        {
            t.operator()(time)
        } -> std::same_as<typename T::type>;
    };

////////////////////////////////////////////////////////////

template <typename T>
concept Lerpable =
    requires(T const& t, f32 time) {
        {
            T::Lerp(t, t, time)
        } -> std::same_as<T>;
    };

////////////////////////////////////////////////////////////

class TCOB_API tween_base : public updatable {
public:
    explicit tween_base(milliseconds duration);
    tween_base(tween_base const& other) noexcept                    = delete;
    auto operator=(tween_base const& other) noexcept -> tween_base& = delete;
    tween_base(tween_base&& other) noexcept                         = delete;
    auto operator=(tween_base&& other) noexcept -> tween_base&      = delete;
    ~tween_base() override;

    std::optional<milliseconds> Interval {};
    signal<>                    Finished;

    auto get_progress() const -> f64;
    auto get_status() const -> playback_status;
    auto get_mode() const -> playback_mode;
    auto is_looping() const -> bool;

    void start(playback_mode mode = playback_mode::Normal);
    void stop();
    void restart();

    void pause();
    void resume();
    void toggle_pause();

private:
    void on_update(milliseconds deltaTime) final;

    void virtual update_values() = 0;

    milliseconds _duration {0};
    milliseconds _elapsedTime {0};
    milliseconds _currentInterval {0};

    playback_status _status {playback_status::Stopped};
    playback_mode   _mode {};
};

////////////////////////////////////////////////////////////

template <TweenFunction Func>
class tween final : public tween_base {
public:
    using func_type  = Func;
    using value_type = typename Func::type;

    tween(milliseconds duration, func_type&& ptr);

    prop<value_type> Value;

    auto add_output(value_type* dest) -> connection;

private:
    void update_values() override;

    func_type _function;
};

////////////////////////////////////////////////////////////

class TCOB_API queue final : public updatable {
public:
    auto is_empty() const -> bool;

    void push(auto&&... autom);
    void pop();

    void start(playback_mode mode = playback_mode::Normal);
    void stop();

private:
    void on_update(milliseconds deltaTime) override;

    std::queue<std::shared_ptr<tween_base>> _queue {};
    bool                                    _isRunning {false};
    bool                                    _isLooping {false};
    playback_mode                           _mode {};
};

////////////////////////////////////////////////////////////

namespace func {

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

        type StartValue {};
        type EndValue {};
        f64  Exponent {1.0};

        auto operator()(f64 t) const -> type;
    };

    ////////////////////////////////////////////////////////////

    template <typename T>
    class inverse_power final {
    public:
        using type = T;

        type StartValue {};
        type EndValue {};
        f64  Exponent {1.0};

        auto operator()(f64 t) const -> type;
    };

    ////////////////////////////////////////////////////////////

    template <typename T>
    class linear final {
    public:
        using type = T;

        type StartValue {};
        type EndValue {};

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

        type Edge0 {};
        type Edge1 {};

        auto operator()(f64 t) const -> type;
    };

    ////////////////////////////////////////////////////////////

    template <typename T>
    class smootherstep final {
    public:
        using type = T;

        type Edge0 {};
        type Edge1 {};

        auto operator()(f64 t) const -> type;
    };

    ////////////////////////////////////////////////////////////

    template <typename T>
    class sine_wave final {
    public:
        using type = T;

        type MinValue {};
        type MaxValue {};
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

        type MinValue {};
        type MaxValue {};
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

        type MinValue {};
        type MaxValue {};
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

        type MinValue {};
        type MaxValue {};
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

        type Begin {};
        type ControlPoint {};
        type End {};

        auto operator()(f64 t) const -> type;
    };

    ////////////////////////////////////////////////////////////

    class cubic_bezier_curve final {
    public:
        using type = point_f;

        type Begin {};
        type ControlPoint0 {};
        type ControlPoint1 {};
        type End {};

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

        callable(T& obj);

        auto operator()(f64 t) const -> type;

    private:
        T& _obj;
    };

    ////////////////////////////////////////////////////////////

}

template <typename T>
using curve_tween = tween<func::curve<T>>;
template <typename T>
using curve_point = func::curve<T>::point;

template <typename T>
using power_tween = tween<func::power<T>>;

template <typename T>
using inverse_power_tween = tween<func::inverse_power<T>>;

template <typename T>
using linear_tween = tween<func::linear<T>>;

using circular_tween = tween<func::circular>;

template <typename T>
using smoothstep_tween = tween<func::smoothstep<T>>;

template <typename T>
using smootherstep_tween = tween<func::smootherstep<T>>;

template <typename T>
using sine_wave_tween = tween<func::sine_wave<T>>;

template <typename T>
using triange_wave_tween = tween<func::triange_wave<T>>;

template <typename T>
using square_wave_tween = tween<func::square_wave<T>>;

template <typename T>
using sawtooth_wave_tween = tween<func::sawtooth_wave<T>>;

using quad_bezier_curve_tween = tween<func::quad_bezier_curve>;

using cubic_bezier_curve_tween = tween<func::cubic_bezier_curve>;

using bezier_curve_tween = tween<func::bezier_curve>;

using catmull_rom_tween = tween<func::catmull_rom>;

template <auto Func>
using function_tween = tween<func::function<Func>>;

template <typename T>
using callable_tween = tween<func::callable<T>>;

}

#include "Tween.inl"
