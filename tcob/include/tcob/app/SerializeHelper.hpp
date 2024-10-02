// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

// IWYU pragma: always_keep
// TODO: move back to module

#include "tcob/tcob_config.hpp"

#include "tcob/audio/SoundGenerator.hpp"
#include "tcob/core/Color.hpp"
#include "tcob/core/Concepts.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/gfx/Animation.hpp"
#include "tcob/gfx/ColorGradient.hpp"
#include "tcob/gfx/Font.hpp"
#include "tcob/gfx/drawables/ParticleSystem.hpp"

namespace tcob {

////////////////////////////////////////////////////////////

template <Arithmetic T>
void Serialize(point<T> const& v, auto&& s)
{
    s["x"] = v.X;
    s["y"] = v.Y;
}

template <Arithmetic T>
auto Deserialize(point<T>& v, auto&& s) -> bool
{
    return s.try_get(v.X, "x") && s.try_get(v.Y, "y");
}

////////////////////////////////////////////////////////////

template <Arithmetic T>
void Serialize(rect<T> const& v, auto&& s)
{
    s["x"]      = v.X;
    s["y"]      = v.Y;
    s["width"]  = v.Width;
    s["height"] = v.Height;
}

template <Arithmetic T>
auto Deserialize(rect<T>& v, auto&& s) -> bool
{
    return s.try_get(v.X, "x") && s.try_get(v.Y, "y")
        && s.try_get(v.Width, "width") && s.try_get(v.Height, "height");
}

////////////////////////////////////////////////////////////

template <Arithmetic T>
void Serialize(size<T> const& v, auto&& s)
{
    s["width"]  = v.Width;
    s["height"] = v.Height;
}

template <Arithmetic T>
auto Deserialize(size<T>& v, auto&& s) -> bool
{
    return s.try_get(v.Width, "width") && s.try_get(v.Height, "height");
}

////////////////////////////////////////////////////////////

void Serialize(color v, auto&& s)
{
    s["r"] = v.R;
    s["g"] = v.G;
    s["b"] = v.B;
    s["a"] = v.A;
}

auto Deserialize(color& v, auto&& s) -> bool
{
    if (s.try_get(v.R, "r") && s.try_get(v.G, "g") && s.try_get(v.B, "b")) {
        if (!s.try_get(v.A, "a")) {
            v.A = static_cast<u8>(255);
        }
        return true;
    }
    return false;
}

////////////////////////////////////////////////////////////

}

namespace tcob::gfx {

////////////////////////////////////////////////////////////

void Serialize(particle_template const& v, auto&& s)
{
    s["acceleration"] = v.Acceleration;
    s["direction"]    = v.Direction;
    s["lifetime"]     = v.Lifetime;
    s["scale"]        = v.Scale;
    s["size"]         = v.Size;
    s["speed"]        = v.Speed;
    s["spin"]         = v.Spin;
    s["texture"]      = v.Texture;
    s["color"]        = v.Color;
    s["transparency"] = v.Transparency;
}

auto Deserialize(particle_template& v, auto&& s) -> bool
{
    return s.try_get(v.Acceleration, "acceleration")
        && s.try_get(v.Direction, "direction")
        && s.try_get(v.Lifetime, "lifetime")
        && s.try_get(v.Scale, "scale")
        && s.try_get(v.Size, "size")
        && s.try_get(v.Speed, "speed")
        && s.try_get(v.Spin, "spin")
        && s.try_get(v.Texture, "texture")
        && s.try_get(v.Color, "color")
        && s.try_get(v.Transparency, "transparency");
}

////////////////////////////////////////////////////////////

void Serialize(particle_emitter const& v, auto&& s)
{
    s["template"]   = v.Template;
    s["spawn_area"] = v.SpawnArea;
    s["spawn_rate"] = v.SpawnRate;
    if (v.Lifetime) {
        s["lifetime"] = *v.Lifetime;
    }
}

auto Deserialize(particle_emitter& v, auto&& s) -> bool
{
    if (s.has("lifetime")) {
        v.Lifetime = s["lifetime"].template as<milliseconds>();
    }
    return s.try_get(v.Template, "template")
        && s.try_get(v.SpawnArea, "spawn_area")
        && s.try_get(v.SpawnRate, "spawn_rate");
}

////////////////////////////////////////////////////////////

void Serialize(frame const& v, auto&& s)
{
    s["name"]     = v.Name;
    s["duration"] = v.Duration;
}

auto Deserialize(frame& v, auto&& s) -> bool
{
    return s.try_get(v.Name, "name") && s.try_get(v.Duration, "duration");
}

////////////////////////////////////////////////////////////

void Serialize(color_stop const& v, auto&& s)
{
    s["pos"]   = v.Position;
    s["value"] = v.Value;
}

auto Deserialize(color_stop& v, auto&& s) -> bool
{
    return s.try_get(v.Position, "pos") && s.try_get(v.Value, "value");
}

////////////////////////////////////////////////////////////

void Serialize(font::style const& v, auto&& s)
{
    s["is_italic"] = v.IsItalic;
    s["weight"]    = v.Weight;
}

auto Deserialize(font::style& v, auto&& s) -> bool
{
    return s.try_get(v.IsItalic, "is_italic")
        && s.try_get(v.Weight, "weight");
}

////////////////////////////////////////////////////////////

void Serialize(texture_region const& v, auto&& s)
{
    s["level"] = v.Level;
    Serialize(v.UVRect, s);
}

auto Deserialize(texture_region& v, auto&& s) -> bool
{
    return s.try_get(v.Level, "level") && Deserialize(v.UVRect, s);
}

////////////////////////////////////////////////////////////

void Serialize(alignments const& v, auto&& s)
{
    s["horizontal"] = v.Horizontal;
    s["vertical"]   = v.Vertical;
}

auto Deserialize(alignments& v, auto&& s) -> bool
{
    return s.try_get(v.Horizontal, "horizontal") && s.try_get(v.Vertical, "vertical");
}

////////////////////////////////////////////////////////////

void Serialize(video_config const& v, auto&& s)
{
    s[Cfg::Video::fullscreen]             = v.FullScreen;
    s[Cfg::Video::use_desktop_resolution] = v.UseDesktopResolution;
    s[Cfg::Video::resolution]             = v.Resolution;
    s[Cfg::Video::frame_limit]            = v.FrameLimit;
    s[Cfg::Video::vsync]                  = v.VSync;
    s[Cfg::Video::render_system]          = v.RenderSystem;
}

auto Deserialize(video_config& v, auto&& s) -> bool
{
    return s.try_get(v.FullScreen, Cfg::Video::fullscreen)
        && s.try_get(v.UseDesktopResolution, Cfg::Video::use_desktop_resolution)
        && s.try_get(v.Resolution, Cfg::Video::resolution)
        && s.try_get(v.FrameLimit, Cfg::Video::frame_limit)
        && s.try_get(v.VSync, Cfg::Video::vsync)
        && s.try_get(v.RenderSystem, Cfg::Video::render_system);
}

////////////////////////////////////////////////////////////

}

