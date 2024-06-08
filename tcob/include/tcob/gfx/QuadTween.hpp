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
            t.apply(prop)
        };
    };

////////////////////////////////////////////////////////////

class TCOB_API quad_tween_base : public tweening::tween_base {
public:
    quad_tween_base(milliseconds duration);

    void add_quad(quad& q);

    void clear_quads();

protected:
    auto get_destination_quads() const -> std::vector<std::reference_wrapper<quad>> const&;

    auto get_source_quads() const -> std::vector<quad> const&;

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
    void add(u8 id, std::shared_ptr<quad_tween_base> effect);
    auto has(u8 id) const -> bool;

    void start_all(playback_style mode = playback_style::Normal);
    void stop_all();

    void add_quad(u8 id, quad& q) const;
    void clear_quads();

private:
    void on_update(milliseconds deltaTime) override;

    flat_map<u8, std::shared_ptr<quad_tween_base>> _effects {};
};

////////////////////////////////////////////////////////////

class TCOB_API typing_effect final {
public:
    void operator()(quad_tween_properties const& prop) const;
};

////////////////////////////////////////////////////////////

class TCOB_API fade_in_effect final {
public:
    void operator()(quad_tween_properties const& prop) const;
};

////////////////////////////////////////////////////////////

class TCOB_API fade_out_effect final {
public:
    void operator()(quad_tween_properties const& prop) const;
};

////////////////////////////////////////////////////////////

class TCOB_API blink_effect final {
public:
    color Color0;
    color Color1;
    f32   Frequency {0};

    void operator()(quad_tween_properties const& prop);
};

////////////////////////////////////////////////////////////

class TCOB_API shake_effect final {
public:
    f32 Intensity {0};
    f32 Frequency {0};
    rng RNG;

    void operator()(quad_tween_properties const& prop);
};

////////////////////////////////////////////////////////////

class TCOB_API wave_effect final {
public:
    f32 Height;
    f32 Amplitude;

    void operator()(quad_tween_properties const& prop) const;
};

template <typename... Funcs>
auto make_unique_quad_tween(milliseconds duration, Funcs&&... args) -> std::unique_ptr<quad_tween<Funcs...>>;
template <typename... Funcs>
auto make_shared_quad_tween(milliseconds duration, Funcs&&... args) -> std::shared_ptr<quad_tween<Funcs...>>;

}

#include "QuadTween.inl"
