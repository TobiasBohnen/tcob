// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <concepts>
#include <queue>
#include <vector>

#include <tcob/core/Random.hpp>
#include <tcob/core/Updatable.hpp>
#include <tcob/core/data/Color.hpp>
#include <tcob/core/data/Point.hpp>
#include <tcob/gfx/Quad.hpp>
#include <tcob/thirdparty/sigslot/signal.hpp>

namespace tcob {

template <typename T>
concept AutomationFunction = requires(T& t, f32 elapsedRatio)
{
    typename T::type;

    std::same_as<decltype(t.Duration), MilliSeconds>;

    {
        t.value(elapsedRatio)
        } -> std::same_as<typename T::type>;
};

////////////////////////////////////////////////////////////

template <typename T>
concept Interpolatable = requires(T& t, f32 elapsedRatio)
{
    {
        t.interpolate(t, elapsedRatio)
        } -> std::same_as<T>;
};

////////////////////////////////////////////////////////////

class AutomationBase : public Updatable {
public:
    explicit AutomationBase(MilliSeconds duration);
    virtual ~AutomationBase() = default;

    void start(bool looped = false);
    void restart();
    void toggle_pause();
    void stop();

    void interval(MilliSeconds interval);

    auto is_running() const -> bool;

    auto progress() const -> f32;

    void update(MilliSeconds deltaTime) override;

private:
    virtual void update_values() = 0;

    bool _isRunning { false };
    bool _looped { false };

    MilliSeconds _duration { 0 };
    MilliSeconds _elapsedTime { 0 };

    MilliSeconds _interval { 0 };
    MilliSeconds _currentInterval { 0 };
};

////////////////////////////////////////////////////////////

template <AutomationFunction Func>
class Automation final : public AutomationBase {
    using func_type = typename Func::type;

public:
    Automation(Func&& ptr)
        : AutomationBase { ptr.Duration }
        , _function { std::move(ptr) }
    {
    }

    sigslot::signal<func_type> ValueChanged;

    auto add_output(func_type* dest)
    {
        return ValueChanged.connect([dest](const func_type& val) { *dest = val; });
    }

    auto value() const -> func_type
    {
        return _function.value(progress());
    }

private:
    void update_values() override
    {
        ValueChanged(value());
    }

    Func _function;
};

template <typename Func, typename... Rs>
auto make_unique_automation(MilliSeconds duration, Rs&&... args) -> std::unique_ptr<Automation<Func>>
{
    return std::unique_ptr<Automation<Func>>(new Automation<Func> { Func { duration, std::forward<Rs>(args)... } });
}

template <typename Func, typename... Rs>
auto make_shared_automation(MilliSeconds duration, Rs&&... args) -> std::shared_ptr<Automation<Func>>
{
    return std::shared_ptr<Automation<Func>>(new Automation<Func> { Func { duration, std::forward<Rs>(args)... } });
}

////////////////////////////////////////////////////////////

class AutomationQueue final : public Updatable {

public:
    template <typename... Ts>
    void push(Ts&&... autom)
    {
        (_queue.push(std::forward<Ts>(autom)), ...);
    }

    void start(bool looped = false);
    void stop_and_clear();

    auto is_empty() -> bool;

    void update(MilliSeconds deltaTime) override;

private:
    std::queue<std::shared_ptr<AutomationBase>> _queue;
    bool _isRunning { false };
    bool _looped { false };
};

/////////FUNCTIONS//////////////////////////////////////////

template <typename T>
class LinearFunctionChain final {
public:
    using type = T;

    LinearFunctionChain(MilliSeconds duration, std::vector<T>&& elements)
        : Duration { duration }
        , _elements { std::move(elements) }
    {
    }

    auto value(f32 elapsedRatio) const -> T
    {
        if (_elements.empty()) {
            return T {};
        }

        const isize size { _elements.size() - 1 };
        const f32 relElapsed { size * elapsedRatio };
        const isize index { static_cast<isize>(relElapsed) };
        if (index >= size) {
            return _elements[index];
        } else {
            const T& current { _elements[index] };
            const T& next { _elements[index + 1] };

            if constexpr (Interpolatable<T>) {
                return current.interpolate(next, relElapsed - index);
            } else {
                return current + ((next - current) * (relElapsed - index));
            }
        }
    }

    MilliSeconds Duration;

private:
    std::vector<T> _elements;
};

////////////////////////////////////////////////////////////

template <typename T>
struct PowerFunction final {
    using type = T;

    MilliSeconds Duration;
    T StartValue;
    T EndValue;
    f32 Exponent;

    auto value(f32 elapsedRatio) const -> T
    {
        if (Exponent <= 0 && elapsedRatio == 0) {
            return StartValue;
        }

        if constexpr (Interpolatable<T>) {
            return StartValue.interpolate(EndValue, std::pow(elapsedRatio, Exponent));
        } else {
            return StartValue + ((EndValue - StartValue) * std::pow(elapsedRatio, Exponent));
        }
    }
};

////////////////////////////////////////////////////////////

template <typename T>
struct InversePowerFunction final {
    using type = T;

    MilliSeconds Duration;
    T StartValue;
    T EndValue;
    f32 Exponent;

    auto value(f32 elapsedRatio) const -> T
    {
        if (Exponent <= 0 && elapsedRatio == 0) {
            return StartValue;
        }

        if constexpr (Interpolatable<T>) {
            return StartValue.interpolate(EndValue, 1 - std::pow(1 - elapsedRatio, Exponent));
        } else {
            return StartValue + ((EndValue - StartValue) * (1 - std::pow(1 - elapsedRatio, Exponent)));
        }
    }
};

////////////////////////////////////////////////////////////

template <typename T>
struct LinearFunction final {
    using type = T;

