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
concept QuadEffectFunction = requires(T& t, Quad& q)
{
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

template <QuadEffectFunction... Funcs>
class QuadEffects : public QuadEffectBase {
public:
    QuadEffects(MilliSeconds duration, Funcs&&... ptr)
        : QuadEffectBase { duration }
        , _functions { std::make_tuple(ptr...) }
    {
    }

protected:
    void update_values() override
    {
        isize i { 0 };
        const f32 e { progress() };
        const auto& refQuads { ref_quads() };
        const auto& oriQuads { ori_quads() };

        for (isize i { 0 }; i < refQuads.size(); ++i) {
            std::apply([=](auto&&... func) { (func.value(e, i, refQuads.size(), refQuads[i], oriQuads[i]), ...); },
                _functions);
        }
    }

private:
    std::tuple<Funcs...> _functions;
};

template <typename... Funcs>
auto make_unique_quadeffects(MilliSeconds duration, Funcs&&... args) -> std::unique_ptr<QuadEffects<Funcs...>>
{
    return std::unique_ptr<QuadEffects<Funcs...>>(new QuadEffects<Funcs...> { duration, std::forward<Funcs>(args)... });
}

template <typename... Funcs>
auto make_shared_quadeffects(MilliSeconds duration, Funcs&&... args) -> std::shared_ptr<QuadEffects<Funcs...>>
{
    return std::shared_ptr<QuadEffects<Funcs...>>(new QuadEffects<Funcs...> { duration, std::forward<Funcs>(args)... });
}

////////////////////////////////////////////////////////////

struct TypingEffect final {
    void value(f32 progress, isize index, isize length, Quad& dest, const Quad& src) const;
};

////////////////////////////////////////////////////////////

struct FadeInEffect final {
    void value(f32 progress, isize index, isize length, Quad& dest, const Quad& src) const;
};

////////////////////////////////////////////////////////////

struct FadeOutEffect final {
    void value(f32 progress, isize index, isize length, Quad& dest, const Quad& src) const;
};

////////////////////////////////////////////////////////////

struct BlinkEffect final {

    Color Color0;
    Color Color1;
    f32 Frequency;

    void value(f32 progress, isize index, isize length, Quad& dest, const Quad& src);
};

////////////////////////////////////////////////////////////

struct ShakeEffect final {

    f32 Intensity;
    f32 Frequency;
    Random RNG;

    void value(f32 progress, isize index, isize length, Quad& dest, const Quad& src);
};

////////////////////////////////////////////////////////////

struct WaveEffect final {

    f32 Height;
    f32 Amplitude;

    void value(f32 progress, isize index, isize length, Quad& dest, const Quad& src);
};

}