// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <array>
#include <concepts>
#include <span>
#include <tuple>
#include <vector>

#include "tcob/core/Color.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/random/Random.hpp"
#include "tcob/core/tweening/Tween.hpp"
#include "tcob/gfx/Geometry.hpp"
#include "tcob/gfx/Gfx.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

struct vertex_tween_group {
    std::vector<vertex> Verts;
    rect_f              BBox {};
};

template <typename T>
concept VertexTweenFunction =
    requires(T& f, f64 t, std::span<vertex_tween_group> verts) {
        { f(t, verts) };
    };

////////////////////////////////////////////////////////////

namespace detail {
    class TCOB_API vertex_tween_base : public tween_base {
    public:
        explicit vertex_tween_base(milliseconds duration);

        void add_group(std::span<vertex> verts);

        void clear_groups();

    protected:
        auto source_groups() const -> std::vector<vertex_tween_group> const&;
        void copy_to_dest(std::span<vertex_tween_group const> groups);

    private:
        std::vector<vertex_tween_group> _srcGroups; // copies
        std::vector<std::span<vertex>>  _dstGroups;
    };

}

////////////////////////////////////////////////////////////

template <VertexTweenFunction... Funcs>
class vertex_tween final : public detail::vertex_tween_base {
public:
    explicit vertex_tween(milliseconds duration, Funcs&&... ptr);

protected:
    void update_values() override;

private:
    std::tuple<Funcs...> _functions;
};

////////////////////////////////////////////////////////////

namespace effects {

    class TCOB_API typing final { // alpha
    public:
        void operator()(f64 t, std::span<vertex_tween_group> groups) const;
    };

    ////////////////////////////////////////////////////////////

    class TCOB_API fade_in final { // alpha
    public:
        i32 Width {1};             // in chars

        void operator()(f64 t, std::span<vertex_tween_group> groups) const;
    };

    ////////////////////////////////////////////////////////////

    class TCOB_API fade_out final { // alpha
    public:
        i32 Width {1};              // in chars

        void operator()(f64 t, std::span<vertex_tween_group> groups) const;
    };

    ////////////////////////////////////////////////////////////

    class TCOB_API blink final { // color
    public:
        color Color0 {colors::White};
        color Color1 {colors::Black};
        f32   Frequency {1.0f};

        void operator()(f64 t, std::span<vertex_tween_group> groups);
    };

    ////////////////////////////////////////////////////////////

    class TCOB_API gradient final { // color
    public:
        std::array<color, 256> Gradient;

        void operator()(f64 t, std::span<vertex_tween_group> groups) const;
    };

    ////////////////////////////////////////////////////////////

    class TCOB_API shake final { // position
    public:
        f32 Intensity {1.0f};    // in pixel
        rng RNG;

        void operator()(f64 t, std::span<vertex_tween_group> groups);
    };

    ////////////////////////////////////////////////////////////

    class TCOB_API jitter final { // position
    public:
        f32 Intensity {1.0f};     // in pixel
        rng RNG {};

        void operator()(f64 t, std::span<vertex_tween_group> groups);
    };

    ////////////////////////////////////////////////////////////

    class TCOB_API wave final { // y
    public:
        f32 Height {0};         // in pixel
        f32 Amplitude {1.0f};

        void operator()(f64 t, std::span<vertex_tween_group> groups) const;
    };

    ////////////////////////////////////////////////////////////

    class TCOB_API bounce final { // y
    public:
        f32 Height {0};           // in pixel

        void operator()(f64 t, std::span<vertex_tween_group> groups) const;
    };

    ////////////////////////////////////////////////////////////

    class TCOB_API elastic final { // y
    public:
        f32 Height {0};            // in pixel
        f64 Amplitude {1.0};
        f64 Period {0.3};

        void operator()(f64 t, std::span<vertex_tween_group> groups) const;
    };

    ////////////////////////////////////////////////////////////

    class TCOB_API back final { // y
    public:
        f32 Height {0};         // in pixel
        f64 Overshoot {1.70158};

        void operator()(f64 t, std::span<vertex_tween_group> groups) const;
    };

    ////////////////////////////////////////////////////////////

    class TCOB_API size final { // position
    public:
        f32 WidthStart {0};
        f32 WidthEnd {1.0f};
        f32 HeightStart {0};
        f32 HeightEnd {1.0f};

        alignment Anchor {};

        void operator()(f64 t, std::span<vertex_tween_group> groups) const;
    };

    ////////////////////////////////////////////////////////////

    class TCOB_API rotate final { // position
    public:
        f32 Speed {1.0f};

        void operator()(f64 t, std::span<vertex_tween_group> groups) const;
    };

}

}

#include "VertexTween.inl"
