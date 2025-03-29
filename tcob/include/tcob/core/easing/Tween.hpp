// Copyright (c) 2025 Tobias Bohnen
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
#include "tcob/core/easing/Easing.hpp"

namespace tcob {
////////////////////////////////////////////////////////////

class TCOB_API tween_base : public updatable {
public:
    explicit tween_base(milliseconds duration);
    tween_base(tween_base const& other) noexcept                    = delete;
    auto operator=(tween_base const& other) noexcept -> tween_base& = delete;
    tween_base(tween_base&& other) noexcept                         = delete;
    auto operator=(tween_base&& other) noexcept -> tween_base&      = delete;
    ~tween_base() override;

    signal<> Finished;

    std::optional<milliseconds> Interval {};

    auto progress() const -> f64;
    auto state() const -> playback_state;
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

    playback_state _state {playback_state::Stopped};
    playback_mode  _mode {};
};

////////////////////////////////////////////////////////////

template <easing::Function Func>
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

    void start(playback_mode mode = playback_mode::Normal);
    void stop();

private:
    void on_update(milliseconds deltaTime) override;

    std::queue<std::shared_ptr<tween_base>> _queue {};
    bool                                    _isRunning {false};
    bool                                    _isLooping {false};
    playback_mode                           _mode {};
};

template <typename T>
using curve_tween = tween<easing::curve<T>>;
template <typename T>
using curve_point = easing::curve<T>::point;

template <typename T>
using power_tween = tween<easing::power<T>>;

template <typename T>
using inverse_power_tween = tween<easing::inverse_power<T>>;

template <typename T>
using linear_tween = tween<easing::linear<T>>;

using circular_tween = tween<easing::circular>;

template <typename T>
using smoothstep_tween = tween<easing::smoothstep<T>>;

template <typename T>
using smootherstep_tween = tween<easing::smootherstep<T>>;

template <typename T>
using sine_wave_tween = tween<easing::sine_wave<T>>;

template <typename T>
using triange_wave_tween = tween<easing::triange_wave<T>>;

template <typename T>
using square_wave_tween = tween<easing::square_wave<T>>;

template <typename T>
using sawtooth_wave_tween = tween<easing::sawtooth_wave<T>>;

using quad_bezier_curve_tween = tween<easing::quad_bezier_curve>;

using cubic_bezier_curve_tween = tween<easing::cubic_bezier_curve>;

using bezier_curve_tween = tween<easing::bezier_curve>;

using catmull_rom_tween = tween<easing::catmull_rom>;

template <auto Func>
using function_tween = tween<easing::function<Func>>;

template <typename T>
using callable_tween = tween<easing::callable<T>>;

}

#include "Tween.inl"
