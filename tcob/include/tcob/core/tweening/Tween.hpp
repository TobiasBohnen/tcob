// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <concepts>
#include <memory>
#include <optional>
#include <queue>

#include "tcob/core/Common.hpp"
#include "tcob/core/Interfaces.hpp"
#include "tcob/core/Property.hpp"
#include "tcob/core/Signal.hpp"
#include "tcob/core/tweening/TweenFunc.hpp"

namespace tcob {
////////////////////////////////////////////////////////////

class TCOB_API tween_base : public updatable, public non_copyable {
public:
    explicit tween_base(milliseconds duration);
    ~tween_base() override;

    signal<void, tween_base> Finished;

    std::optional<milliseconds> Interval {};

    auto progress() const -> f64;
    auto playback_progress() const -> f64;

    auto state() const -> playback_state;
    auto is_looping() const -> bool;

    void start(playback_mode mode = playback_mode::Once);
    void stop();
    void restart();

    void pause();
    void resume();
    void toggle_pause();

private:
    void on_update(milliseconds deltaTime) final;

    virtual void update_values() = 0;

    milliseconds _duration {0};
    milliseconds _elapsedTime {0};
    milliseconds _currentInterval {0};

    playback_state _state {playback_state::Stopped};
    playback_mode  _mode {};
};

////////////////////////////////////////////////////////////

template <tween_funcs::Function Func>
class tween final : public tween_base {
public:
    using func_type  = Func;
    using value_type = typename Func::type;

    tween(milliseconds duration);
    tween(milliseconds duration, func_type&& func);

    prop<value_type> Value;
    func_type        Function;

    auto add_output(value_type* dest) -> connection;

private:
    void update_values() override;
};

////////////////////////////////////////////////////////////

class TCOB_API tween_queue final : public updatable {
public:
    auto is_empty() const -> bool;

    void push(auto&&... autom);
    void pop();

    void start(playback_mode mode = playback_mode::Once);
    void stop_and_clear();

private:
    void on_update(milliseconds deltaTime) override;

    std::queue<std::shared_ptr<tween_base>> _queue {};
    bool                                    _isRunning {false};
    bool                                    _isLooping {false};
    playback_mode                           _mode {};
};

////////////////////////////////////////////////////////////

template <typename T, tween_funcs::ease_mode Mode = tween_funcs::ease_mode::In>
using polynomial_tween = tween<tween_funcs::polynomial<T, Mode>>;

template <typename T, tween_funcs::ease_mode Mode = tween_funcs::ease_mode::In>
using exponential_tween = tween<tween_funcs::exponential<T, Mode>>;

template <typename T>
using linear_tween = tween<tween_funcs::linear<T>>;

template <typename T>
using smoothstep_tween = tween<tween_funcs::smoothstep<T>>;

template <typename T>
using smootherstep_tween = tween<tween_funcs::smootherstep<T>>;

template <typename T, tween_funcs::ease_mode Mode = tween_funcs::ease_mode::Out>
using bounce_tween = tween<tween_funcs::bounce<T, Mode>>;

template <typename T, tween_funcs::ease_mode Mode = tween_funcs::ease_mode::Out>
using elastic_tween = tween<tween_funcs::elastic<T, Mode>>;

template <typename T, tween_funcs::ease_mode Mode = tween_funcs::ease_mode::Out>
using back_tween = tween<tween_funcs::back<T, Mode>>;

template <typename T, tween_funcs::ease_mode Mode = tween_funcs::ease_mode::Out>
using circular_tween = tween<tween_funcs::circular<T, Mode>>;

template <typename T>
using sine_wave_tween = tween<tween_funcs::sine_wave<T>>;

template <typename T>
using triangle_wave_tween = tween<tween_funcs::triangle_wave<T>>;

template <typename T>
using square_wave_tween = tween<tween_funcs::square_wave<T>>;

template <typename T>
using sawtooth_wave_tween = tween<tween_funcs::sawtooth_wave<T>>;

using quad_bezier_curve_tween = tween<tween_funcs::quad_bezier_curve>;

using cubic_bezier_curve_tween = tween<tween_funcs::cubic_bezier_curve>;

using bezier_curve_tween = tween<tween_funcs::bezier_curve>;

using catmull_rom_tween = tween<tween_funcs::catmull_rom>;

using circular_motion_tween = tween<tween_funcs::circular_motion>;

template <typename T>
using curve_tween = tween<tween_funcs::curve<T>>;
template <typename T>
using curve_point = tween_funcs::curve<T>::point;

template <typename T>
using callable_tween = tween<tween_funcs::callable<T>>;

}

#include "Tween.inl"
