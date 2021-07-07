// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <concepts>
#include <functional>
#include <vector>

#include <tcob/core/Automation.hpp>
#include <tcob/gfx/Quad.hpp>

namespace tcob {

template <typename T>
concept HasInterval = requires(T& t)
{
    std::same_as<decltype(t.Interval), MilliSeconds>;
};

template <typename T>
concept QuadEffectFunction = requires(T& t, Quad& q)
{
    std::same_as<decltype(t.Duration), MilliSeconds>;

    {
        t.value(0.0f, 0, 0, q, q)
    };
};

////////////////////////////////////////////////////////////
class QuadEffectBase : public AutomationBase {
public:
    QuadEffectBase(MilliSeconds duration)
        : AutomationBase { duration }
    {
    }

    void add_quad(Quad& q)
    {
        _quads.emplace_back(q);
        _oriQuads.push_back(q);
    }

    void clear_quads()
    {
        _quads.clear();
        _oriQuads.clear();
    }

protected:
    auto ref_quads() const -> const std::vector<std::reference_wrapper<Quad>>&
    {
        return _quads;
    }

    auto ori_quads() const -> const std::vector<Quad>&
    {
        return _oriQuads;
    }

private:
    std::vector<std::reference_wrapper<Quad>> _quads {};
    std::vector<Quad> _oriQuads {};
};

template <QuadEffectFunction Func>
class QuadEffect : public QuadEffectBase {
public:
    QuadEffect(Func&& ptr)
        : QuadEffectBase { ptr.Duration }
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
        const f32 e { progress() };
        const auto& refQuads { ref_quads() };
        const auto& oriQuads { ori_quads() };

        for (auto& q : refQuads) {
            _function.value(e, i, refQuads.size(), q, oriQuads[i]);
            ++i;
        }
    }

private:
    Func _function;
};

template <typename Func, typename... Rs>
auto make_unique_quadeffect(MilliSeconds duration, Rs&&... args) -> std::unique_ptr<QuadEffect<Func>>
{
    return std::unique_ptr<QuadEffect<Func>>(new QuadEffect<Func> { Func { duration, std::forward<Rs>(args)... } });
}

template <typename Func, typename... Rs>
auto make_shared_quadeffect(MilliSeconds duration, Rs&&... args) -> std::shared_ptr<QuadEffect<Func>>
{
    return std::shared_ptr<QuadEffect<Func>>(new QuadEffect<Func> { Func { duration, std::forward<Rs>(args)... } });
}

////////////////////////////////////////////////////////////

struct TypingEffect final {
    MilliSeconds Duration;

    void value(f32 progress, isize index, isize length, Quad& dest, const Quad& src) const;
};

////////////////////////////////////////////////////////////

struct FadeInEffect final {
    MilliSeconds Duration;

    void value(f32 progress, isize index, isize length, Quad& dest, const Quad& src) const;
};

////////////////////////////////////////////////////////////

struct FadeOutEffect final {
    MilliSeconds Duration;

    void value(f32 progress, isize index, isize length, Quad& dest, const Quad& src) const;
};

////////////////////////////////////////////////////////////

struct BlinkEffect final {
public:
    BlinkEffect(MilliSeconds duration, MilliSeconds interval, Color color0, Color color1);

    MilliSeconds Duration;
    MilliSeconds Interval;
    Color Color0;
    Color Color1;

    void value(f32 progress, isize index, isize length, Quad& dest, const Quad& src);

private:
    bool _flip { false };
};

////////////////////////////////////////////////////////////

struct ShakeEffect final {
    MilliSeconds Duration;
    MilliSeconds Interval;
    f32 Intensity;
    Random RNG;

    void value(f32 progress, isize index, isize length, Quad& dest, const Quad& src);
};

////////////////////////////////////////////////////////////

struct WaveEffect final {
    MilliSeconds Duration;
    f32 Height;
    f32 Intensity;

    void value(f32 progress, isize index, isize length, Quad& dest, const Quad& src);
};

}