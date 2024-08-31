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

struct quad_tween_properties {
    f64                                           Progress {};
    std::span<quad const>                         SrcQuads {};
    std::span<std::reference_wrapper<quad> const> DestQuads {};
};

////////////////////////////////////////////////////////////

template <typename T>
concept QuadTweenFunction =
    requires(T& t, quad_tween_properties const& prop) {
        {
            t(prop)
        };
    };

////////////////////////////////////////////////////////////

class TCOB_API quad_tween_base : public tweening::tween_base {
public:
    quad_tween_base(milliseconds duration);

    void add_quad(quad& q);

    void clear_quads();

protected:
    auto get_props() const -> quad_tween_properties;

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

    void add_quad(u8 id, quad& q) const;
    void clear_quads();

private:
    void on_update(milliseconds deltaTime) override;

    flat_map<u8, std::shared_ptr<quad_tween_base>> _effects {};
};

////////////////////////////////////////////////////////////

namespace effect {

    class TCOB_API typing final {
    public:
        void operator()(quad_tween_properties const& prop) const;
    };

    ////////////////////////////////////////////////////////////

    class TCOB_API fade_in final {
    public:
        void operator()(quad_tween_properties const& prop) const;
    };

    ////////////////////////////////////////////////////////////

    class TCOB_API fade_out final {
    public:
        void operator()(quad_tween_properties const& prop) const;
    };

    ////////////////////////////////////////////////////////////

    class TCOB_API blink final {
    public:
        color Color0;
        color Color1;
        f32   Frequency {0};

        void operator()(quad_tween_properties const& prop);
    };

    ////////////////////////////////////////////////////////////

    class TCOB_API shake final {
    public:
        f32 Intensity {0};
        f32 Frequency {0};
        rng RNG;

        void operator()(quad_tween_properties const& prop);
    };

    ////////////////////////////////////////////////////////////

    class TCOB_API wave final {
    public:
        f32 Height;
        f32 Amplitude;

        void operator()(quad_tween_properties const& prop) const;
    };

    ////////////////////////////////////////////////////////////

    class TCOB_API height final {
    public:
        f32                Begin {0};
        f32                End {1};
        vertical_alignment Anchor {vertical_alignment::Middle};

        void operator()(quad_tween_properties const& prop) const;
    };

    ////////////////////////////////////////////////////////////

    class TCOB_API rotate final {
    public:
        void operator()(quad_tween_properties const& prop) const;
    };

}

}

#include "QuadTween.inl"