namespace tcob::audio {

////////////////////////////////////////////////////////////

void Serialize(sound_wave const& v, auto&& s)
{
    s["random_seed"]      = v.RandomSeed;
    s["sample_rate"]      = v.SampleRate;
    s["wave_type"]        = v.WaveType;
    s["attack_time"]      = v.AttackTime;
    s["sustain_time"]     = v.SustainTime;
    s["sustain_punch"]    = v.SustainPunch;
    s["decay_time"]       = v.DecayTime;
    s["start_frequency"]  = v.StartFrequency;
    s["min_frequency"]    = v.MinFrequency;
    s["slide"]            = v.Slide;
    s["delta_slide"]      = v.DeltaSlide;
    s["vibrato_depth"]    = v.VibratoDepth;
    s["vibrato_speed"]    = v.VibratoSpeed;
    s["change_amount"]    = v.ChangeAmount;
    s["change_speed"]     = v.ChangeSpeed;
    s["square_duty"]      = v.SquareDuty;
    s["duty_sweep"]       = v.DutySweep;
    s["repeat_speed"]     = v.RepeatSpeed;
    s["phaser_offset"]    = v.PhaserOffset;
    s["phaser_sweep"]     = v.PhaserSweep;
    s["lpf_cutoff"]       = v.LowPassFilterCutoff;
    s["lpf_cutoff_sweep"] = v.LowPassFilterCutoffSweep;
    s["lpf_resonance"]    = v.LowPassFilterResonance;
    s["hpf_cutoff"]       = v.HighPassFilterCutoff;
    s["hpf_cutoff_sweep"] = v.HighPassFilterCutoffSweep;
}

auto Deserialize(sound_wave& v, auto&& s) -> bool
{
    return s.try_get(v.RandomSeed, "random_seed")
        && s.try_get(v.SampleRate, "sample_rate")
        && s.try_get(v.WaveType, "wave_type")
        && s.try_get(v.AttackTime, "attack_time")
        && s.try_get(v.SustainTime, "sustain_time")
        && s.try_get(v.SustainPunch, "sustain_punch")
        && s.try_get(v.DecayTime, "decay_time")
        && s.try_get(v.StartFrequency, "start_frequency")
        && s.try_get(v.MinFrequency, "min_frequency")
        && s.try_get(v.Slide, "slide")
        && s.try_get(v.DeltaSlide, "delta_slide")
        && s.try_get(v.VibratoDepth, "vibrato_depth")
        && s.try_get(v.VibratoSpeed, "vibrato_speed")
        && s.try_get(v.ChangeAmount, "change_amount")
        && s.try_get(v.ChangeSpeed, "change_speed")
        && s.try_get(v.SquareDuty, "square_duty")
        && s.try_get(v.DutySweep, "duty_sweep")
        && s.try_get(v.RepeatSpeed, "repeat_speed")
        && s.try_get(v.PhaserOffset, "phaser_offset")
        && s.try_get(v.PhaserSweep, "phaser_sweep")
        && s.try_get(v.LowPassFilterCutoff, "lpf_cutoff")
        && s.try_get(v.LowPassFilterCutoffSweep, "lpf_cutoff_sweep")
        && s.try_get(v.LowPassFilterResonance, "lpf_resonance")
        && s.try_get(v.HighPassFilterCutoff, "hpf_cutoff")
        && s.try_get(v.HighPassFilterCutoffSweep, "hpf_cutoff_sweep");
}
}
