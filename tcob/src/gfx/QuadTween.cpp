// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/QuadTween.hpp"

namespace tcob::gfx {
using namespace std::chrono_literals;

////////////////////////////////////////////////////////////

void quad_tweens::add(u8 id, std::shared_ptr<quad_tween_base> effect)
{
    if (id == 0) {
        // TODO: log error
        return;
    }

    _effects[id] = std::move(effect);
}

void quad_tweens::start_all(playback_style mode)
{
    for (auto& [_, effect] : _effects) {
        effect->start(mode);
    }
}

void quad_tweens::stop_all()
{
    for (auto& [_, effect] : _effects) {
        effect->stop();
    }
}

void quad_tweens::add_quad(u8 id, quad& q) const
{
    _effects.at(id)->add_quad(q);
}

auto quad_tweens::has(u8 id) const -> bool
{
    return _effects.contains(id);
}

void quad_tweens::clear_quads()
{
    for (auto& [_, effect] : _effects) {
        effect->clear_quads();
    }
}

void quad_tweens::on_update(milliseconds deltaTime)
{
    for (auto& [_, effect] : _effects) {
        effect->update(deltaTime);
    }
}

////////////////////////////////////////////////////////////

void typing_effect::operator()(quad_tween_properties const& prop) const
{
    usize const fadeidx {static_cast<usize>(prop.Progress * prop.DestQuads.size())};
    for (usize idx {0}; idx < prop.DestQuads.size(); ++idx) {
        quad& dest {prop.DestQuads[idx].get()};
        if (idx <= fadeidx) {
            dest[3].Color[3] = 255;
            dest[1].Color[3] = 255;
            dest[0].Color[3] = 255;
            dest[2].Color[3] = 255;
        } else {
            dest[3].Color[3] = 0;
            dest[1].Color[3] = 0;
            dest[0].Color[3] = 0;
            dest[2].Color[3] = 0;
        }
    }
}

////////////////////////////////////////////////////////////

void fade_in_effect::operator()(quad_tween_properties const& prop) const
{
    usize const fadeidx {static_cast<usize>(prop.Progress * prop.DestQuads.size())};
    for (usize idx {0}; idx < prop.DestQuads.size(); ++idx) {
        quad& dest {prop.DestQuads[idx].get()};
        if (idx < fadeidx) {
            dest[3].Color[3] = 255;
            dest[1].Color[3] = 255;
            dest[0].Color[3] = 255;
            dest[2].Color[3] = 255;
        } else if (idx > fadeidx) {
            dest[3].Color[3] = 0;
            dest[1].Color[3] = 0;
            dest[0].Color[3] = 0;
            dest[2].Color[3] = 0;
        } else {
            f64 const val {(prop.Progress * prop.DestQuads.size()) - fadeidx};
            dest[3].Color[3] = static_cast<u8>(val * 255.);
            dest[1].Color[3] = static_cast<u8>(val * 255.);
            dest[0].Color[3] = static_cast<u8>(val * 255.);
            dest[2].Color[3] = static_cast<u8>(val * 255.);
        }
    }
}

////////////////////////////////////////////////////////////

void fade_out_effect::operator()(quad_tween_properties const& prop) const
{
    usize const fadeidx {static_cast<usize>(prop.Progress * prop.DestQuads.size())};
    for (usize idx {0}; idx < prop.DestQuads.size(); ++idx) {
        quad& dest {prop.DestQuads[idx].get()};
        if (fadeidx < idx) {
            dest[3].Color[3] = 255;
            dest[1].Color[3] = 255;
            dest[0].Color[3] = 255;
            dest[2].Color[3] = 255;
        } else if (fadeidx > idx) {
            dest[3].Color[3] = 0;
            dest[1].Color[3] = 0;
            dest[0].Color[3] = 0;
            dest[2].Color[3] = 0;
        } else {
            f64 const val {1 - ((prop.Progress * prop.DestQuads.size()) - fadeidx)};
            dest[3].Color[3] = static_cast<u8>(val * 255.);
            dest[1].Color[3] = static_cast<u8>(val * 255.);
            dest[0].Color[3] = static_cast<u8>(val * 255.);
            dest[2].Color[3] = static_cast<u8>(val * 255.);
        }
    }
}

////////////////////////////////////////////////////////////

void blink_effect::operator()(quad_tween_properties const& prop)
{
    f64 const  x {std::round(Frequency * prop.Progress) / 2};
    bool const flip {2 * (x - std::floor(x)) == 0.};
    for (usize idx {0}; idx < prop.DestQuads.size(); ++idx) {
        geometry::set_color(prop.DestQuads[idx], flip ? Color0 : Color1);
    }
}

////////////////////////////////////////////////////////////

void shake_effect::operator()(quad_tween_properties const& prop)
{
    for (usize idx {0}; idx < prop.DestQuads.size(); ++idx) {
        quad& dest {prop.DestQuads[idx].get()};

        f32 const   r {RNG(-Intensity, Intensity)};
        quad const& src {prop.SrcQuads[idx]};
        switch (RNG(0, 1)) {
        case 0:
            dest[3].Position[0] = src[3].Position[0] + r;
            dest[3].Position[1] = src[3].Position[1] + r;
            dest[1].Position[0] = src[1].Position[0] + r;
            dest[1].Position[1] = src[1].Position[1] + r;
            dest[0].Position[0] = src[0].Position[0] + r;
            dest[0].Position[1] = src[0].Position[1] + r;
            dest[2].Position[0] = src[2].Position[0] + r;
            dest[2].Position[1] = src[2].Position[1] + r;
            break;
        case 1:
            dest[3].Position[0] = src[3].Position[0] + r;
            dest[3].Position[1] = src[3].Position[1] - r;
            dest[1].Position[0] = src[1].Position[0] + r;
            dest[1].Position[1] = src[1].Position[1] - r;
            dest[0].Position[0] = src[0].Position[0] + r;
            dest[0].Position[1] = src[0].Position[1] - r;
            dest[2].Position[0] = src[2].Position[0] + r;
            dest[2].Position[1] = src[2].Position[1] - r;
            break;
        default:
            break;
        }
    }
}

////////////////////////////////////////////////////////////

void wave_effect::operator()(quad_tween_properties const& prop) const
{
    for (usize idx {0}; idx < prop.DestQuads.size(); ++idx) {
        quad& dest {prop.DestQuads[idx].get()};

        quad const& src {prop.SrcQuads[idx]};
        f64 const   phase {static_cast<f64>(idx) / prop.DestQuads.size()};
        f64 const   factor {(std::sin((TAU * prop.Progress) + (0.75 * TAU) + phase * Amplitude) + 1) / 2};

        f64 const val {factor * Height};

        dest[3].Position[1] = static_cast<f32>(src[3].Position[1] + val);
        dest[1].Position[1] = static_cast<f32>(src[1].Position[1] + val);
        dest[0].Position[1] = static_cast<f32>(src[0].Position[1] + val);
        dest[2].Position[1] = static_cast<f32>(src[2].Position[1] + val);
    }
}

////////////////////////////////////////////////////////////

quad_tween_base::quad_tween_base(milliseconds duration)
    : tween_base {duration}
{
}

void quad_tween_base::add_quad(quad& q)
{
    _dstQuads.emplace_back(q);
    _srcQuads.push_back(q);
}

void quad_tween_base::clear_quads()
{
    _dstQuads.clear();
    _srcQuads.clear();
}

auto quad_tween_base::get_destination_quads() const -> std::vector<std::reference_wrapper<quad>> const&
{
    return _dstQuads;
}

auto quad_tween_base::get_source_quads() const -> std::vector<quad> const&
{
    return _srcQuads;
}

}
