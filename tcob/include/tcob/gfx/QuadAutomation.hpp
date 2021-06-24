// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <concepts>
#include <functional>
#include <map>

#include <tcob/core/Automation.hpp>
#include <tcob/gfx/Quad.hpp>

namespace tcob {

template <typename T>
concept HasInterval = requires(T& t)
{
    std::same_as<decltype(t.Interval), MilliSeconds>;
};

template <typename T>
concept QuadAutomationFunction = requires(T& t, Quad& q)
{
    std::same_as<decltype(t.Duration), MilliSeconds>;

    {
        t.value(0.0f, 0, 0, q)
    };
};

////////////////////////////////////////////////////////////
class QuadAutomationBase : public AutomationBase {
public:
    QuadAutomationBase(MilliSeconds duration)
        : AutomationBase { duration }
    {
    }

    void add_quad(isize idx, Quad& q)
    {
        _quads.emplace(idx, q);
    }

protected:
    auto quads() -> const std::map<isize, std::reference_wrapper<Quad>>&
    {
        return _quads;
    }

private:
    std::map<isize, std::reference_wrapper<Quad>> _quads {};
};

template <QuadAutomationFunction Func>
class QuadAutomation : public QuadAutomationBase {
public:
    QuadAutomation(Func&& ptr)
        : QuadAutomationBase { ptr.Duration }
        , _function { std::move(ptr) }
    {
        if constexpr (HasInterval<Func>) {
            interval(ptr.Interval);
        }
    }

protected:
    void update_values() override
    {
        isize i { 0 };
        for (auto& [k, v] : quads()) {
            _function.value(progress(), i++, quads().size(), v);
        }
    }

private:
    Func _function;
};

template <typename Func, typename... Rs>
auto make_unique_quadautomation(MilliSeconds duration, Rs&&... args) -> std::unique_ptr<QuadAutomation<Func>>
{
    return std::unique_ptr<QuadAutomation<Func>>(new QuadAutomation<Func> { Func { duration, std::forward<Rs>(args)... } });
}

template <typename Func, typename... Rs>
auto make_shared_quadautomation(MilliSeconds duration, Rs&&... args) -> std::shared_ptr<QuadAutomation<Func>>
{
    return std::shared_ptr<QuadAutomation<Func>>(new QuadAutomation<Func> { Func { duration, std::forward<Rs>(args)... } });
}

////////////////////////////////////////////////////////////

struct FadeInEffect final {
    MilliSeconds Duration;

    void value(f32 progress, isize index, isize length, Quad& quad);
};

////////////////////////////////////////////////////////////

struct FadeOutEffect final {
    MilliSeconds Duration;

    void value(f32 progress, isize index, isize length, Quad& quad);
};

////////////////////////////////////////////////////////////

struct BlinkEffect final {
public:
    BlinkEffect(MilliSeconds duration, MilliSeconds interval, Color color0, Color color1);

    MilliSeconds Duration;
    MilliSeconds Interval;
    Color Color0;
    Color Color1;

    void value(f32 progress, isize index, isize length, Quad& quad);

private:
    bool _flip { false };
};

}