// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/QuadTween.hpp"

namespace tcob::gfx {
using namespace std::chrono_literals;

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

auto quad_tween_base::get_props() const -> quad_tween_properties
{
    return {
        .Progress  = get_progress(),
        .SrcQuads  = _srcQuads,
        .DestQuads = _dstQuads};
}

////////////////////////////////////////////////////////////

void quad_tweens::start_all(playback_mode mode)
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

namespace effect {

    void typing::operator()(quad_tween_properties const& prop) const
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

    void fade_in::operator()(quad_tween_properties const& prop) const
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

    void fade_out::operator()(quad_tween_properties const& prop) const
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

    void blink::operator()(quad_tween_properties const& prop)
    {
        f64 const  x {std::round(Frequency * prop.Progress) / 2};
        bool const flip {2 * (x - std::floor(x)) == 0.};
        for (usize idx {0}; idx < prop.DestQuads.size(); ++idx) {
            geometry::set_color(prop.DestQuads[idx], flip ? Color0 : Color1);
        }
    }

    ////////////////////////////////////////////////////////////

    void shake::operator()(quad_tween_properties const& prop)
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

    void wave::operator()(quad_tween_properties const& prop) const
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

    void height::operator()(quad_tween_properties const& prop) const
    {
        f32 maxHeight {0};
        for (auto const& src : prop.SrcQuads) {
            f32 const height {src[2].Position[1] - src[3].Position[1]};
            if (height > maxHeight) { maxHeight = height; }
        }

        f32 const difference {End - Begin};
        f32 const fac {difference > 0 ? static_cast<f32>(prop.Progress * difference)
                                      : static_cast<f32>(1.0 - (prop.Progress * -difference))};

        for (usize idx {0}; idx < prop.DestQuads.size(); ++idx) {
            quad&       dest {prop.DestQuads[idx].get()};
            quad const& src {prop.SrcQuads[idx]};

            rect_f const rect {rect_f::FromLTRB(src[3].Position[0], src[3].Position[1], src[1].Position[0], src[1].Position[1])};
            transform    scale;

            switch (Anchor) {
            case vertical_alignment::Top:
                scale.scale_at({1, fac}, {rect.get_center().X, rect.top() - maxHeight + rect.Height});
                break;
            case vertical_alignment::Middle:
                scale.scale_at({1, fac}, {rect.get_center().X, rect.get_center().Y - (maxHeight - rect.Height) / 2});
                break;
            case vertical_alignment::Bottom:
                scale.scale_at({1, fac}, {rect.get_center().X, rect.bottom()});
                break;
            }

            for (i32 i {0}; i < 4; ++i) {
                dest[i].Position[1] = (scale * point_f {0, src[i].Position[1]}).Y;
            }
        }
    }

    ////////////////////////////////////////////////////////////

    void rotate::operator()(quad_tween_properties const& prop) const
    {
        for (usize idx {0}; idx < prop.DestQuads.size(); ++idx) {
            quad&       dest {prop.DestQuads[idx].get()};
            quad const& src {prop.SrcQuads[idx]};

            rect_f const rect {rect_f::FromLTRB(src[3].Position[0], src[3].Position[1], src[1].Position[0], src[1].Position[1])};
            transform    rot;
            rot.rotate_at(degree_f {static_cast<f32>(360 * prop.Progress)}, rect.get_center());

            for (i32 i {0}; i < 4; ++i) {
                point_f const pos {rot * point_f {src[i].Position[0], src[i].Position[1]}};
                dest[i].Position[0] = pos.X;
                dest[i].Position[1] = pos.Y;
            }
        }
    }

}
}