    MilliSeconds Duration;
    T StartValue;
    T EndValue;

    auto value(f32 elapsedRatio) const -> T
    {
        if (elapsedRatio == 0) {
            return StartValue;
        }

        if constexpr (Interpolatable<T>) {
            return StartValue.interpolate(EndValue, elapsedRatio);
        } else {
            return StartValue + ((EndValue - StartValue) * elapsedRatio);
        }
    }
};

////////////////////////////////////////////////////////////

template <typename T>
struct SmoothstepFunction final {
    using type = T;

    MilliSeconds Duration;
    T Edge0;
    T Edge1;

    auto value(f32 elapsedRatio) const -> T
    {
        if (elapsedRatio == 0) {
            return Edge0;
        }

        f32 e { elapsedRatio * elapsedRatio * (3.f - 2.f * elapsedRatio) };
        if constexpr (Interpolatable<T>) {
            return Edge0.interpolate(Edge1, e);
        } else {
            return Edge0 + ((Edge1 - Edge0) * e);
        }
    }
};

////////////////////////////////////////////////////////////

template <typename T>
struct SmootherstepFunction final {
    using type = T;

    MilliSeconds Duration;
    T Edge0;
    T Edge1;

    auto value(f32 elapsedRatio) const -> T
    {
        if (elapsedRatio == 0) {
            return Edge0;
        }

        f32 e { elapsedRatio * elapsedRatio * elapsedRatio * (elapsedRatio * (elapsedRatio * 6 - 15) + 10) };
        if constexpr (Interpolatable<T>) {
            return Edge0.interpolate(Edge1, e);
        } else {
            return Edge0 + ((Edge1 - Edge0) * e);
        }
    }
};

////////////////////////////////////////////////////////////

template <typename T>
struct SineWaveFunction final {
    using type = T;

    MilliSeconds Duration;
    T MinValue;
    T MaxValue;
    f32 Frequency;
    f32 Phase;

    auto value(f32 elapsedRatio) const -> T
    {
        const f64 val { waveValue((Frequency * elapsedRatio)) };
        if constexpr (Interpolatable<T>) {
            return MinValue.interpolate(MaxValue, val);
        } else {
            return MinValue + ((MaxValue - MinValue) * val);
        }
    }

private:
    auto waveValue(f64 time) const -> f64
    {
        return (std::sin((TAU * time) + (0.75 * TAU) + Phase) + 1) / 2;
    }
};

////////////////////////////////////////////////////////////

template <typename T>
struct TriangeWaveFunction final {
    using type = T;

    MilliSeconds Duration;
    T MinValue;
    T MaxValue;
    f32 Frequency;
    f32 Phase;

    auto value(f32 elapsedRatio) const -> T
    {
        const f64 val { waveValue((Frequency * elapsedRatio) + Phase) };
        if constexpr (Interpolatable<T>) {
            return MinValue.interpolate(MaxValue, val);
        } else {
            return MinValue + ((MaxValue - MinValue) * val);
        }
    }

private:
    auto waveValue(f64 time) const -> f64
    {
        return 2 * std::abs(std::round(time) - time);
    }
};

////////////////////////////////////////////////////////////

template <typename T>
struct SquareWaveFunction final {
    using type = T;

    MilliSeconds Duration;
    T MinValue;
    T MaxValue;
    f32 Frequency;
    f32 Phase;

    auto value(f32 elapsedRatio) const -> T
    {
        const f64 val { waveValue((Frequency * elapsedRatio)) };
        if constexpr (Interpolatable<T>) {
            return MinValue.interpolate(MaxValue, val);
        } else {
            return MinValue + ((MaxValue - MinValue) * val);
        }
    }

private:
    auto waveValue(f64 time) const -> f64
    {
        const f64 x { std::round(time + Phase) / 2 };
        return 2 * (x - std::floor(x));
    }
};

////////////////////////////////////////////////////////////

template <typename T>
struct SawtoothWaveFunction final {
    using type = T;

    MilliSeconds Duration;
    T MinValue;
    T MaxValue;
    f32 Frequency;
    f32 Phase;

    auto value(f32 elapsedRatio) const -> T
    {
        const f64 val { waveValue((Frequency * elapsedRatio) + Phase) };
        if constexpr (Interpolatable<T>) {
            return MinValue.interpolate(MaxValue, val);
        } else {
            return MinValue + ((MaxValue - MinValue) * val);
        }
    }

private:
    auto waveValue(f64 time) const -> f64
    {
        return time - std::floor(time);
    }
};

////////////////////////////////////////////////////////////

struct CubicBezierFunction final {
    using type = PointF;

    MilliSeconds Duration;
    PointF Start;
    PointF ControlPoint0;
    PointF ControlPoint1;
    PointF End;

    auto value(f32 elapsedRatio) const -> PointF
    {
        const PointF a { point_in_line(Start, ControlPoint0, elapsedRatio) };
        const PointF b { point_in_line(ControlPoint0, ControlPoint1, elapsedRatio) };
        const PointF c { point_in_line(ControlPoint1, End, elapsedRatio) };
        return point_in_line(point_in_line(a, b, elapsedRatio), point_in_line(b, c, elapsedRatio), elapsedRatio);
    }

private:
    auto point_in_line(const PointF& a, const PointF& b, f32 t) const -> PointF
    {
        return {
            a.X - ((a.X - b.X) * t),
            a.Y - ((a.Y - b.Y) * t)
        };
    }
};

////////////////////////////////////////////////////////////

template <typename T>
struct RandomFunction final {
    using type = T;

    MilliSeconds Duration;
    T MinValue;
    T MaxValue;
    mutable Random RNG;

    auto value(f32 elapsedRatio) const -> T
    {
        return RNG(MinValue, MaxValue);
    }
};
}