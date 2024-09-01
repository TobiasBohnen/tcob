// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <concepts>
#include <functional>
#include <span>
#include <vector>

#include "tcob/core/FlatMap.hpp"
#include "tcob/core/Interfaces.hpp"
#include "tcob/core/random/Random.hpp"
#include "tcob/core/tweening/Tween.hpp"
#include "tcob/gfx/Geometry.hpp"

namespace tcob::gfx {

////////////////////////////////////////////////////////////

template <typename T>
concept QuadTweenFunction =
    requires(T& f, f64 t, std::span<quad> quads) {
        {
            f(t, quads)
        };
    };

////////////////////////////////////////////////////////////

class TCOB_API quad_tween_base : public tweening::tween_base {
public:
    quad_tween_base(milliseconds duration);

    void add_quad(std::reference_wrapper<quad> q);

    void clear_quads();

protected:
    auto get_source_quads() const -> std::vector<quad> const&;
    void set_quads(std::span<quad> quads);

private:
    std::vector<std::reference_wrapper<quad>> _dstQuads {};
    std::vector<quad>                         _srcQuads {};
};

////////////////////////////////////////////////////////////

template <typename... Funcs>
class quad_tween : public quad_tween_base {
public:
    quad_tween(milliseconds duration, Funcs&&... ptr);

protected:
    void update_values() override;

private:
    std::tuple<Funcs...> _functions;
};

////////////////////////////////////////////////////////////

class TCOB_API quad_tweens : public updatable {
public:
    template <typename... Funcs>
    auto create(u8 id, milliseconds duration, Funcs&&... args) -> std::shared_ptr<quad_tween<Funcs...>>;

    auto has(u8 id) const -> bool;

    void start_all(playback_mode mode = playback_mode::Normal);
    void stop_all();

    void add_quad(u8 id, std::reference_wrapper<quad> q) const; // FIXME: better API
    void clear_quads();

private:
    void on_update(milliseconds deltaTime) override;

    flat_map<u8, std::shared_ptr<quad_tween_base>> _effects {};
};

////////////////////////////////////////////////////////////

namespace effect {

    class TCOB_API typing final { // Color
    public:
        void operator()(f64 t, std::span<quad> quads) const;
    };

    ////////////////////////////////////////////////////////////

    class TCOB_API fade_in final { // Color
    public:
        void operator()(f64 t, std::span<quad> quads) const;
    };

    ////////////////////////////////////////////////////////////

    class TCOB_API fade_out final { // Color
    public:
        void operator()(f64 t, std::span<quad> quads) const;
    };

    ////////////////////////////////////////////////////////////

    class TCOB_API blink final { // Color
    public:
        color Color0;
        color Color1;
        f32   Frequency {0};

        void operator()(f64 t, std::span<quad> quads);
    };

    ////////////////////////////////////////////////////////////

    class TCOB_API shake final { // X,Y
    public:
        f32 Intensity {0};
        rng RNG;

        void operator()(f64 t, std::span<quad> quads);
    };

    ////////////////////////////////////////////////////////////

    class TCOB_API wave final { // Y
    public:
        f32 Height;
        f32 Amplitude;

        void operator()(f64 t, std::span<quad> quads) const;
    };

    ////////////////////////////////////////////////////////////

    class TCOB_API size final { // X,Y
    public:
        f32 WidthStart {0};
        f32 WidthEnd {0};
        f32 HeightStart {0};
        f32 HeightEnd {0};

        alignments Anchor {};

        void operator()(f64 t, std::span<quad> quads) const;
    };

    ////////////////////////////////////////////////////////////

    class TCOB_API rotate final { // X,Y
    public:
        void operator()(f64 t, std::span<quad> quads) const;
    };

}

}

#include "QuadTween.inl"
